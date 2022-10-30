#pragma once

#include <stdbool.h>

void node_status_init();

void set_wifi_configured(bool status);
bool get_wifi_configured();
void set_wifi_connected(bool status);
bool get_wifi_connected();
