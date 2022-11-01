idf_component_register(
    SRCS "cmd_wifi.c"
         "cmd_sys.c"
         "node_console.c"
    INCLUDE_DIRS "include"
    REQUIRES console 
             node_network
             freertos
             spi_flash)
