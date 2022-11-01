idf_component_register(
    SRCS "node_mqtt.c"
         "node_wifi.c"
         "node_network.c"
    INCLUDE_DIRS "include"
    REQUIRES mqtt)
