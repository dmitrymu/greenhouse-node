#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "mqtt_client.h"
#include "node_mqtt.h"
#include "node_wifi.h"

static const char *TAG = "mqtt";

static EventGroupHandle_t mqtt_event_group;

enum mqtt_const_internal
{
    CONNECTED_BIT = BIT0,
    DEFAULT_CONNECT_TIMEOUT_MS = 3000
};

static
esp_mqtt_client_config_t mqtt_cfg = {
    // .host = "192.168.240.2",
    // .port = 1883
    .uri = "mqtt://192.168.240.2:1883/"
};

enum mqtt_cont_internal
{
    MQTT_QUEUE_LENGTH = 8,
    MQTT_QUEUE_READ_MS = 1000
};

static QueueHandle_t mqtt_queue_handle;
static StaticQueue_t mqtt_queue;
static uint8_t mqtt_queue_buf[ MQTT_QUEUE_LENGTH * sizeof(mqtt_message_t) ];

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        xEventGroupSetBits(mqtt_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        xEventGroupClearBits(mqtt_event_group, CONNECTED_BIT);
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        //ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_task(void* data)
{
    while (!wifi_wait_for_connection(DEFAULT_CONNECT_TIMEOUT_MS))
    {
        ESP_LOGW(TAG, "Waiting for wifi connection");
    }

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    // Now run message publish loop
    while (true)
    {
        mqtt_message_t msg;
        if (xQueueReceive(mqtt_queue_handle,
                          &(msg),
                          MQTT_QUEUE_READ_MS/portTICK_PERIOD_MS) == pdPASS)
        {
            esp_mqtt_client_publish(client, msg.topic, msg.data, 0, 1, 0);
        }
    }
}

void mqtt_start()
{
    mqtt_event_group = xEventGroupCreate();
    mqtt_queue_handle = xQueueCreateStatic(MQTT_QUEUE_LENGTH,
                                           sizeof(mqtt_message_t),
                                           mqtt_queue_buf,
                                           &mqtt_queue);
    xTaskCreate(&mqtt_task, "mqtt_task", 8192, NULL, 5, NULL);
}

bool mqtt_wait_for_connection(int timeoutMS)
{
    int bits = xEventGroupWaitBits(mqtt_event_group,
                                   CONNECTED_BIT,
                                   pdFALSE,
                                   pdTRUE,
                                   timeoutMS / portTICK_PERIOD_MS);
    return (bits & CONNECTED_BIT) != 0;
}

void mqtt_send_message(const mqtt_message_t *msg)
{
    BaseType_t rc = xQueueSend(mqtt_queue_handle,
                               (void *)msg,
                               (TickType_t)0);
    if (rc != pdTRUE)
    {
         ESP_LOGE(TAG, "Queue is full");
   }
}