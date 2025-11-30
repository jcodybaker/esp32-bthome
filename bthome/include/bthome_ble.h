#ifndef BTHOME_BLE_H
#define BTHOME_BLE_H

#include <stdint.h>
#include <stdbool.h>
#include "bthome.h"
#include "esp_gap_ble_api.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Callback function type for BTHome packet reception
 * @param addr The BLE address of the device
 * @param rssi The RSSI value of the advertisement
 * @param packet The decoded BTHome packet
 * @param user_data User-provided data pointer
 */
typedef void (*bthome_ble_callback_t)(esp_bd_addr_t addr, int rssi, 
                                       const bthome_packet_t *packet, void *user_data);

/**
 * BLE scanner configuration
 */
typedef struct {
    uint32_t scan_duration;        // Scan duration in seconds (0 = continuous)
    esp_ble_scan_type_t scan_type; // Active or passive scan
    esp_ble_addr_type_t own_addr_type;
    esp_ble_scan_filter_t filter_policy;
    uint16_t scan_interval;        // Scan interval (units of 0.625ms)
    uint16_t scan_window;          // Scan window (units of 0.625ms)
    bthome_ble_callback_t callback; // Callback for received packets
    void *user_data;               // User data passed to callback
} bthome_ble_scanner_config_t;

/**
 * Initialize the BTHome BLE scanner
 * Initializes the BLE controller and GAP
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bthome_ble_scanner_init(void);

/**
 * Deinitialize the BTHome BLE scanner
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bthome_ble_scanner_deinit(void);

/**
 * Start scanning for BTHome advertisements
 * @param config Scanner configuration
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bthome_ble_scanner_start(const bthome_ble_scanner_config_t *config);

/**
 * Stop scanning for BTHome advertisements
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t bthome_ble_scanner_stop(void);

/**
 * Get default scanner configuration
 * @param config Configuration structure to populate with defaults
 */
void bthome_ble_scanner_get_default_config(bthome_ble_scanner_config_t *config);

/**
 * Check if a BLE advertisement contains BTHome service data
 * @param adv_data Advertisement data
 * @param adv_data_len Length of advertisement data
 * @return true if BTHome service data is present
 */
bool bthome_ble_is_bthome_advertisement(const uint8_t *adv_data, uint8_t adv_data_len);

#ifdef __cplusplus
}
#endif

#endif // BTHOME_BLE_H
