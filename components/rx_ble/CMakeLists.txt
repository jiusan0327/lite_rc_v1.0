idf_component_register(SRCS "rx_ble.c" 
                    INCLUDE_DIRS "include"
                    REQUIRES
                    driver
                    nvs_flash
                    bt
                    
                    )
