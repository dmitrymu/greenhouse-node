idf_component_register(SRCS "node_mqtt.c"
                    INCLUDE_DIRS "include"
                    REQUIRES mqtt
                             node_wifi)
