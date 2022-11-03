idf_component_register(
  SRCS "main.c"
  INCLUDE_DIRS "."
  REQUIRES node_console
           node_network
           node_sensors
           esp32-ds18b20
           esp32-owb
           nvs_flash
           freertos)