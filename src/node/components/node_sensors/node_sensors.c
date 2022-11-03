#include <assert.h>
#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "node_sensors.h"
#include "node_1wire.h"

static node_sensor_t* sensors_head = NULL;
static node_sensor_t* sensors_tail = NULL;

static SemaphoreHandle_t sensors_lock;
static StaticSemaphore_t sensors_mutex;

enum sensors_const_internal
{
    SENSORS_MUTEX_LOCK_TIMEOUT_MS = 10
};


void
node_sensors_start()
{
    sensors_lock = xSemaphoreCreateMutexStatic(&sensors_mutex);
    assert(sensors_lock != NULL);

    sensors_1wire_start();
}


bool
node_sensors_lock()
{
    return xSemaphoreTake(sensors_lock,
                          SENSORS_MUTEX_LOCK_TIMEOUT_MS / portTICK_PERIOD_MS)
           == pdTRUE;
}

void
node_sensors_unlock()
{
    xSemaphoreGive(sensors_lock);
}

const node_sensor_t *
node_sensor_enum_start()
{
    if (node_sensors_lock())
    {
        return sensors_head;
    }
    return NULL;
}

void
node_sensor_enum_finish()
{
    node_sensors_unlock();
}

bool
node_sensor_add(node_sensor_t * sensor)
{
    sensor->next = NULL;

    if (sensors_head == NULL)
    {
        sensors_head = sensor;
    }
    else
    {
        sensors_tail->next = sensor;
    }

    sensors_tail = sensor;
    return true;
}


bool
node_sensor_remove(node_sensor_t * sensor)
{
    if (sensors_head == NULL)
    {
        /* Empty sensor list. */
        return false;
    }

    if (sensors_head == sensor)
    {   /* Special case - found ar head. */
        sensors_head = sensor->next;
        sensor->next = NULL;
        return true;
    }

    node_sensor_t * prev = sensors_head;
    while (prev != NULL && prev->next != sensor)
    {
        prev = prev->next;
    }

    if (prev == NULL)
    {
        /* Not found in the list. */
        return false;
    }

    /* At this point prev->next == sensor */
    prev->next = sensor->next;
    sensor->next = NULL;
    return true;
}
