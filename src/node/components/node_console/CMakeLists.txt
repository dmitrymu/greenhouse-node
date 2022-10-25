idf_component_register(
    SRCS "cmd_sys.c"
         "node_console.c"
    INCLUDE_DIRS "include"
    REQUIRES console 
             freertos
             spi_flash)
