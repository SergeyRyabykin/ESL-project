#include "custom_buttons.h"
#include "nrfx_gpiote.h"
#include "app_timer.h"
#include "nrf_drv_clock.h"

#define DEBOUNCE_TIME_MS 10
#define DOUBLE_CLICK_TIME_MS 500

typedef struct {
    uint32_t pin;
    volatile custom_button_state_t button_state;
    volatile unsigned char current_click_num;
    volatile bool state_is_processed;
} custom_button_ctx_t;

APP_TIMER_DEF(debounce_timer);
APP_TIMER_DEF(double_click_timer);

static custom_button_ctx_t g_custom_buttons[MAX_NUM_CUSTOM_BUTTONS] = {0};
static unsigned char g_num_reg_pins = 0;

void custom_button_pin_config(uint32_t pin)
{
    nrf_gpio_cfg(pin, 
                 GPIO_PIN_CNF_DIR_Input, 
                 GPIO_PIN_CNF_INPUT_Connect,
                 GPIO_PIN_CNF_PULL_Pullup,
                 GPIO_PIN_CNF_DRIVE_S0S1,
                 GPIO_PIN_CNF_SENSE_Low
                );
}

bool custom_button_is_pressed(uint32_t pin)
{
    return (!nrf_gpio_pin_read(pin));
}

bool custom_button_is_released(uint32_t pin)
{
    return (nrf_gpio_pin_read(pin));
}

bool custom_button_te_is_pressed(uint32_t pin)
{
    return !nrfx_gpiote_in_is_set(pin);
}

bool custom_button_te_is_released(uint32_t pin)
{
    return nrfx_gpiote_in_is_set(pin);
}

static inline custom_button_ctx_t * get_button_context_by_pin(const uint32_t pin)
{
    custom_button_ctx_t *context = NULL;
    
    for(unsigned int i = 0; i < g_num_reg_pins; i++) {
        if(pin == g_custom_buttons[i].pin) {
            context = &g_custom_buttons[i];
        }
    }

    return context;
}

static void custom_click_timeout_handler(void *context)
{
    custom_button_ctx_t * button = (custom_button_ctx_t *) context;

    if(custom_button_te_is_pressed(button->pin)) {
        switch (button->current_click_num) {
            case 1: button->button_state = SINGLE_CLICK_PRESSED; break;
            case 2: button->button_state = DOUBLE_CLICK_PRESSED; break;
        }
    }

    if(custom_button_te_is_released(button->pin)) {
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

    button->state_is_processed = false;
    button->current_click_num = 0;
}

static void custom_click_debounce_timeout_handler(void *context)
{
    custom_button_ctx_t * button = (custom_button_ctx_t *) context;

    if(custom_button_te_is_pressed(button->pin)) {
        // To stabilize double click behavior if there was casual third press
        if(2 > button->current_click_num) {
            button->current_click_num++;
            app_timer_start(double_click_timer, APP_TIMER_TICKS(DOUBLE_CLICK_TIME_MS), (void *)button);
        }
    }

    // To set proper release state when the button was released after double click timer had expired
    if(0 == button->current_click_num && custom_button_te_is_released(button->pin)) {
        if(SINGLE_CLICK_PRESSED == button->button_state) {
            button->button_state = SINGLE_CLICK_RELEASED;
        }
        else {
            button->button_state = DOUBLE_CLICK_RELEASED;
        }
    }
}

static void custom_button_toggle_event_handler(nrfx_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    custom_button_ctx_t *button = NULL;

    for(unsigned int i = 0; i < MAX_NUM_CUSTOM_BUTTONS; i++) {
        if(pin == g_custom_buttons[i].pin) {
            button = &g_custom_buttons[i];
        }
    }

    if(button) {
        app_timer_stop(debounce_timer);
        app_timer_start(debounce_timer, APP_TIMER_TICKS(DEBOUNCE_TIME_MS), (void *)button);
    }
}

ret_code_t custom_button_events_init(void)
{
    ret_code_t ret = 0;

    ret = nrf_drv_clock_init();
    if (NRF_SUCCESS != ret && NRF_ERROR_MODULE_ALREADY_INITIALIZED != ret) {
        return ret;
    }

    nrf_drv_clock_lfclk_request(NULL);

    // RTC initialization
    ret = app_timer_init();
    if(NRF_SUCCESS != ret) {
        return ret;
    }

    // Debounce timer creation
    ret = app_timer_create(&debounce_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_click_debounce_timeout_handler);
    if(NRF_SUCCESS != ret) {
        return ret;
    }

    // Double click timer creation
    ret = app_timer_create(&double_click_timer, APP_TIMER_MODE_SINGLE_SHOT, custom_click_timeout_handler);
    if(NRF_SUCCESS != ret) {
        return ret;
    }

    // GPIOTE initialization
    if(!nrfx_gpiote_is_init()) {
        ret = nrfx_gpiote_init();
        if(NRFX_SUCCESS != ret) {
            return ret;
        }
    }

    return ret;
}

ret_code_t custom_button_event_enable(const uint32_t pin, const nrfx_gpiote_in_config_t *gpiote_cfg)
{
    ret_code_t ret = NRF_ERROR_STORAGE_FULL;

    if(!gpiote_cfg) {
        return (ret = NRF_ERROR_INVALID_PARAM);
    }

    if(g_num_reg_pins < MAX_NUM_CUSTOM_BUTTONS) {
        g_custom_buttons[g_num_reg_pins++].pin = pin;
        ret = nrfx_gpiote_in_init(pin, gpiote_cfg, custom_button_toggle_event_handler);

        nrfx_gpiote_in_event_enable(pin, true);
    }

    return ret;
}

bool custom_button_processed(const uint32_t pin, const bool is_processed)
{
    custom_button_ctx_t *context = get_button_context_by_pin(pin);

    if(context) {
        context->state_is_processed = true;
        return (context->state_is_processed);
    }

    return false;
}

custom_button_state_t custom_button_get_state(const uint32_t pin)
{
    custom_button_ctx_t *context = get_button_context_by_pin(pin);

    if(context) {
        return (context->button_state);
    }

    return DEFAULT_UNKNOWN;
}

bool custom_button_is_processed(const uint32_t pin)
{
    custom_button_ctx_t *context = get_button_context_by_pin(pin);

    if(context) {
        return (context->state_is_processed);
    }

    return true;
}

