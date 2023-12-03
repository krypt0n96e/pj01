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

#define EXAMPLE_ADC1_CHAN0 ADC_CHANNEL_6
#define DEVICE_ID 1
#define HOST "http://192.168.1.24:8888"
#define MAX_HTTP_OUTPUT_BUFFER 256
#define MAX_POST_SIZE 4096
#define TEMP_SIZE0 10
#define VALUE_PER_POST 200
#define TASK0_DELAY 500
#define TASK1_DELAY 2000
#define TASK2_DELAY 20

static const char *TAG_HTTP = "HTTP_CLIENT";
const static char *TAG_ADC = "ONE_SHOT_ADC";

static uint8_t logs = 0;
static uint8_t writeStage = 0;
static uint8_t tempPostIndex;
static char temp[TEMP_SIZE0][MAX_POST_SIZE];

TaskHandle_t task_adc_oneshot_write = NULL;
TaskHandle_t task_http_get_data = NULL;
TaskHandle_t task_http_post_data = NULL;

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

int http_post_json_handle(char *post_data)
{
    esp_http_client_handle_t client = esp_http_client_init(&(esp_http_client_config_t){
        .url = "http://192.168.1.24:8888/esp",
        .event_handler = _http_event_handler,
        .method = HTTP_METHOD_POST,
    });

    esp_http_client_set_header(client, "Content-Type", "application/json");

    // Set the JSON data as the POST field
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

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
        return 1;
    }
    esp_http_client_close(client);
    // Clean up
    esp_http_client_cleanup(client);
    return 0;
}

void http_get_handle()
{
    char *url = (char *)pvPortMalloc(40);
    sprintf(url, "%s/device?id=%d", HOST, DEVICE_ID);

    char *output_buffer = (char *)pvPortMalloc(MAX_HTTP_OUTPUT_BUFFER + 1);
    int content_length = 0;
    esp_http_client_config_t config = {
        .url = url,
        .method = HTTP_METHOD_GET,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err = esp_http_client_open(client, 0);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG_HTTP, "Failed to open HTTP connection: %s", esp_err_to_name(err));
    }
    else
    {
        content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0)
        {
            ESP_LOGE(TAG_HTTP, "HTTP client fetch headers failed");
        }
        else
        {
            int data_read = esp_http_client_read_response(client, output_buffer, MAX_HTTP_OUTPUT_BUFFER);
            if (data_read >= 0)
            {
                ESP_LOGI(TAG_HTTP, "HTTP GET Status = %d, content_length = %" PRId64,
                         esp_http_client_get_status_code(client),
                         esp_http_client_get_content_length(client));

                logs = output_buffer[23] - '0';
                // ESP_LOGI(TAG_HTTP, "LOG: %d", logs);
            }
            else
            {
                ESP_LOGE(TAG_HTTP, "Failed to read response");
            }
        }
    }
    esp_http_client_close(client);
    // Clean up
    esp_http_client_cleanup(client);
    vPortFree(url);
    vPortFree(output_buffer);
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
        .atten = ADC_ATTEN_DB_11,
        .bitwidth = ADC_BITWIDTH_12,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, EXAMPLE_ADC1_CHAN0, &config));
    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, EXAMPLE_ADC1_CHAN0, value));
    ESP_LOGI(TAG_ADC, "ADC%d Channel[%d] Raw Data: %d", ADC_UNIT_1 + 1, EXAMPLE_ADC1_CHAN0, *value);
    // Tear Down
    ESP_ERROR_CHECK(adc_oneshot_del_unit(adc1_handle));
}

void http_post_data(void *pvParameters)
{
    // JSON data to be sent in the POST request
    char *post_data = (char *)pvPortMalloc(MAX_POST_SIZE + 41);
    if (post_data == NULL)
    {
        ESP_LOGE(TAG_HTTP, "Failed to allocate memory for post_data");
        // Xử lý lỗi khác (thoát hoặc thực hiện các hành động khác)
        return;
    }
    // Run the HTTP POST JSON example
    while (1)
    {
        if (writeStage)
        {
            printf("thoi diem bat dau gui: \n");
            snprintf(post_data, MAX_POST_SIZE + 40, "{\"device_id\":\"%d\",\"data\":\"%s\"}", DEVICE_ID, temp[tempPostIndex]);
            // printf("%s\n", post_data);
            if (http_post_json_handle(post_data) == 0)
            {
                writeStage = 0;
            }
            printf("thoi diem gui xong:\n--------------------------\n");
        }

        vTaskDelay(TASK1_DELAY / portTICK_PERIOD_MS);
    }
    vPortFree(post_data);
}

void http_get_data(void *pvParameters)
{
    while (1)
    {
        http_get_handle();
        if (logs == 1)
        {
            if ((eTaskGetState(task_adc_oneshot_write) == eSuspended))
            {
                vTaskResume(task_http_post_data);
                vTaskResume(task_adc_oneshot_write);
                ESP_LOGI(TAG_HTTP, "LOG START");
            }
        }
        else if (eTaskGetState(task_adc_oneshot_write) == eBlocked)
        {
            vTaskSuspend(task_http_post_data);
            vTaskSuspend(task_adc_oneshot_write);
            ESP_LOGI(TAG_HTTP, "LOG STOP");
        }
        vTaskDelay(TASK0_DELAY / portTICK_PERIOD_MS);
    }
}

void adc_oneshot_write(void *pvParameters)
{
    int count = 0;
    uint8_t tempWriteIndex = 0;
    while (1)
    {

        int value;
        oneshot_adc_read(&value);
        uint32_t current_time_miliseconds = esp_log_timestamp();
        sprintf(temp[tempWriteIndex] + strlen(temp[tempWriteIndex]), "?%ld&%d", current_time_miliseconds, value);
        count++;
        printf("%d", count);
        if (count == VALUE_PER_POST)
        {
            count = 0;
            writeStage = 1;
            tempWriteIndex++;
            tempPostIndex = tempWriteIndex - 1;
            if (tempWriteIndex == 10)
            {
                tempWriteIndex = 0;
            }
            *temp[tempWriteIndex] = '\0';
        }
        vTaskDelay(TASK2_DELAY / portTICK_PERIOD_MS);
        ESP_LOGI("ADC_WRITE", "ADC write done");
    }
}

void app_main(void)
{

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());

    xTaskCreatePinnedToCore(http_get_data, "Task0", 4096, NULL, 1, &task_http_get_data, 0);
    // xTaskCreate(http_get_data, "HTTP_GET", 4096, NULL, 1, NULL);
    xTaskCreatePinnedToCore(http_post_data, "Task1", 6144, NULL, 2, &task_http_post_data, 0);
    xTaskCreatePinnedToCore(adc_oneshot_write, "Task2", 4096, NULL, 1, &task_adc_oneshot_write, 1);
    vTaskSuspend(task_http_post_data);
    vTaskSuspend(task_adc_oneshot_write);
    // vTaskDelete(task_http_post_data);
}
