#include <assert.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "owb.h"
#include "owb_rmt.h"
#include "ds18b20.h"
#include "node_network.h"
#include "node_sensors_private.h"

enum sensors_1wire_const_internal
{
    SENSORS_1WIRE_GPIO = GPIO_NUM_21,
    SENSORS_1WIRE_MAX_DEVICES = 16,
    SENSORS_1WIRE_DS18B20_FAMILY_CODE = 0x28,
    SENSORS_1WIRE_DS18B20_RESOLUTION = DS18B20_RESOLUTION_12_BIT,
    SENSORS_1WIRE_SAMPLE_PERIOD_MS = 1000
};

/**
 * Sensor structure specific for 1-wire.
*/
typedef struct sensors_1wire
{
    node_sensor_t generic;          /**< Generic sensor descriptor. */
    OneWireBus_ROMCode rom_code;    /**< Device ROM code. */
} sensors_1wire_t;

static const char *TAG = "1wire";

static OneWireBus *owb;
static owb_rmt_driver_info rmt_driver_info;

/** number of sensors attached to the bus. */
static int sensors_1wire_count = 0;

    /** Array of sensor descriptors. */
static sensors_1wire_t sensors_1wire[SENSORS_1WIRE_MAX_DEVICES];

/** Array of sensor names. */
static char sensors_1wire_names[SENSORS_1WIRE_MAX_DEVICES]
                               [NODE_SENSORS_MAX_NAME_LEN];

static const char* SENSORS_1WIRE_DS18B20_QUANTITY = "temperature";
static const char* SENSORS_1WIRE_DS18B20_UNIT = "\\u00b0C";


void
sensors_1wire_bus_init()
{
    /* Make sure that sensor list functions will work. */
    _Static_assert(offsetof(struct sensors_1wire, generic) == 0,
                   "sensors_1wire_t is not properly aligned");

    // Create a 1-Wire bus, using the RMT timeslot driver
    owb = owb_rmt_initialize(&rmt_driver_info,
                             SENSORS_1WIRE_GPIO,
                             RMT_CHANNEL_0,
                             RMT_CHANNEL_1);
    owb_use_crc(owb, true); // enable CRC check for ROM code
}

bool
sensors_1wire_reset()
{
    if (!node_sensors_lock())
    {
        return false;
    }

    for (int n = 0; n < sensors_1wire_count; ++n)
    {
        node_sensor_remove(&sensors_1wire[n].generic);
    }

    node_sensors_unlock();

    sensors_1wire_count = 0;
    bzero(sensors_1wire, sizeof(sensors_1wire));
    bzero(sensors_1wire_names, sizeof(sensors_1wire_names));
    return true;
}

bool
sensors_1wire_add_to_list()
{
    if (!node_sensors_lock())
    {
        return false;
    }

    for (int n = 0; n < sensors_1wire_count; ++n)
    {
        node_sensor_add(&sensors_1wire[n].generic);
    }

    node_sensors_unlock();
    return true;
}

bool sensors_1wire_find_devices()
{
    if (!sensors_1wire_reset())
    {
        return false;
    }

    OneWireBus_SearchState search_state = {0};
    bool found = false;
    owb_search_first(owb, &search_state, &found);
    while (found)
    {
        owb_string_from_rom_code(search_state.rom_code,
                                 sensors_1wire_names[sensors_1wire_count],
                                 sizeof(sensors_1wire_names[sensors_1wire_count]));

        sensors_1wire[sensors_1wire_count].generic.name = sensors_1wire_names + sensors_1wire_count;
        sensors_1wire[sensors_1wire_count].rom_code = search_state.rom_code;

        if (sensors_1wire[sensors_1wire_count].rom_code.fields.family[0] == SENSORS_1WIRE_DS18B20_FAMILY_CODE)
        {
            sensors_1wire[sensors_1wire_count].generic.quantity = SENSORS_1WIRE_DS18B20_QUANTITY;
            sensors_1wire[sensors_1wire_count].generic.unit = SENSORS_1WIRE_DS18B20_UNIT;
            ++sensors_1wire_count;
        }
        else
        {
            ESP_LOGW(TAG, "Skipped unknown 1-wire device %s", (const char*)(sensors_1wire_names + sensors_1wire_count));
        }

        owb_search_next(owb, &search_state, &found);
    }

    if (!sensors_1wire_add_to_list())
    {
        return false;
    }

    return true;
}

bool
sensors_1wire_DS18B20_read()
{
    if (!node_sensors_lock())
    {
        return false;
    }

    DS18B20_Info devices[SENSORS_1WIRE_MAX_DEVICES];
    bzero(devices, sizeof(devices));
    for (int n = 0; n < sensors_1wire_count; ++n)
    {
        DS18B20_Info* sensor = devices + n;
        ds18b20_init(sensor, owb, sensors_1wire[n].rom_code); // associate with bus and device
        ds18b20_use_crc(sensor, true); // enable CRC check on all reads
        ds18b20_set_resolution(sensor, SENSORS_1WIRE_DS18B20_RESOLUTION);
    }

    node_sensors_unlock();

    ds18b20_convert_all(owb);

    // In this application all devices use the same resolution,
    // so use the first device to determine the delay
    ds18b20_wait_for_conversion(&devices[0]);

    // Read the results immediately after conversion otherwise it may fail
    // (using printf before reading may take too long)
    float readings[SENSORS_1WIRE_MAX_DEVICES] = {0};
    DS18B20_ERROR errors[SENSORS_1WIRE_MAX_DEVICES] = {0};

    for (int i = 0; i < sensors_1wire_count; ++i)
    {
        errors[i] = ds18b20_read_temp(&devices[i], &readings[i]);
    }

    // Print results in a separate loop, after all have been read
    int errors_count = 0;
    for (int i = 0; i < sensors_1wire_count; ++i)
    {
        if (errors[i] == DS18B20_OK)
        {
            node_mqtt_send_sensor_value(
                sensors_1wire[i].generic.name,
                sensors_1wire[i].generic.quantity,
                sensors_1wire[i].generic.unit,
                readings[i]
            );
        }
        else
        {
            ++errors_count;
        }
    }

    return errors_count == 0;
}


void sensors_1wire_task()
{
  sensors_1wire_bus_init();  
  
  while(true)
  {
    TickType_t last_wake_time = xTaskGetTickCount();
    while(!node_network_ready_wait(1000))
    {
      /* MQTT not ready */
    }

    while(!sensors_1wire_find_devices())
    {
        /* Semaphore is busy or no devicws*/
        vTaskDelayUntil(&last_wake_time, SENSORS_1WIRE_SAMPLE_PERIOD_MS / portTICK_PERIOD_MS);
    }

    while(sensors_1wire_DS18B20_read())
    {
        vTaskDelayUntil(&last_wake_time, SENSORS_1WIRE_SAMPLE_PERIOD_MS / portTICK_PERIOD_MS);
    }
  }
}

void sensors_1wire_start()
{
    xTaskCreate(&sensors_1wire_task, "1wire_task", 8192, NULL, 5, NULL);
}
