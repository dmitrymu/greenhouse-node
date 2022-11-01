#pragma once

#include "node_network.h"

void mqtt_start();

void mqtt_send_message(const mqtt_message_t* msg);

bool mqtt_wait_for_connection(int timeoutMS);
