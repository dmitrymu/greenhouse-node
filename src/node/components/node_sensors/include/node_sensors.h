#pragma once
/**
 * Interface to all sensors supported by the Node
*/
#include <stdbool.h>

/**
 * Constants for sensors component.
*/
enum node_sensors_const
{
    NODE_SENSORS_MAX_NAME_LEN = 32  /**< Sensor name length limit. */
};

typedef struct node_sensor node_sensor_t;
/**
 * Sensor descriptor
*/
struct node_sensor
{
    node_sensor_t* next;    /**< Next sensor (NULL for last in the list). */
    const char* name;       /**< Sensor name */
    const char* quantity;   /**< Sensor quantity (e.g. temperature) */
    const char* unit;       /**< Sensor unit (e.g. degrees C)*/
};

/**
 * Initialize all sensors subsystems, start tasks.
 * 
 * Sensors will wait for network subsystem to start publishing data.
*/
void
node_sensors_start();

/**
 * Start enumeration of the sensors.
 * 
 * The function locks sensor list so any updates are disabled.
*/
const node_sensor_t*
node_sensor_enum_start();

/**
 * Finish enumeration of the sensors.
 * 
 * The function unlocks sensor list.
*/
void
node_sensor_enum_finish();
