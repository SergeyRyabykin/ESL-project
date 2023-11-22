
#include <stdint.h>
#include <assert.h>

#include "sdk_config.h"
#include "nrf_delay.h"
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

#define PWM_BASE_CLOCK_HZ 125000 // Have not to be more then 1 MHz to get correct outcomes. See value NRFX_PWM_DEFAULT_CONFIG_BASE_CLOCK in sdk_config.h
#define PWM_BASE_PERIOD_US (1000000 / PWM_BASE_CLOCK_HZ)
#define PWM_TIME_BETWEEN_HANDLERS_US (NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE * PWM_BASE_PERIOD_US)

#define PWM_DEFAULT_MODE_IND_INC    0
#define PWM_HUE_MODE_IND_INC        (PERIOD_MS / 2 * 1000 / PWM_TIME_BETWEEN_HANDLERS_US)
#define PWM_SATURATION_MODE_IND_INC        (PERIOD_MS * 1000 / PWM_TIME_BETWEEN_HANDLERS_US)
#define PWM_VALUE_MODE_IND_INC NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE

#define PWM_PLAYBACK_COUNT 1

#define DEBOUNCE_TIME_MS 10
#define DOUBLE_CLICK_TIME_MS 300

static const unsigned int device_id[] = {6, 5, 7, 7};
static const uint32_t leds_list[] = CUSTOM_LEDS_LIST;

_Static_assert(ARRAY_SIZE(device_id) <= ARRAY_SIZE(leds_list), "The number of digits in ID exceeds the number of leds!");
_Static_assert(PWM_BASE_CLOCK_HZ <= 1000000 , "The PWM base clock have to be less or equal of 1 MHz frequency!");


typedef enum {
    DEFAULT_MODE,
    HUE_MODE,
    SATURATION_MODE,
    VALUE_MODE,
    MODE_NUMBER
} hsv_control_state_t;

static volatile uint16_t status_indicator_step = 0;
static hsv_control_state_t app_state = DEFAULT_MODE;

static custom_button_context_t main_button = {
    .pin = BUTTON
};

APP_TIMER_DEF(debounce_timer);
APP_TIMER_DEF(double_click_timer);

static volatile nrf_pwm_values_individual_t pwm_values;

static nrfx_pwm_config_t pwm_config = NRFX_PWM_DEFAULT_CONFIG;
static nrfx_pwm_t pwm_inst = NRFX_PWM_INSTANCE(0);
static nrf_pwm_sequence_t pwm_sequence = {
    .values = (nrf_pwm_values_t){.p_individual = (nrf_pwm_values_individual_t *)&pwm_values},
    .length = NRF_PWM_VALUES_LENGTH(pwm_values),
    .repeats = 100,
};

static struct hsv hsv_color = {
    .hue = 277,
    .saturation = 100,
    .brightness = 100
};

struct hsv_control_state {
    custom_button_context_t *button;
    bool saturation_direction;
    bool brightness_direction;
};

struct hsv_control_state hsv_ctrl_state_ctx = { .button = &main_button };


void process_hsv_state(struct hsv *color, struct hsv_control_state *state)
{
    if(SINGLE_CLICK_PRESSED == state->button->button_state) {
        switch(app_state) {
            case HUE_MODE:
                if(360 <= ++color->hue) {
                    color->hue = 0;
                }
                break;
            case SATURATION_MODE:
                if(state->saturation_direction) {
                    if(MAX_SATURATION <= ++color->saturation) {
                        state->saturation_direction = false;
                    }
                }
                else {
                    if(0 >= --color->saturation) {
                        state->saturation_direction = true;
                    }
                }
                break;
            case VALUE_MODE: 
                if(state->brightness_direction) {
                    if(MAX_SATURATION <= ++color->brightness) {
                        state->brightness_direction = false;
                    }
                }
                else {
                    if(0 >= --color->brightness) {
                        state->brightness_direction = true;
                    }
                }
                break;
            default:
                break;
        }
    }

    union rgb the_color = { .components = {0} };

    hsv_to_rgb(hsv_color, &the_color);
    
    CRITICAL_REGION_ENTER();
    pwm_values.channel_1 = the_color.red;
    pwm_values.channel_2 = the_color.green;
    pwm_values.channel_3 = the_color.blue;
    CRITICAL_REGION_EXIT();
}


void application_state_handler(const custom_button_context_t *button)
{
    if(DOUBLE_CLICK_RELEASED == button->button_state) {
        if(MODE_NUMBER <= ++app_state) {
            app_state = DEFAULT_MODE;
        }
    }

    switch (app_state) {
        case DEFAULT_MODE: 
            status_indicator_step = 0; 
            pwm_values.channel_0 = PWM_DEFAULT_MODE_IND_INC; 
            break;
        case HUE_MODE: 
            status_indicator_step = PWM_HUE_MODE_IND_INC;
            break;
        case SATURATION_MODE: 
            status_indicator_step = PWM_SATURATION_MODE_IND_INC; 
            break;
        case VALUE_MODE: 
            status_indicator_step = 0; 
            pwm_values.channel_0 = PWM_VALUE_MODE_IND_INC; 
            break;
        default:
            break;
    }

    NRF_LOG_INFO("Mode: %d", app_state);
}

void custom_pwm_handler(nrfx_pwm_evt_type_t event_type)
{
    static bool status_indicator_direction = true;

    if(status_indicator_step) {
        if(status_indicator_direction) {
            if(NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE >= (pwm_values.channel_0 + status_indicator_step)) {
                pwm_values.channel_0 += status_indicator_step;
            }
            else {
                pwm_values.channel_0 = NRFX_PWM_DEFAULT_CONFIG_TOP_VALUE; // To prevent overruning of the board during missmatch step values because of increasing in the event handler
                status_indicator_direction = false;
            }
        }
        else {
            if(0 <= (pwm_values.channel_0 - status_indicator_step)) {
                pwm_values.channel_0 -= status_indicator_step;
            }
            else {
                pwm_values.channel_0 = 0;  // To prevent overruning of the board during missmatch step values because of increasing in the event handler
                status_indicator_direction = true;
            }
        }

    }

    process_hsv_state(&hsv_color, &hsv_ctrl_state_ctx);
}


// Function to process call if APP_ERROR_CHECK macros failed
void app_error_fault_handler(uint32_t id, uint32_t pc, uint32_t info)
{
    led_on(LED_R);
    // TODO: This approach doesn't work. There is no error message in terminal. Perhaps it depends on start log time.
    NRF_LOG_INFO("ERROR: %d", info);
    while(true) {
        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }
}


// Cutstom timer handler to precess number of button press
void custom_double_click_timer_timeout_handler(void *context)
{
    custom_button_context_t * button = (custom_button_context_t *) context;

    if(button_te_is_pressed(button->pin)) {
        switch (button->current_click_num) {
            case 1: button->button_state = SINGLE_CLICK_PRESSED; break;
            case 2: button->button_state = DOUBLE_CLICK_PRESSED; break;
        }
    }

    if(button_te_is_released(button->pin)) {
        switch(button->current_click_num) {
            case 0:
                if(SINGLE_CLICK_PRESSED == button->button_state) {
                    button->button_state = SINGLE_CLICK_RELEASED;
                }
                else {
                    button->button_state = DOUBLE_CLICK_RELEASED;
                }
                break;
            case 1: button->button_state = SINGLE_CLICK_RELEASED; break;
            case 2: button->button_state = DOUBLE_CLICK_RELEASED; break;
        }
    }

    button->current_click_num = 0;

    application_state_handler(button);
}

// Custom timer handler to process button press and release to avoid redundant bounce
void custom_debounce_timer_timeout_handler(void *context)
{
    custom_button_context_t * button = (custom_button_context_t *) context;

    if(button_te_is_pressed(button->pin)) {
        // To stabilize double click behavior if there was casual third press
        if(2 > button->current_click_num) {
            button->current_click_num++;
        }
    }
}


void custom_button_toggle_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    uint32_t ret = 0;

    ret = app_timer_stop(debounce_timer);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(debounce_timer, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), (void *)&main_button);
    APP_ERROR_CHECK(ret);
    ret = app_timer_start(double_click_timer, APP_TIMER_TICKS(DOUBLE_CLICK_TIME_MS), (void *)&main_button);
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

    while(true) {

        LOG_BACKEND_USB_PROCESS();
        NRF_LOG_PROCESS();
    }   
}

