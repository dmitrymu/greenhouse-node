#pragma once

#include <stdbool.h>

/**
 * Public interface of the network layer.
 * 
 * Node uses MQTT as a pub/sub messaging protocol.
 */
/**
 * Constants for message structure 
 */
enum mqtt_const
{
    MQTT_MAX_TOPIC_LEN = 128,   /** Topic buffer length */
    MQTT_MAX_DATA_LEN = 128     /** Data buffer length */
};

/**
 * MQTT message to be published.
 * 
 * Message contains:
 * - topic string which is used by subscribers to filter messages;
 * - data blob, for Node it's always JSON text.
 * 
*/
typedef struct mqtt_message
{
    char topic[MQTT_MAX_TOPIC_LEN]; /** Topic buffer */
    char data[MQTT_MAX_DATA_LEN];   /** Data buffer */
} mqtt_message_t;                   /** Alias for message structure */

/**
 * Start network layer.
 * 
 * After start network layer is not necessary ready.  Return value indicates
 * whether WiFi was successfully connected. If not, console command is necessary
 * to provide credentials.
 * 
 * @return true if WiFi connected to AP, false otherwise.
*/
bool node_network_start();

/**
 * Wait for network ready to transport messages.
 * 
 * @timeoutMS   wait timeout in milliseconds
 * @return true if network layer is ready for messaging, false otherwise
*/
bool node_network_ready_wait(int timeoutMS);


void node_mqtt_send_message(const mqtt_message_t* msg);
