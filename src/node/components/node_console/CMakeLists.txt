idf_component_register(
    SRCS "cmd_wifi.c"
         "cmd_sys.c"
         "cmd_nvs.c"
         "node_console.c"
    INCLUDE_DIRS "include"
    REQUIRES console 
             node_network
             nvs_flash
             freertos
             spi_flash)
