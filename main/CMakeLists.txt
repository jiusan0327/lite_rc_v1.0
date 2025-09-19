idf_component_register(SRCS "main.c" 
                    INCLUDE_DIRS "."
                        REQUIRES
                        displayer_driver
                        pca9555
                        buzz
                        driver
                        uart_TR
                        ADC
                        esp_system
                        rx_ble
                        bt
                        user_ui
                    )
