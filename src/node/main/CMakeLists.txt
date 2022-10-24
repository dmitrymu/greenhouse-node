set(
  COMPONENT_REQUIRES console
                     esp32-ds18b20
                     esp32-owb
                     ) 

idf_component_register(
  SRCS "main.c"
#       "console.c"
  INCLUDE_DIRS ".")