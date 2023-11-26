#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "protocol_examples_common.h"
#include "esp_http_client.h"

#include "pir.h"
#include <driver/adc.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_adc/adc_cali_scheme.h>

#include "esp_spiffs.h"
#include "esp_log.h"

#include "esp_task_wdt.h"

#define SPIFFS_TAG "spiffs"
#define BUFFER_SIZE 512

static const char *TAG = "HTTP_POST_JSON_EXAMPLE";

void deleteLine(const int line, int *lines);

static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGE(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        break;
    }
    return ESP_OK;
}

int http_post_json_example()
{
    FILE *file = fopen("/spiffs/data.txt", "r");
    if (file == NULL)
    {
        ESP_LOGE(TAG, "Error read file");
        return 1;
    }
    ESP_LOGI(SPIFFS_TAG, "File reading...");

    char post_data[BUFFER_SIZE];
    fgets(post_data, sizeof(post_data), file);
    ESP_LOGI(TAG, "Data: %s", post_data);
    fclose(file);

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
        ESP_LOGI(TAG, "HTTP POST Status = %d, content_length = %lld",
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));
    }
    else
    {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }

    // Clean up
    esp_http_client_cleanup(client);
    return 0;
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    // Initialize SPIFFS
    esp_vfs_spiffs_conf_t spiffs_config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_err_t ret = esp_vfs_spiffs_register(&spiffs_config);
    if (ret != ESP_OK)
    {
        ESP_LOGE(SPIFFS_TAG, "Failed to mount SPIFFS (%s)", esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(SPIFFS_TAG, "SPIFFS mounted successfully");

    // Run the HTTP POST JSON example
    pir_init_adc_channel_1(ADC1_CHANNEL_0);

    int count = 0, lines = 1;
    while (1)
    {
        FILE *file = fopen("/spiffs/data.txt", "a"); // Open in append mode
        if (file == NULL)
        {
            ESP_LOGE(SPIFFS_TAG, "Error opening file");
            continue;
        }

        if (!count)
        {
            fprintf(file, "{\"device_id\":\"1\",\"data\":\"");
        }
        fprintf(file, "#%lld--%d", time(0), adc1_get_raw(ADC1_CHANNEL_0));
        ESP_LOGI(TAG, "Writing #%lld--%d", time(0), adc1_get_raw(ADC1_CHANNEL_0));
        count++;
        if (count == 10)
        {
            fprintf(file, "\"}\n");
            lines++;
            ESP_LOGI(TAG, "Lines=%d", lines);
            count = 0;
        }
        fclose(file);
        if (lines > 1)
        {
            http_post_json_example();
            deleteLine(1, &lines);
        }
        vTaskDelay(pdMS_TO_TICKS(1000)); // Nghỉ một giây
    }
    esp_vfs_spiffs_unregister(NULL);
}

void deleteLine(const int line, int *lines)
{
    FILE *src = fopen("/spiffs/data.txt", "w"); // Open in append mode
    if (src == NULL)
    {
        ESP_LOGE(SPIFFS_TAG, "Error opening file in delete func");
        return;
    }
    FILE *temp = fopen("/spiffs/temp.txt", "w");
    ;
    if (temp == NULL)
    {
        ESP_LOGE(TAG, "Error open temp file in delete func");
        return;
    }
    char buffer[BUFFER_SIZE];
    int count = 1;
    while ((fgets(buffer, BUFFER_SIZE, src)) != NULL)
    {
        if (line != count)
            fputs(buffer, temp);
        count++;
    }
    *lines-=1;
    ESP_LOGI(TAG, "Delete first line successfully");
    fclose(src);
    fclose(temp);
}