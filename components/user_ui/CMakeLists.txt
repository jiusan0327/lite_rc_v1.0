idf_component_register(SRCS "user_ui.c"
                    INCLUDE_DIRS "include"
                    REQUIRES
                    nvs_flash
                    driver
                    displayer_driver
                    pca9555
                    buzz
                    rmt_fsm
                    )
