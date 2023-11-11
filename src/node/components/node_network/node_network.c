#include <stdio.h>
#include <string.h>>
#include "node_network.h"
#include "node_wifi.h"
#include "node_mqtt.h"


bool node_network_start()
{
    wifi_init();
    mqtt_start();
    return wifi_run();
}

/**
 * Wait for network ready to transport messages.
 * 
 * @timeoutMS   wait timeout in milliseconds
 * @return true if network layer is ready for messaging, false otherwise
*/
bool node_network_ready_wait(int timeoutMS)
{
    return mqtt_wait_for_connection(timeoutMS);
}

void node_mqtt_send_sensor_value(const char *name,
                                 const char *quantity,
                                 const char *unit,
                                 float value)
{
    mqtt_message_t msg;
    bzero(&msg, sizeof(msg));
    snprintf(msg.topic,
             sizeof(msg.topic),
             "nodes/node1/%s/%s",
             quantity,
             name);
    snprintf(msg.data,
             sizeof(msg.data),
             "{\"value\": %.1f, \"unit\": \"%s\"}",
             value,
             unit);
    node_mqtt_send_message(&msg);
}

void node_mqtt_send_message(const mqtt_message_t* msg)
{
    mqtt_send_message(msg);
}
