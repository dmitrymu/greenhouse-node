#include <stdio.h>
#include <strings.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "node_status.h"

static const char *TAG = "status";

enum wifi_status_t
{
    WIFI_CONFIGURED = 0x01,
    WIFI_CONNECTED = 0x02,
};

static
struct node_status 
{
    SemaphoreHandle_t lock;
    int wifi_status;
} node;

static StaticSemaphore_t mutex;

static const int MUTEX_LOCK_TIMEOUT_MS = 10;

static void set_bit(int* mem, int bit, bool status)
{
    if (status) {
        *mem |= bit;
    } else {
        *mem &= ~bit;
    }
}

static bool get_bit(int* mem, int bit)
{
    return (*mem & bit) != 0;
}

void safe_set_bit(int* mem, int bit, bool status)
{
    if (xSemaphoreTake(node.lock, MUTEX_LOCK_TIMEOUT_MS / portTICK_PERIOD_MS) ) {
        set_bit(mem, bit, status);
        xSemaphoreGive( node.lock );
    } else {
        ESP_LOGE(TAG, "Cannot take status mutex in %d ms", MUTEX_LOCK_TIMEOUT_MS);
    }
}

bool safe_get_bit(int* mem, int bit)
{
    if (xSemaphoreTake(node.lock, MUTEX_LOCK_TIMEOUT_MS / portTICK_PERIOD_MS) ) {
        bool status = get_bit(mem, bit);
        xSemaphoreGive( node.lock );
        return status;
    } else {
        ESP_LOGE(TAG, "Cannot take status mutex in %d ms", MUTEX_LOCK_TIMEOUT_MS);
        return false;
    }
}


void node_status_init()
{
    bzero(&node, sizeof(node));
    node.lock = xSemaphoreCreateMutexStatic(&mutex);
}


void set_wifi_configured(bool status)
{
    safe_set_bit(&node.wifi_status, WIFI_CONFIGURED, status);
}

bool get_wifi_configured()
{
    return safe_get_bit(&node.wifi_status, WIFI_CONFIGURED);
}

void set_wifi_connected(bool status)
{
    safe_set_bit(&node.wifi_status, WIFI_CONNECTED, status);
}

bool get_wifi_connected()
{
    return safe_get_bit(&node.wifi_status, WIFI_CONNECTED);
}
