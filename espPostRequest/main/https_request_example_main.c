#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "esp_http_client.h"
#include "soc/soc_caps.h"
#include "esp_adc/adc_oneshot.h"

static const char *TAG_HTTP = "HTTP_POST_JSON_EXAMPLE";

const static char *TAG_ADC = "EXAMPLE";
#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_0

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG_HTTP, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG_HTTP, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

void http_post_json_example(char *post_data)
{
    esp_http_client_handle_t client = esp_http_client_init(&(esp_http_client_config_t){
        .url = "http://192.168.1.24:8888/esp",
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
    });

    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Set the JSON data as the POST field
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    // esp_http_client_write(client, post_data, strlen(post_data));

    // Perform the HTTP POST request
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG_HTTP, "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG_HTTP, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up
    esp_http_client_cleanup(client);
}

void oneshot_adc_read(int *value)
{
    //-------------ADC1 Init---------------//
    adc_oneshot_unit_handle_t adc1_handle;
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, value));
    ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, *value);
    // Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    // JSON data to be sent in the POST request
    char *post_data = (char *)malloc(strlen("{\"device_id\":\"1\",\"data\":\"") + 1);
    char *s = (char *)malloc(100);

    // Run the HTTP POST JSON example
    while (1)
    {
        printf("thoi diem bat dau gui: %ld\n", esp_log_timestamp());
        strcpy(post_data, "{\"device_id\":\"1\",\"data\":\"");

        int value;
        oneshot_adc_read(&value);

        // Lấy thời gian hiện tại ở đơn vị giây
        uint32_t current_time_miliseconds;
        current_time_miliseconds=esp_log_timestamp();

        snprintf(s, 100, "%d+time:%ld\"}", value,current_time_miliseconds);
        printf("%s\n", s);
        strcat(post_data, s);
        printf("%s\n", post_data);
        http_post_json_example(post_data);
        printf("thoi diem gui xong: %ld\n--------------------------\n", esp_log_timestamp());
        vTaskDelay(200 / portTICK_PERIOD_MS);
    }
    
}
