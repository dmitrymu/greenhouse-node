idf_component_register(
    SRCS "node_sensors.c"
         "node_1wire.c"
         "node_adc.c"
    INCLUDE_DIRS "include"
    REQUIRES esp32-ds18b20
             esp32-owb
             esp_adc_cal                
             #driver
             node_network
             freertos)