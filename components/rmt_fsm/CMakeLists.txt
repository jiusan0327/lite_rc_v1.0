idf_component_register(SRCS "rmt_fsm.c"
                    INCLUDE_DIRS "include"
                    REQUIRES
                    driver
                    ADC
                    buzz
                    displayer_driver
                    pca9555
                    uart_TR
                    esp_system
                    rx_ble
                    )
