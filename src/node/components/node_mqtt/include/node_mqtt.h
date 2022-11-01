#pragma once

enum mqtt_const
{
    MQTT_MAX_TOPIC_LEN = 128,
    MQTT_MAX_DATA_LEN = 128
};

typedef struct mqtt_message
{
    char topic[MQTT_MAX_TOPIC_LEN];
    char data[MQTT_MAX_DATA_LEN];
} mqtt_message_t;


void mqtt_start();

void mqtt_send_message(const mqtt_message_t* msg);

bool mqtt_wait_for_connection(int timeoutMS);
