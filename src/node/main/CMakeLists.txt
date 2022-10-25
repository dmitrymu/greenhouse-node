set(
  COMPONENT_REQUIRES console
                     node_console
                     esp32-ds18b20
                     esp32-owb
                     freertos
                     ) 

idf_component_register(
  SRCS "main.c"
       "onewire.c" 
  INCLUDE_DIRS "."
  REQUIRES node_console
           esp32-ds18b20
           esp32-owb
           nvs_flash
           freertos)