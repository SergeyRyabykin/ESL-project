#include "app_usbd.h"
#include "app_usbd_serial_num.h"
#include "app_usbd_cdc_acm.h"

#include "custom_cmd.h"
#include "custom_queue.h"
#include "custom_cli.h"

#define READ_SIZE 1

/* Make sure that they don't intersect with LOG_BACKEND_USB_CDC_ACM */
#define CUSTOM_CDC_ACM_COMM_INTERFACE  2
#define CUSTOM_CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN3

#define CUSTOM_CDC_ACM_DATA_INTERFACE  3
#define CUSTOM_CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN4
#define CUSTOM_CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT4

static char m_rx_buffer[READ_SIZE];
static char g_cmd_str[CUSTOM_CMD_STR_LENGTH];
static char g_message[CUSTOM_QUEUE_ROOM_SIZE];
static const char g_cmd_error_message[] = "Unknown command\r\n";

static custom_queue_t g_custom_queue_output = CUSTOM_QUEUE_INIT_VALUES;
static custom_cmd_ctx_t *g_custom_cmd_ctx_ptr = NULL;
static custom_app_ctx_t *g_custom_app_ctx_ptr = NULL;

static void custom_usb_event_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event);



APP_USBD_CDC_ACM_GLOBAL_DEF(custom_usb_cdc_acm,
                            custom_usb_event_handler,
                            CUSTOM_CDC_ACM_COMM_INTERFACE,
                            CUSTOM_CDC_ACM_DATA_INTERFACE,
                            CUSTOM_CDC_ACM_COMM_EPIN,
                            CUSTOM_CDC_ACM_DATA_EPIN,
                            CUSTOM_CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_NONE);



ret_code_t custom_cli_init(const custom_app_ctx_t *app_ctx)
{
    g_custom_app_ctx_ptr = (custom_app_ctx_t *)app_ctx;
    g_custom_cmd_ctx_ptr = (custom_cmd_ctx_t *)app_ctx->custom_cmd_ctx;

    app_usbd_class_inst_t const * custom_class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&custom_usb_cdc_acm);
    return app_usbd_class_append(custom_class_cdc_acm);
}



ret_code_t custom_cli_print(char *str)
{
    return custom_queue_add(&g_custom_queue_output, str);
}

static void custom_usb_event_handler(app_usbd_class_inst_t const * p_inst,
                           app_usbd_cdc_acm_user_event_t event)
{
    switch (event) {
    case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN: {
        ret_code_t ret;
        ret = app_usbd_cdc_acm_read(&custom_usb_cdc_acm, m_rx_buffer, READ_SIZE);
        UNUSED_VARIABLE(ret);
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE: {
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_TX_DONE: {
        if(!custom_queue_is_empty(&g_custom_queue_output)) {
            custom_queue_get(g_message, &g_custom_queue_output);
            app_usbd_cdc_acm_write(&custom_usb_cdc_acm,
                                    g_message,
                                    strlen(g_message));
        }
        break;
    }
    case APP_USBD_CDC_ACM_USER_EVT_RX_DONE: {
        ret_code_t ret;
        do {
            /*Get amount of data transfered*/
            size_t size = app_usbd_cdc_acm_rx_size(&custom_usb_cdc_acm);

            (void)size;

            /* It's the simple version of an echo. Note that writing doesn't
             * block execution, and if we have a lot of characters to read and
             * write, some characters can be missed.
             */
            if (m_rx_buffer[0] == '\r' || m_rx_buffer[0] == '\n') {
                // TODO: The problem is that the queue must be used alwyas to get real state of the transmitter.
                // TODO: Organize the output by adding data to queue.
                if(!custom_queue_is_empty(&g_custom_queue_output)) {
                    custom_queue_add(&g_custom_queue_output, "\r\n");

                }
                else {
                    ret = app_usbd_cdc_acm_write(&custom_usb_cdc_acm, "\r\n", 2);
                }

                if(strlen(g_cmd_str)) {
                    if(g_custom_app_ctx_ptr && g_custom_cmd_ctx_ptr) {
                        ret = custom_cmd_execute(g_cmd_str, g_custom_cmd_ctx_ptr, g_custom_app_ctx_ptr);
                        if(NRF_SUCCESS != ret) {
                            custom_queue_add(&g_custom_queue_output, g_cmd_error_message);
                        }
                    }
                }
                g_cmd_str[0] = '\0';

            }
            else {
                if(!custom_queue_is_empty(&g_custom_queue_output)) {
                    custom_queue_add(&g_custom_queue_output, m_rx_buffer);
                }
                else {
                    ret = app_usbd_cdc_acm_write(&custom_usb_cdc_acm,
                                                m_rx_buffer,
                                                READ_SIZE);
                }

                strncat(g_cmd_str, m_rx_buffer, sizeof(m_rx_buffer));
            }

            /* Fetch data until internal buffer is empty */
            ret = app_usbd_cdc_acm_read(&custom_usb_cdc_acm,
                                        m_rx_buffer,
                                        READ_SIZE);
        } while (ret == NRF_SUCCESS);

        break;
    }
    default:
        break;
    }
}