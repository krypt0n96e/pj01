#include "pir.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/adc_types_legacy.h"

static esp_adc_cal_characteristics_t adc1_chars;
static esp_adc_cal_characteristics_t adc2_chars;

void pir_init_adc_channel_1(adc1_channel_t num)
{
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);

    adc1_config_width(ADC_WIDTH_BIT_DEFAULT);
    adc1_config_channel_atten(num, ADC_ATTEN_DB_11);

}

void pir_init_adc_channel_2(adc2_channel_t num){
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_DEFAULT, 0, &adc2_chars);
    adc2_config_channel_atten(num, ADC_ATTEN_DB_11);
}