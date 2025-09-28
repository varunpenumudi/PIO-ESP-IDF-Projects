#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "esp_log.h"


#define UART_EAL_RX_BUFF_SIZE 1024
#define UART_EAL_TX_BUFF_SIZE 0

#define UART2_RX_PIN     16
#define UART2_TX_PIN     17


static const char* TAG = "MAIN_APP";


void uart_eal_init(uart_port_t uart_port_num, int rx_pin, int tx_pin, int baud_rate, int event_queue_size, QueueHandle_t *event_queue) {
    // driver install
    ESP_ERROR_CHECK(uart_driver_install(uart_port_num, UART_EAL_RX_BUFF_SIZE, UART_EAL_TX_BUFF_SIZE, event_queue_size, event_queue, 0));
    ESP_LOGI(TAG, "UART Port %d Driver Installed", uart_port_num);

    // uart communication config
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };
    ESP_ERROR_CHECK(uart_param_config(uart_port_num, &uart_config));
    ESP_LOGI(TAG, "UART Port %d is configured for %d baudrate", uart_port_num, baud_rate);

    // set pins for uart
    ESP_ERROR_CHECK(uart_set_pin(uart_port_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_LOGI(TAG, "UART Pins for Port: %d  are Set as Tx pin - IO%d, Rx Pin - IO%d", uart_port_num, tx_pin, rx_pin);
}


void app_main() {
    int uart2_event_queue_size = 10;
    QueueHandle_t uart2_event_queue;
    uart_eal_init(UART_NUM_2, UART2_RX_PIN, UART2_TX_PIN, 115200, uart2_event_queue_size, &uart2_event_queue);

    while (true) {
        while (getchar() == -1)  {
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
        uint8_t buffer[8] = {0x01, 0x02, 0x03, 0x04, 0x05, [7] = 0x08};
        uart_write_bytes(UART_NUM_2, buffer, 8);
        ESP_LOG_BUFFER_HEX(TAG, buffer, 8);
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}