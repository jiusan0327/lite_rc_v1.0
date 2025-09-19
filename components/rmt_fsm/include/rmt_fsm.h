#ifndef __RMT_FSM_H__
#define __RMT_FSM_H__

#include "driver/gpio.h"    
#include "esp_log.h"
#include "esp_err.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "st7735s.h"
#include "buzz.h"
#include "pca9555.h"
#include "uart_TR.h"
#include "ADC.h"
#include "rx_ble.h"


void rmt_fsm_init(void);
void rmt_fsm_task(void *pvParameters);


typedef enum  {
    RMT_FSM_INIT = 0,
    RMT_FSM_IDLE,
    RMT_FSM_ACTIVE,
    RMT_FSM_ERROR,
} rmt_fsm_state_t;

void nvs_save_rc_robot(uint8_t rc_mode, uint16_t robot_id);
void nvs_read_rc_robot(uint8_t *rc_mode, uint8_t *robot_id);
#endif // __RMT_FSM_H__