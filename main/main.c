#include <stdio.h>
#include <stdint.h>
#include "rmt_fsm.h"
#include "user_ui.h"


void app_main(void)
{       
    rx_spp_ble_init();
    vTaskDelay(100 / portTICK_PERIOD_MS); 
    rmt_fsm_init(); // Initialize the FSM
    buzz_init(); // Initialize the buzzer
    pca9555_init(); // Initialize the PCA9555 I/O expander
    xTaskCreate(joystick_task, "joystick_task", 4096, NULL, 5, NULL); // Create joystick task
    uart_TR_init_dma(9600); // Initialize UART with DMA at 9600 baud rate
    vTaskDelay(100 / portTICK_PERIOD_MS); // Short delay to ensure UART
    user_ui_init(); // Finalize user interface initialization
    while (1)
    {   
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
