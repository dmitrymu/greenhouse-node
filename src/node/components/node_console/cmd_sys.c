
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "esp_console.h"
#include "esp_spi_flash.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void register_cmd_sys();
static int sys_version();
static int sys_tasks();

static const char *TAG = "cmd_system";

static struct {
    struct arg_str *op;
    struct arg_end *end;
} sys_args;

void register_system(void)
{
    register_cmd_sys();
}

static int cmd_sys(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &sys_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, sys_args.end, argv[0]);
        return 1;
    }

    const char *op = sys_args.op->sval[0];

    if (strcasecmp(op, "free") == 0) {
        printf("Free heap %d bytes\n", esp_get_free_heap_size());
    } else if (strcasecmp(op, "heap") == 0) {
        uint32_t heap_size = heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
        printf("Min heap size: %u bytes\n", heap_size);
    } else if (strcasecmp(op, "version") == 0) {
        return sys_version();
    } else if (strcasecmp(op, "tasks") == 0) {
        return sys_tasks();
    } else if (strcasecmp(op, "restart") == 0) {
        ESP_LOGI(TAG, "Restarting");
        esp_restart();
    } else {
        printf("Unsupported system command '%s'\r\n", op);
        return 1;
    }

    return 0;
}

static void register_cmd_sys()
{
    sys_args.op = arg_str0(NULL, NULL, "<op>", "operation: free/heap/version/tasks/restart");
    sys_args.end = arg_end(1);

    const esp_console_cmd_t cmd = {
        .command = "sys",
        .help = "Execute system command\n"
        "sys free - Show size of free heap memory\n"
        "sys heap - Show min heap size\n"
        "sys version - Show version of chip and SDK\n"
        "sys tasks - Show information about running tasks\n"
        "sys restart - Software reset of the chip\n",
        .hint = NULL,
        .func = &cmd_sys,
        .argtable = &sys_args,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );
}

/* 'version' command */
static int sys_version()
{
    esp_chip_info_t info;
    esp_chip_info(&info);
    printf("IDF Version:%s\r\n", esp_get_idf_version());
    printf("Chip info:\r\n");
    printf("\tmodel:%s\r\n", info.model == CHIP_ESP32 ? "ESP32" : "Unknown");
    printf("\tcores:%d\r\n", info.cores);
    printf("\tfeature:%s%s%s%s%d%s\r\n",
           info.features & CHIP_FEATURE_WIFI_BGN ? "/802.11bgn" : "",
           info.features & CHIP_FEATURE_BLE ? "/BLE" : "",
           info.features & CHIP_FEATURE_BT ? "/BT" : "",
           info.features & CHIP_FEATURE_EMB_FLASH ? "/Embedded-Flash:" : "/External-Flash:",
           spi_flash_get_chip_size() / (1024 * 1024), " MB");
    printf("\trevision number:%d\r\n", info.revision);
    return 0;
}

/** 'tasks' command prints the list of tasks and related information */

static int sys_tasks()
{
    const size_t bytes_per_task = 40; /* see vTaskList description */
    char *task_list_buffer = malloc(uxTaskGetNumberOfTasks() * bytes_per_task);
    if (task_list_buffer == NULL) {
        ESP_LOGE(TAG, "failed to allocate buffer for vTaskList output");
        return 1;
    }
    fputs("Task Name\tStatus\tPrio\tHWM\tTask#", stdout);
    fputs("\tAffinity", stdout);
    fputs("\n", stdout);
    vTaskList(task_list_buffer);
    fputs(task_list_buffer, stdout);
    free(task_list_buffer);
    return 0;
}
