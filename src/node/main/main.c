#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "node_console.h"
#include "node_network.h"
#include "node_sensors.h"
#include "nvs_flash.h"

static void initialize_nvs(void)
{
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

void app_main(void)
{
  static const char *tag = "main";

  initialize_nvs();

  if (!node_network_start())
  {
    ESP_LOGW(tag, "Network requires WiFi credentials");
  }

  node_sensors_start();

  console_run();

  ESP_LOGI(tag, "app_main exit.");
}
