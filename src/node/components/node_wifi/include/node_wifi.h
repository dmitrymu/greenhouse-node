#pragma once

void wifi_init();

void wifi_run();

void wifi_scan();

bool wifi_connect(const char *ssid, const char *pass);
