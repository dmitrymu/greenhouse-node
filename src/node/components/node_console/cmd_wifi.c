#include "cmd_wifi.h"
#include "node_wifi.h"

#include "argtable3/argtable3.h"
#include "esp_log.h"
#include "esp_console.h"

/** Arguments used by 'join' function */
static struct {
//    struct arg_int *timeout;
    struct arg_str *ssid;
    struct arg_str *password;
    struct arg_end *end;
} join_args;


static int cmd_wifi_connect(int argc, char **argv)
{
    int nerrors = arg_parse(argc, argv, (void **) &join_args);
    if (nerrors != 0) {
        arg_print_errors(stderr, join_args.end, argv[0]);
        return 1;
    }
    ESP_LOGI(__func__, "Connecting to '%s'",
             join_args.ssid->sval[0]);

    bool status = wifi_connect(join_args.ssid->sval[0],
                               join_args.password->sval[0]);
    return status ? 0 : 1;
}

static int cmd_wifi_scan(int argc, char **argv)
{
    wifi_scan();
    return 0;
}


void register_wifi(void)
{
//    join_args.timeout = arg_int0(NULL, "timeout", "<t>", "Connection timeout, ms");
    join_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID of AP");
    join_args.password = arg_str0(NULL, NULL, "<pass>", "PSK of AP");
    join_args.end = arg_end(2);

    const esp_console_cmd_t join_cmd = {
        .command = "wifi.connect",
        .help = "Join WiFi AP as a station",
        .hint = NULL,
        .func = &cmd_wifi_connect,
        .argtable = &join_args
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&join_cmd) );

    const esp_console_cmd_t list_cmd = {
        .command = "wifi.list",
        .help = "List available WiFi AP",
        .hint = NULL,
        .func = &cmd_wifi_scan,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&list_cmd) );

    const esp_console_cmd_t log_cmd = {
        .command = "wifi.status",
        .help = "Log WiFi connection status",
        .hint = NULL,
        .func = &wifi_print_status
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&log_cmd) );
}
