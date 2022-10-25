set(
  COMPONENT_REQUIRES console
                     esp32-ds18b20
                     esp32-owb
                     freertos
                     ) 

idf_component_register(
  SRCS "main.c"
       "console.c"
       "onewire.c" 
  INCLUDE_DIRS ".")