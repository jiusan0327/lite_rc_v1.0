#ifndef __ADC_H_
#define __ADC_H_

#pragma once
#include "driver/adc.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"  
#include "sdkconfig.h"
#include "driver/gpio.h"


#define ADC_RIGHT_X_PIN      ADC1_CHANNEL_5   //GPIO 6
#define ADC_RIGHT_Y_PIN      ADC1_CHANNEL_6   //GPIO 7
#define ADC_LEFT_X_PIN       ADC1_CHANNEL_7   //GPIO 8
#define ADC_LEFT_Y_PIN       ADC1_CHANNEL_1   //GPIO 2
#define ADC_ATTEN           ADC_ATTEN_DB_11  // 0-3.3V量程
#define ADC_BIT_WIDTH       ADC_BITWIDTH_12  // 12位分辨率
#define SAMPLE_NUM          64               // 滑动平均采样数
#define READ_LEN            256              // DMA读取长度
#define MAX_STORE_BUF_SIZE  2048             // DMA缓冲区大小
esp_err_t joystick_init(void);
void joystick_task(void *pvParameters);
float joystick_get_value(uint8_t ch);



#endif

