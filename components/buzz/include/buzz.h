#ifndef __BUZZ_H
#define __BUZZ_H



#include <stdint.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <memory.h>


#define BUZZ_PIN           (GPIO_NUM_11)


#define BUZZ_Clr gpio_set_level(BUZZ_PIN,0)//SCL=SCLK
#define BUZZ_Set gpio_set_level(BUZZ_PIN,1)
void buzz_init(void);
void buzz_on(uint32_t duration_ms);
void buzz_off(void);
#endif
