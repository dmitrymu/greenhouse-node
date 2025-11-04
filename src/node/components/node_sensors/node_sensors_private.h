#pragma once
/**
 * Sensor list interface.
 * 
 * All functions should be called after sensors_start().
*/
#include "node_sensors.h"

/**
 * Lock sensors list for thread-safe update.
 * 
 * Always must be followed by node_sensors_unlock()!
*/
bool
node_sensors_lock();


/**
 * Unlock sensors list after update.
*/
void
node_sensors_unlock();

/**
 * Add the sensor to the list.
 * 
 * Caller is responsible for locking and unlocking.
*/
bool
node_sensor_add(node_sensor_t * sensor);

/**
 * Remove the sensor from the list.
 * 
 * Caller is responsible for locking and unlocking.
*/
bool
node_sensor_remove(node_sensor_t * sensor);
