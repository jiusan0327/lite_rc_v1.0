idf_component_register(SRCS "ADC.c"
                    INCLUDE_DIRS "include"
                    REQUIRES
                    driver
                    esp_adc
                    esp_system
                    uart_TR)
