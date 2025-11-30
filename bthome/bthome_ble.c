#include "bthome_ble.h"
#include "bthome.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include <string.h>

static const char *TAG = "bthome_ble";

// Scanner state
static struct {
    bool initialized;
    bool scanning;
    bthome_ble_scanner_config_t config;
} scanner_state = {0};

// Forward declarations
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

void bthome_ble_scanner_get_default_config(bthome_ble_scanner_config_t *config) {
    config->scan_duration = 0;  // Continuous
    config->scan_type = BLE_SCAN_TYPE_PASSIVE;
    config->own_addr_type = BLE_ADDR_TYPE_PUBLIC;
    config->filter_policy = BLE_SCAN_FILTER_ALLOW_ALL;
    config->scan_interval = 0x50;  // 50ms
    config->scan_window = 0x30;    // 30ms
    config->callback = NULL;
    config->user_data = NULL;
}

esp_err_t bthome_ble_scanner_init(void) {
    if (scanner_state.initialized) {
        ESP_LOGW(TAG, "Scanner already initialized");
        return ESP_OK;
    }

    esp_err_t ret;

    // Release BT Classic memory if not needed
    ret = esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Failed to release BT Classic memory: %s", esp_err_to_name(ret));
    }

    // Initialize BT controller
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize BT controller: %s", esp_err_to_name(ret));
        return ret;
    }

    // Enable BT controller in BLE mode
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable BT controller: %s", esp_err_to_name(ret));
        esp_bt_controller_deinit();
        return ret;
    }

    // Initialize Bluedroid
    ret = esp_bluedroid_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize Bluedroid: %s", esp_err_to_name(ret));
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        return ret;
    }

    // Enable Bluedroid
    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable Bluedroid: %s", esp_err_to_name(ret));
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        return ret;
    }

    // Register GAP callback
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register GAP callback: %s", esp_err_to_name(ret));
        esp_bluedroid_disable();
        esp_bluedroid_deinit();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        return ret;
    }

    scanner_state.initialized = true;
    scanner_state.scanning = false;
    ESP_LOGI(TAG, "BTHome BLE scanner initialized");

    return ESP_OK;
}

esp_err_t bthome_ble_scanner_deinit(void) {
    if (!scanner_state.initialized) {
        return ESP_OK;
    }

    if (scanner_state.scanning) {
        bthome_ble_scanner_stop();
    }

    esp_bluedroid_disable();
    esp_bluedroid_deinit();
    esp_bt_controller_disable();
    esp_bt_controller_deinit();

    scanner_state.initialized = false;
    ESP_LOGI(TAG, "BTHome BLE scanner deinitialized");

    return ESP_OK;
}

bool bthome_ble_is_bthome_advertisement(const uint8_t *adv_data, uint8_t adv_data_len) {
    size_t offset = 0;
    
    while (offset < adv_data_len) {
        if (offset + 1 >= adv_data_len) {
            break;
        }
        
        uint8_t ad_len = adv_data[offset++];
        if (ad_len == 0) {
            continue;
        }
        
        if (offset + ad_len > adv_data_len) {
            break;
        }
        
        uint8_t ad_type = adv_data[offset];
        
        // Check for Service Data - 16-bit UUID (0x16)
        if (ad_type == 0x16 && ad_len >= 3) {
            // Check if UUID matches BTHome (0xFCD2 in little endian)
            uint16_t uuid = adv_data[offset + 1] | (adv_data[offset + 2] << 8);
            if (uuid == BTHOME_UUID_LE) {
                return true;
            }
        }
        
        offset += ad_len;
    }
    
    return false;
}

static void process_scan_result(esp_ble_gap_cb_param_t *scan_result) {
    if (!scanner_state.config.callback) {
        return;
    }

    uint8_t *adv_data = scan_result->scan_rst.ble_adv;
    uint8_t adv_data_len = scan_result->scan_rst.adv_data_len;
    
    // Check if this is a BTHome advertisement
    if (!bthome_ble_is_bthome_advertisement(adv_data, adv_data_len)) {
        return;
    }
    
    // Decode the BTHome packet
    bthome_packet_t packet;
    int result = bthome_decode_advertisement(adv_data, adv_data_len, &packet);
    
    if (result == 0) {
        // Call user callback
        scanner_state.config.callback(
            scan_result->scan_rst.bda,
            scan_result->scan_rst.rssi,
            &packet,
            scanner_state.config.user_data
        );
        
        // Free the packet resources
        bthome_packet_free(&packet);
    } else {
        ESP_LOGD(TAG, "Failed to decode BTHome packet: %d", result);
    }
}

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
        case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT:
            if (param->scan_param_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Scan parameters set successfully");
                esp_ble_gap_start_scanning(scanner_state.config.scan_duration);
            } else {
                ESP_LOGE(TAG, "Failed to set scan parameters: %d", param->scan_param_cmpl.status);
            }
            break;

        case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
            if (param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Scan started successfully");
                scanner_state.scanning = true;
            } else {
                ESP_LOGE(TAG, "Failed to start scan: %d", param->scan_start_cmpl.status);
                scanner_state.scanning = false;
            }
            break;

        case ESP_GAP_BLE_SCAN_RESULT_EVT:
            switch (param->scan_rst.search_evt) {
                case ESP_GAP_SEARCH_INQ_RES_EVT:
                    // Process scan result
                    process_scan_result(param);
                    break;
                    
                case ESP_GAP_SEARCH_INQ_CMPL_EVT:
                    ESP_LOGI(TAG, "Scan complete");
                    scanner_state.scanning = false;
                    break;
                    
                default:
                    break;
            }
            break;

        case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
            if (param->scan_stop_cmpl.status == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Scan stopped successfully");
            } else {
                ESP_LOGE(TAG, "Failed to stop scan: %d", param->scan_stop_cmpl.status);
            }
            scanner_state.scanning = false;
            break;

        default:
            break;
    }
}

esp_err_t bthome_ble_scanner_start(const bthome_ble_scanner_config_t *config) {
    if (!scanner_state.initialized) {
        ESP_LOGE(TAG, "Scanner not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (scanner_state.scanning) {
        ESP_LOGW(TAG, "Scanner already running");
        return ESP_ERR_INVALID_STATE;
    }

    if (!config || !config->callback) {
        ESP_LOGE(TAG, "Invalid configuration or missing callback");
        return ESP_ERR_INVALID_ARG;
    }

    // Store configuration
    memcpy(&scanner_state.config, config, sizeof(bthome_ble_scanner_config_t));

    // Configure scan parameters
    static esp_ble_scan_params_t scan_params = {0};
    scan_params.scan_type = config->scan_type;
    scan_params.own_addr_type = config->own_addr_type;
    scan_params.scan_filter_policy = config->filter_policy;
    scan_params.scan_interval = config->scan_interval;
    scan_params.scan_window = config->scan_window;
    scan_params.scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE;

    esp_err_t ret = esp_ble_gap_set_scan_params(&scan_params);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set scan params: %s", esp_err_to_name(ret));
        return ret;
    }

    // Scan will start automatically in the GAP event handler
    return ESP_OK;
}

esp_err_t bthome_ble_scanner_stop(void) {
    if (!scanner_state.initialized) {
        ESP_LOGE(TAG, "Scanner not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!scanner_state.scanning) {
        ESP_LOGW(TAG, "Scanner not running");
        return ESP_OK;
    }

    esp_err_t ret = esp_ble_gap_stop_scanning();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to stop scanning: %s", esp_err_to_name(ret));
        return ret;
    }

    return ESP_OK;
}
