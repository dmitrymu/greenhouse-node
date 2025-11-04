#include <stdio.h>
#include "cmd_sensors.h"
#include "esp_log.h"
#include "esp_console.h"
#include "node_sensors.h"


static int cmd_sensors_list(int argc, char **argv)
{
    for (node_sensor_t *sensor = node_sensor_enum_start();
         sensor != NULL;
         sensor = sensor->next)
    {
        printf("%s: %s(%s)\r\n",
               sensor->name,
               sensor->quantity,
               sensor->unit);
    }
    node_sensor_enum_finish();
    return 0;
}


void register_sensors()
{

    const esp_console_cmd_t list_cmd = {
        .command = "sensors.list",
        .help = "List all available sensors",
        .hint = NULL,
        .func = &cmd_sensors_list,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&list_cmd) );
}
