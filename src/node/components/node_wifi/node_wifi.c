#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "node_status.h"
#include "node_wifi.h"

static const char *TAG = "wifi";

static esp_netif_t *sta_netif = NULL;
static EventGroupHandle_t wifi_event_group;
const int CONNECTED_BIT = BIT0;
const int DEFAULT_CONNECT_TIMEOUT_MS = 3000;

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        set_wifi_configured(true);
        set_wifi_connected(false);
        ESP_LOGW(TAG, "Disconnected");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "Connected");
        set_wifi_connected(true);
    }
}

void wifi_init()
{
    esp_log_level_set("wifi", ESP_LOG_WARN);
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    ESP_ERROR_CHECK(esp_wifi_start());
}

static bool wifi_connect_internal(wifi_config_t *config)
{
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, config));
    esp_wifi_connect();

    return wifi_wait_for_connection(DEFAULT_CONNECT_TIMEOUT_MS);
}

bool wifi_wait_for_connection(int timeoutMS)
{
    int bits = xEventGroupWaitBits(
        wifi_event_group,
        CONNECTED_BIT,
        pdFALSE,
        pdTRUE,
        timeoutMS / portTICK_PERIOD_MS);
    return (bits & CONNECTED_BIT) != 0;
}

bool wifi_connect(const char *ssid, const char *pass)
{
    wifi_config_t config = {0};
    strlcpy((char *)config.sta.ssid, ssid, sizeof(config.sta.ssid));
    if (pass)
    {
        strlcpy((char *)config.sta.password, pass, sizeof(config.sta.password));
    }

    return wifi_connect_internal(&config);
}

void wifi_run()
{
    wifi_config_t config;
    esp_err_t ret = esp_wifi_get_config(ESP_IF_WIFI_STA, &config);
    if (ret == ESP_OK)
    {
        if (wifi_connect_internal(&config))
        {
            set_wifi_configured(true);
        }
    }
    else
    {
        set_wifi_configured(false);
        ESP_LOGW(TAG, "Wifi configuration not found in flash memory");
    }
}

void wifi_scan()
{
    static const int DEFAULT_SCAN_LIST_SIZE = 16;

    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    bzero(ap_info, sizeof(ap_info));

    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    printf("%u APs scanned. Limited to %d ", ap_count, DEFAULT_SCAN_LIST_SIZE);
    printf("SSID\tRSSI\tChannel\r\n");
    for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
    {
        printf("%s\t%d\t%d\r\n", ap_info[i].ssid, ap_info[i].rssi, ap_info[i].primary);
    }
}

void wifi_print_status()
{
    bool configured = get_wifi_configured();
    bool connected = get_wifi_connected();
    printf("Wifi status: %sconfigured, %sconnected\r\n",
           configured ? "" : "not ",
           connected ? "" : "not ");

    if (sta_netif == NULL)
    {
        ESP_LOGE(TAG, "Cannot obtain station interface");
        return;
    }

    if (!esp_netif_is_netif_up(sta_netif))
    {
        ESP_LOGW(TAG, "Station interface is down");
    }

    esp_netif_ip_info_t ip;
    ESP_ERROR_CHECK(esp_netif_get_ip_info(sta_netif, &ip));
    printf("IPv4 address:" IPSTR "; ", IP2STR(&ip.ip));
    printf("netmask:\t\t" IPSTR "; ", IP2STR(&ip.netmask));
    printf("gateway:\t\t" IPSTR, IP2STR(&ip.gw));
    printf("\r\n");
}
