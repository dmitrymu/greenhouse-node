#pragma once

#include <stdbool.h>

void wifi_init();

void wifi_run();

void wifi_scan();

bool wifi_connect(const char *ssid, const char *pass);

void wifi_print_status();

bool wifi_wait_for_connection(int timeoutMS);
