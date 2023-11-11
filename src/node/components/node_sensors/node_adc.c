#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "node_adc.h"
#include "node_network.h"
#include "node_sensors_private.h"

//ADC Channels
#define ADC1_EXAMPLE_CHAN0          ADC1_CHANNEL_0

//ADC Attenuation
#define ADC_EXAMPLE_ATTEN           ADC_ATTEN_DB_11

//ADC Calibration
#define ADC_EXAMPLE_CALI_SCHEME     ESP_ADC_CAL_VAL_EFUSE_VREF

static const char *TAG = "ADC";

typedef struct sensor_adc
{
    node_sensor_t generic;          /**< Generic sensor descriptor. */
} sensor_adc_t;

static sensor_adc_t sensors_adc[] = 
{
    {
        {
            .name = "ADC_1_0",
            .quantity = "moisture",
            .unit = "%"
        }
    }
};

static esp_adc_cal_characteristics_t adc1_chars;

static bool adc_calibration_init(void)
{
    esp_err_t ret;
    bool cali_enable = false;

    ret = esp_adc_cal_check_efuse(ADC_EXAMPLE_CALI_SCHEME);
    if (ret == ESP_ERR_NOT_SUPPORTED) {
        ESP_LOGW(TAG, "Calibration scheme not supported, skip software calibration");
    } else if (ret == ESP_ERR_INVALID_VERSION) {
        ESP_LOGW(TAG, "eFuse not burnt, skip software calibration");
    } else if (ret == ESP_OK) {
        cali_enable = true;
        esp_adc_cal_characterize(ADC_UNIT_1, ADC_EXAMPLE_ATTEN, ADC_WIDTH_BIT_DEFAULT, 0, &adc1_chars);
    } else {
        ESP_LOGE(TAG, "Invalid arg");
    }

    return cali_enable;
}

static float vWet = 1700.0;
static float vDry = 2800.0;

static TaskHandle_t adc_task = NULL;

void sensors_adc_task()
{
    if (!adc_calibration_init())
    {
        ESP_LOGE(TAG, "Cannot read calibration data from eFuse. ADC disabled");
        vTaskDelete(adc_task);
    }
    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(ADC1_EXAMPLE_CHAN0, ADC_EXAMPLE_ATTEN));

    while (!node_sensors_lock())
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    node_sensor_add(&sensors_adc[0].generic);

    node_sensors_unlock();


    while (true)
    {
        int adc_raw = adc1_get_raw(ADC1_EXAMPLE_CHAN0);
        //ESP_LOGI(TAG, "raw  data: %d", adc_raw);
        float voltage = esp_adc_cal_raw_to_voltage(adc_raw, &adc1_chars);
        float moisture = 100.0 * (1.0-fmax(fmin((voltage - vWet) / (vDry - vWet), 1.0), 0.0));
        //ESP_LOGI(TAG, "cali data: %5f mV, %3.1f %%", voltage, moisture);
        node_sensor_t* sensor = &sensors_adc[0].generic;
        node_mqtt_send_sensor_value(
            sensor->name,
            sensor->quantity,
            sensor->unit,
            moisture
        );
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void sensors_adc_start()
{
    adc_task = xTaskCreate(&sensors_adc_task, "adc_task", 8192, NULL, 5, NULL);
}
