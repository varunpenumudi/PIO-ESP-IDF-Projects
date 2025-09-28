#include <stdio.h>
#include <string.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"

// A tag for our log messages
static const char *TAG = "SPP_SERVER";

void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param) {
    switch (event) {
        case ESP_BT_GAP_PIN_REQ_EVT: {
            ESP_LOGI(TAG, "GAPP Pin request recieved");
            esp_bt_pin_code_t pin;
            memcpy(pin, "1234", 4);
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin);
            break;
        }
        case ESP_BT_GAP_CFM_REQ_EVT: {
            ESP_LOGI(TAG, "Pairing request recieved, Auto confirming");
            esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
            break;
        }
        case ESP_BT_GAP_AUTH_CMPL_EVT: {
            if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
                ESP_LOGI(TAG, "Succesfully Paired with Server");
            }
            else {
                ESP_LOGI(TAG, "Pairing Operation Failed");
            }
            break;
        }
        default: {
            break;
        }
    }
}

void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(TAG, "SPP Initialized");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        esp_spp_start_srv(ESP_SPP_SEC_AUTHENTICATE, ESP_SPP_ROLE_SLAVE, 0, "SPP_SERVER");
        break;
    case ESP_SPP_SRV_OPEN_EVT:
        ESP_LOGI(TAG, "Client Connected");
        break;
    case ESP_SPP_DATA_IND_EVT:
        ESP_LOGI(TAG, "Data Received: %s", param->data_ind.data);
        if (param->data_ind.len>=2 && memcmp("ON", param->data_ind.data, 2) == 0)  {
            ESP_LOGI(TAG, "Switched on GPIO!");
            gpio_set_level(GPIO_NUM_22, 1);
        }
        else if (param->data_ind.len>=3 && memcmp("OFF", param->data_ind.data, 3) == 0) {
            ESP_LOGI(TAG, "Switched off GPIO!");
            gpio_set_level(GPIO_NUM_22, 0);
        }
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(TAG, "Client Disconnected");
        break;
    default:
        break;
    }
}

void app_main(void)
{
    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);

    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BTDM));

    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_bt_gap_register_callback(esp_bt_gap_cb));
    ESP_ERROR_CHECK(esp_bt_gap_set_device_name("MY_SPP_SERVER"));
    uint8_t iocap = ESP_BT_IO_CAP_OUT;
    ESP_ERROR_CHECK(esp_bt_gap_set_security_param(ESP_BT_SP_IOCAP_MODE, &iocap, sizeof(uint8_t)));

    ESP_ERROR_CHECK(esp_spp_register_callback(esp_spp_cb));
    esp_spp_cfg_t esp_spp_cfg = {
        .mode = ESP_SPP_MODE_CB,
        .enable_l2cap_ertm = true,
        .tx_buffer_size = 0,
    };
    ESP_ERROR_CHECK(esp_spp_enhanced_init(&esp_spp_cfg));
}