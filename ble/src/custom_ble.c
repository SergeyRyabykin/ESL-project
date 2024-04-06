#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "peer_manager_handler.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#include "custom_service.h"
#include "custom_vptr_queue.h"
#include "custom_ble.h"

#define DEVICE_NAME                     "SergeyRyabykin"                             /**< Name of device. Will be included in the advertising data. */
#define MANUFACTURER_NAME               "NordicSemiconductor"                   /**< Manufacturer. Will be passed to Device Information Service. */
#define APP_ADV_INTERVAL                300                                     /**< The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms). */

#define APP_ADV_DURATION                18000                                   /**< The advertising duration (180 seconds) in units of 10 milliseconds. */
#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        /**< Minimum acceptable connection interval (0.1 seconds). */
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        /**< Maximum acceptable connection interval (0.2 second). */
#define SLAVE_LATENCY                   0                                       /**< Slave latency. */
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         /**< Connection supervisory timeout (4 seconds). */

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   /**< Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds). */
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  /**< Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds). */
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       /**< Number of attempts before giving up the connection parameter negotiation. */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */


NRF_BLE_GATT_DEF(m_gatt);                                                       /**< GATT module instance. */
NRF_BLE_QWR_DEF(m_qwr);                                                         /**< Context for the Queued Write module.*/
BLE_ADVERTISING_DEF(m_advertising);                                             /**< Advertising module instance. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

static ble_uuid_t m_adv_uuids[] =                                               /**< Universally unique service identifiers. */
{
    {BLE_UUID_DEVICE_INFORMATION_SERVICE, BLE_UUID_TYPE_BLE},
    {CUSTOM_SERVICE_1_UUID, BLE_UUID_TYPE_BLE},
};

static custom_cb_ble_write_data_t func_cb = NULL;
static uint8_t g_char1_val[100];
static uint8_t g_char2_val[100];

static volatile bool g_is_indicating = false; ///< Flag to watch if the clint's acknowledge was received


static void process_indication_queue(void);
custom_vptr_queue_t indication_queue = CUSTOM_VPTR_QUEUE_INIT_VALUES(&g_is_indicating, process_indication_queue);

// TODO: This part of code has a bug because it is impossible to process the same characteristic in the queue - value will be the same but not the previous one
static void process_indication_queue(void)
{
    ret_code_t ret;
    // ble_custom_characteristic_t *target = NULL;
    void *target = NULL;

    ret = custom_vptr_queue_get(&target, &indication_queue);

    if(NRF_SUCCESS == ret && target){
        ret = custom_ble_send_characteristic_value(m_conn_handle, target, BLE_GATT_HVX_INDICATION);
        if(NRF_SUCCESS == ret){
            g_is_indicating = true;
        }
    }
}

const char char1_description[] = "User defined characteristic №1";
const char char2_description[] = "User defined characteristic №2";

ble_custom_characteristic_t char1 = {
    .char_uuid.uuid = CUSTOM_GATT_CHAR_1_UUID,
    .char_md = { 
        // .char_props.read = 1,
        .char_props.write = 1,
        // .char_props.notify = 1,
        .p_char_user_desc = (const uint8_t *) char1_description,
        .char_user_desc_max_size = sizeof(char1_description),
        .char_user_desc_size =  sizeof(char1_description)
    },
    .attr_md = {
        .vloc = BLE_GATTS_VLOC_STACK,
        // .read_perm.sm = 1,
        // .read_perm.lv = 1,
        .write_perm.sm = 1,
        .write_perm.lv = 1,
    },
    .value = (uint8_t *)&g_char1_val,
    .val_len = sizeof(g_char1_val)
};

ble_custom_characteristic_t char2 = {
    .char_uuid.uuid = CUSTOM_GATT_CHAR_2_UUID,
    .char_md = { 
        .char_props.read = 1,
        .char_props.notify = 1,
        .p_char_user_desc = (const uint8_t *) char2_description,
        .char_user_desc_max_size = sizeof(char2_description),
        .char_user_desc_size =  sizeof(char2_description)
    },
    .attr_md = {
        .vloc = BLE_GATTS_VLOC_STACK,
        .read_perm.sm = 1,
        .read_perm.lv = 1,
    },
    // .value = (uint8_t *)&g_char2_val,
    // .val_len = sizeof(g_char2_val)
};

ble_custom_characteristic_t *characteristics[] = {&char1, &char2};

ble_custom_service_t m_estc_service = {
    .base_service_uuid.uuid128 = CUSTOM_BASE_UUID,
    .service_uuid.uuid = CUSTOM_SERVICE_1_UUID,
    .p_characteristics = characteristics,
    .char_num = ARRAY_SIZE(characteristics)
};



void custom_ble_notify_color_changed(void const *data, uint16_t len)
{
    ret_code_t ret;

    memcpy(g_char2_val, data, len);
    char2.val_len = len;

    ble_gatts_value_t p_val = {
        .len = char2.val_len,
        .p_value = char2.value
    };

    sd_ble_gatts_value_set(m_conn_handle, char2.char_handles.value_handle, &p_val);

    uint16_t type = BLE_GATT_HVX_INVALID;

    ret = custom_ble_get_cccd(m_conn_handle, &char2, &type);

    if(NRF_SUCCESS == ret && BLE_GATT_HVX_INVALID != type){
        custom_ble_send_characteristic_value(m_conn_handle, &char2, type);
    }
}




/**@brief Callback function for asserts in the SoftDevice.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num   Line number of the failing ASSERT call.
 * @param[in] file_name  File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    while(1) {
        // NRF_LOG_INFO("%d : %s", line_num, p_file_name);
        // LOG_BACKEND_USB_PROCESS();
        // NRF_LOG_PROCESS();
    };
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}

/**@brief Function for the GAP initialization.
 *
 * @details This function sets up all the necessary GAP (Generic Access Profile) parameters of the
 *          device including the device name, appearance, and the preferred connection parameters.
 */
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

	err_code = sd_ble_gap_appearance_set(BLE_APPEARANCE_UNKNOWN);
	APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the GATT module.
 */
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling Queued Write Module errors.
 *
 * @details A pointer to this function will be passed to each service which may need to inform the
 *          application about an error.
 *
 * @param[in]   nrf_error   Error code containing information about what went wrong.
 */
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}

/**@brief Function for initializing services that will be used by the application.
 */
static void services_init(void)
{
    ret_code_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // Initialize Queued Write Module.
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    err_code = custom_ble_service_init(&m_estc_service);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the Connection Parameters Module.
 *
 * @details This function will be called for all events in the Connection Parameters Module which
 *          are passed to the application.
 *          @note All this function does is to disconnect. This could have been done by simply
 *                setting the disconnect_on_fail config parameter, but instead we use the event
 *                handler mechanism to demonstrate its use.
 *
 * @param[in] p_evt  Event received from the Connection Parameters Module.
 */
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


/**@brief Function for handling a Connection Parameters error.
 *
 * @param[in] nrf_error  Error code containing information about what went wrong.
 */
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


/**@brief Function for initializing the Connection Parameters module.
 */
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for putting the chip into sleep mode.
 *
 * @note This function will not return.
 */
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // Prepare wakeup buttons.
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // Go to system-off mode (this function will not return; wakeup will cause a reset).
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling advertising events.
 *
 * @details This function will be called for advertising events which are passed to the application.
 *
 * @param[in] ble_adv_evt  Advertising event.
 */
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("ADV Event: Start fast advertising");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            NRF_LOG_INFO("ADV Event: idle, no connectable advertising is ongoing");
            sleep_mode_enter();
            break;

        default:
            break;
    }
}

static ret_code_t custom_ble_erase_gatts_value(uint16_t conn_handle, uint16_t val_handle)
{
    ble_gatts_value_t val = {
        .len = 0,
        .p_value = NULL
    };

    ret_code_t err_code = sd_ble_gatts_value_get(m_conn_handle, char1.char_handles.value_handle, &val);
    if(NRF_SUCCESS == err_code) {
        uint8_t array[val.len];
        memset(array, 0, val.len);
        val.p_value = array;
        NRF_LOG_INFO("Erasing length: %d", val.len);
        err_code = sd_ble_gatts_value_set(m_conn_handle, char1.char_handles.value_handle, &val);
    }

    return err_code;
}

/**@brief Function for handling BLE events.
 *
 * @param[in]   p_ble_evt   Bluetooth stack event.
 * @param[in]   p_context   Unused.
 */
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            // LED indication will be changed when advertising starts.
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);

            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);

            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);

            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request (conn_handle: %d)", p_ble_evt->evt.gap_evt.conn_handle);
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // Disconnect on GATT Client timeout event.
            NRF_LOG_DEBUG("GATT Client Timeout (conn_handle: %d)", p_ble_evt->evt.gattc_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // Disconnect on GATT Server timeout event.
            NRF_LOG_DEBUG("GATT Server Timeout (conn_handle: %d)", p_ble_evt->evt.gatts_evt.conn_handle);
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            NRF_LOG_INFO("BLE_GATTS_EVT_HVN_TX_COMPLETE");
            // TODO: Notification transmission complete
            break; 

        case BLE_GATTS_EVT_HVC:
            NRF_LOG_INFO("BLE_GATTS_EVT_HVC");

            if(!custom_vptr_queue_is_empty(&indication_queue)){
                process_indication_queue();
            }
            else {
                g_is_indicating = false;
            }

            break;

        case BLE_GATTS_EVT_WRITE:
            NRF_LOG_INFO("BLE_GATTS_EVT_WRITE");

            ble_gatts_value_t r_val = {
                .len = 0,
                .p_value = NULL
            };

            err_code = sd_ble_gatts_value_get(m_conn_handle, char1.char_handles.value_handle, &r_val);
            if(NRF_SUCCESS == err_code) {
                r_val.p_value = g_char1_val;
                err_code = sd_ble_gatts_value_get(m_conn_handle, char1.char_handles.value_handle, &r_val);
            }

            if(NRF_SUCCESS == err_code) {
                func_cb(g_char1_val);
            }

            err_code = custom_ble_erase_gatts_value(m_conn_handle, char1.char_handles.value_handle);

            if(NRF_SUCCESS != err_code) {
                NRF_LOG_INFO("VAL ERASING ERROR");
            }
            break;

        default:
            NRF_LOG_INFO("default: %x", p_ble_evt->header.evt_id);
            // No implementation needed.
            break;
    }
}

/**@brief Function for initializing the BLE stack.
 *
 * @details Initializes the SoftDevice and the BLE event interrupt.
 */


static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // Enable BLE stack.
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // Register a handler for BLE events.
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);
}

/**@brief Function for initializing the Advertising functionality.
 */
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    // TODO: 8. Consider moving the device characteristics to the Scan Response if necessary
    init.advdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.advdata.uuids_complete.p_uuids  = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids  = m_adv_uuids;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}

/**@brief Function for starting advertising.
 */
static void advertising_start(void)
{
    ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
    APP_ERROR_CHECK(err_code);
}

void custom_ble_init(custom_hsv_t *color, custom_cb_ble_write_data_t custom_ble_write_data_cb)
{
    char2.value = (uint8_t *)g_char2_val;
    char2.val_len = sizeof(g_char2_val);

    func_cb = custom_ble_write_data_cb;

    ble_stack_init();
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    advertising_start();
}



