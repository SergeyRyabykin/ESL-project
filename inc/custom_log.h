#ifndef CUSTOM_LOG_H__
#define CUSTOM_LOG_H__

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_log_backend_usb.h"

#define LOG(...) { \
    NRF_LOG_INFO(__VA_ARGS__); \
    LOG_BACKEND_USB_PROCESS(); \
    NRF_LOG_PROCESS(); \
}

#endif // CUSTOM_LOG_H__