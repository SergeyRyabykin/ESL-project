
#include <stdint.h>
#include <assert.h>

#include "sdk_config.h"
#include "nrf_delay.h"
// #include "nrfx_systick.h"
#include "app_timer.h"
#include "nrfx_pwm.h"
#include "nrfx_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "custom_leds.h"
#include "custom_buttons.h"
#include "custom_blink.h"
#include "custom_hsv.h"

#define PERIOD_MS 1000
#define DEBOUNCE_TIME_MS 10
#define DOUBLE_CLICK_TIME_MS 500
#define PWM_PLAYBACK_COUNT 1

#define DEFAULT_INDIC_COUNT_VAL 0
#define HUE_INDIC_COUNT_VAL 5
#define SATURATION_INDIC_COUNT_VAL 10
#define BRIGHTNESS_INDIC_COUNT_VAL NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE

#define HUE_VAL_PER_DEGREE (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE / 360)

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

_Static_assert(ARRAY_SIZE(device_id) <= ARRAY_SIZE(leds_list), "The number of digits in ID exceeds the number of leds!");

static volatile unsigned int click_num = 0;
APP_TIMER_DEF(debounce_timer);
APP_TIMER_DEF(double_click_timer);

static volatile nrf_pwm_values_individual_t pwm_values = {
    .channel_0 = 0,
    .channel_1 = 0,
    .channel_2 = 0,
    .channel_3 = 0
};

static nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
static nrfx_pwm_t pwm_inst = NRFX_PWM_INSTANCE(0);
static nrf_pwm_sequence_t pwm_sequence = {
    .values = (nrf_pwm_values_t){.p_individual = (nrf_pwm_values_individual_t *)&pwm_values},
    .length = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats = 100,
    .end_delay = 0
};

typedef enum {
    DEFAULT_MODE,
    HUE_MODE,
    SATURATION_MODE,
    VALUE_MODE,
    MODE_NUMBER
} hsv_control_state_t;

static volatile uint16_t status_indicator_step = 10;
static hsv_control_state_t settings_state = DEFAULT_MODE;
static volatile bool pwm_is_finished = true;
static struct hsv hsv_color = {
    .hue = 277,
    .saturation = 100,
    .brightness = 100
};

void custom_pwm_handler(nrfx_pwm_evt_type_t event_type)
{
    pwm_is_finished = true;
}



void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    led_on(LED_R);
    NRF_LOG_INFO("ERROR: %d", info);
    while(true) {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}

void custom_double_click_timer_timeout_handler(void *context)
{
    click_num = 0;
}

void custom_debounce_timer_timeout_handler(void *context)
{
    // TODO: Create function to change library function.
    if(!nrfx_gpiote_in_is_set(BUTTON)) {
        click_num++;
    }

    if(2 == click_num) {
        click_num = 0;
        if(MODE_NUMBER <= ++settings_state) {
            settings_state = DEFAULT_MODE;
        }

        NRF_LOG_INFO("State: %d", settings_state);
    }
}

void custom_button_toggle_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t ret = 0;

    ret = app_timer_stop(debounce_timer);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(debounce_timer, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), NULL);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(double_click_timer, APP_TIMER_TICKS(DOUBLE_CLICK_TIME_MS), NULL);
    APP_ERROR_CHECK(ret);
}

int main(void)
{
    uint32_t ret = 0;

     // Log initialization
    NRF_LOG_INIT(NULL);
    NRF_LOG_DEFAULT_BACKENDS_INIT();

    config_pins_as_leds(ARRAY_SIZE(leds_list), leds_list);
    all_leds_off(ARRAY_SIZE(leds_list), leds_list);

    config_pin_as_button(BUTTON);

    // Systick initialization
    // nrfx_systick_init();

    // RTC initialization
    ret = app_timer_init();
    APP_ERROR_CHECK(ret);

    // Debounce timer creation
    ret = app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_debounce_timer_timeout_handler);
    APP_ERROR_CHECK(ret);

    // Debounce timer creation
    ret = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_double_click_timer_timeout_handler);
    APP_ERROR_CHECK(ret);


    // GPIOTE initialization
    if(!nrfx_gpiote_is_init()) {
        ret = nrfx_gpiote_init();
        APP_ERROR_CHECK(ret);
    }

    nrfx_gpiote_in_config_t user_gpiote_cfg = NRFX_GPIOTE_CONFIG_IN_SENSE_TOGGLE(false);
    user_gpiote_cfg.pull = NRF_GPIO_PIN_PULLUP;
    ret = nrfx_gpiote_in_init(BUTTON, &user_gpiote_cfg, custom_button_toggle_event_handler);
    APP_ERROR_CHECK(ret);

    nrfx_gpiote_in_event_enable(BUTTON, true);
    
    ret = nrfx_pwm_init(&pwm_inst, &pwm_config, custom_pwm_handler);
    nrfx_pwm_simple_playback(&pwm_inst, &pwm_sequence, PWM_PLAYBACK_COUNT, NRFX_PWM_FLAG_LOOP);

    static bool value_dir = true;
    static bool satur_dir = true;

    while(true) {


        if(pwm_is_finished) {
            if(!nrfx_gpiote_in_is_set(BUTTON)) {
                switch(settings_state) {
                    case HUE_MODE: 
                        if(360 <= ++hsv_color.hue) {
                            hsv_color.hue = 0;
                        };
                        break;
                    case SATURATION_MODE:
                        if(satur_dir){
                            if(MAX_BRIGHTNESS <= ++hsv_color.saturation) {
                                satur_dir = false;
                            }
                        }
                        else {
                            if(0 >= --hsv_color.saturation) {
                                satur_dir = true;
                            }
                        }
                        break;
                    case VALUE_MODE:
                        if(value_dir){
                            if(MAX_BRIGHTNESS <= ++hsv_color.brightness) {
                                value_dir = false;
                            }
                        }
                        else {
                            if(0 >= --hsv_color.brightness) {
                                value_dir = true;
                            }
                        }
                        break;
                    default:
                        ;
                }
            
                union rgb rgb_color = hue_to_rgb(hsv_color.hue);
                // set_brightness(hsv_color.brightness, &rgb_color);
                set_saturation(hsv_color.saturation, &rgb_color);
                
                pwm_values.channel_1 = rgb_color.red;
                pwm_values.channel_2 = rgb_color.green;
                pwm_values.channel_3 = rgb_color.blue;
            }

            // if(value_dir){
            //     if(NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE <= (pwm_values.channel_1 += )) {
            //         value_dir = false;
            //     }
            // }
            // else {
            //     if(0 >= --hsv_color.brightness) {
            //         value_dir = true;
            //     }
            // }

            pwm_is_finished = false;
        }


        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

