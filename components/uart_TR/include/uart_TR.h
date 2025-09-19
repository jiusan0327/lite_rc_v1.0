#ifndef __UART_TR_H_
#define __UART_TR_H_

#pragma once
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "memory.h"

#define UART_PORT_NUM      UART_NUM_1      // 使用UART1
#define BUF_SIZE           128*2            // DMA缓冲区大小（需为128的倍数）
#define RD_BUF_SIZE        BUF_SIZE
#define TXD_PIN            GPIO_NUM_17
#define RXD_PIN            GPIO_NUM_18
#define DMA_QUEUE_LENGTH   4              // DMA队列长度

#define Ling_TR_SET_PIN   GPIO_NUM_20
#define Ling_TR_CS_PIN    GPIO_NUM_19

#define Ling_TR_CS_LOW    gpio_set_level(Ling_TR_CS_PIN, 0)
#define Ling_TR_CS_HIGH   gpio_set_level(Ling_TR_CS_PIN, 1)
#define Ling_TR_SET_LOW   gpio_set_level(Ling_TR_SET_PIN, 0)
#define Ling_TR_SET_HIGH  gpio_set_level(Ling_TR_SET_PIN, 1)





//帧头2字节0xAA55+数据体+2字节crc16_ccitt校验
typedef struct _rc_data_
{
   int16_t ch[4]; //摇杆通道数据
   uint16_t key_state; //按键状态
   uint8_t rc_mode; //遥控模式
   uint16_t robot_id;//遥控器ID
   uint8_t reserve[3];//make sure 4 byte 
}rc_data_t;

extern rc_data_t rc_data;   

//ling TR通讯协议中的CMD部分
//命令字节(HEX)0x56=重启 0x57=恢复出厂 0x58回应恢复 0x59查询版本 0x5A设置参数 0x5B回应设置 0x5C查询参数 0x5D回应查询
typedef enum {
    REBOOT = 0x56,
    RESTORE_FACTORY = 0x57,
    ACK_RESTORE = 0x58,
    QUERY_VERSION = 0x59,
    SET_PARAM = 0x5A,
    ACK_SET = 0x5B,
    QUERY_PARAM = 0x5C,
    ACK_QUERY = 0x5D
} Ling_TR_CMD_e;

//RF Power 1Byte(0~30分别对应-10至+20db的发射)，默认20dbm即对应参数为30
typedef enum{
    RF_POWER_m10dBm = 0,
    RF_POWER_m9dBm = 1,
    RF_POWER_m8dBm = 2,
    RF_POWER_m7dBm = 3,
    RF_POWER_m6dBm = 4,
    RF_POWER_m5dBm = 5,
    RF_POWER_m4dBm = 6,
    RF_POWER_m3dBm = 7,
    RF_POWER_m2dBm = 8,
    RF_POWER_m1dBm = 9,
    RF_POWER_0dBm = 10,
    RF_POWER_1dBm = 11,
    RF_POWER_2dBm = 12,
    RF_POWER_3dBm = 13,
    RF_POWER_4dBm = 14,
    RF_POWER_5dBm = 15,
    RF_POWER_6dBm = 16,
    RF_POWER_7dBm = 17,
    RF_POWER_8dBm = 18,
    RF_POWER_9dBm = 19,
    RF_POWER_10dBm = 20,
    RF_POWER_11dBm = 21,
    RF_POWER_12dBm = 22,
    RF_POWER_13dBm = 23,
    RF_POWER_14dBm = 24,
    RF_POWER_15dBm = 25,
    RF_POWER_16dBm = 26,
    RF_POWER_17dBm = 27,
    RF_POWER_18dBm = 28,
    RF_POWER_19dBm = 29,
    RF_POWER_20dBm = 30
} Ling_TR_RF_Power_e;

//RF 信道 1Byte(0~40),每500kHZ为一个信道，即423.92至443.92MHZ
typedef enum{
    RF_CHANNEL_423_92MHz = 0,
    RF_CHANNEL_424_42MHz = 1,
    RF_CHANNEL_424_92MHz = 2,
    RF_CHANNEL_425_42MHz = 3,
    RF_CHANNEL_425_92MHz = 4,
    RF_CHANNEL_426_42MHz = 5,
    RF_CHANNEL_426_92MHz = 6,
    RF_CHANNEL_427_42MHz = 7,
    RF_CHANNEL_427_92MHz = 8,
    RF_CHANNEL_428_42MHz = 9,
    RF_CHANNEL_428_92MHz = 10,
    RF_CHANNEL_429_42MHz = 11,
    RF_CHANNEL_429_92MHz = 12,
    RF_CHANNEL_430_42MHz = 13,
    RF_CHANNEL_430_92MHz = 14,
    RF_CHANNEL_431_42MHz = 15,
    RF_CHANNEL_431_92MHz = 16,
    RF_CHANNEL_432_42MHz = 17,
    RF_CHANNEL_432_92MHz = 18,
    RF_CHANNEL_433_42MHz = 19,
    RF_CHANNEL_433_92MHz = 20,
    RF_CHANNEL_434_42MHz = 21,
    RF_CHANNEL_434_92MHz = 22,
    RF_CHANNEL_435_42MHz = 23,
    RF_CHANNEL_435_92MHz = 24,
    RF_CHANNEL_436_42MHz = 25,
    RF_CHANNEL_436_92MHz = 26,
    RF_CHANNEL_437_42MHz = 27,
    RF_CHANNEL_437_92MHz = 28,
    RF_CHANNEL_438_42MHz = 29,
    RF_CHANNEL_438_92MHz = 30,
    RF_CHANNEL_439_42MHz = 31,
    RF_CHANNEL_439_92MHz = 32,
    RF_CHANNEL_440_42MHz = 33,
    RF_CHANNEL_440_92MHz = 34,
    RF_CHANNEL_441_42MHz = 35,
    RF_CHANNEL_441_92MHz = 36,
    RF_CHANNEL_442_42MHz = 37,
    RF_CHANNEL_442_92MHz = 38,
    RF_CHANNEL_443_42MHz = 39,
    RF_CHANNEL_443_92MHz = 40
} Ling_TR_RF_Channel_e;

//Baud 1Byte(0~6)波特率,分别对应600/1200/2400/4800/9600/19200/38400,默认9600即4
typedef enum{
    BAUD_600 = 0,
    BAUD_1200 = 1,
    BAUD_2400 = 2,
    BAUD_4800 = 3,
    BAUD_9600 = 4,
    BAUD_19200 = 5,
    BAUD_38400 = 6
} Ling_TR_Baud_e;
void uart_TR_config(Ling_TR_CMD_e cmd, uint16_t robot_id, Ling_TR_Baud_e uart_baud, Ling_TR_RF_Channel_e rf_channel,Ling_TR_RF_Power_e rf_power_level);
bool is_setting_ok(void);
void clear_setting_flag(void);
uint16_t crc16_ccitt(const uint8_t *buf, uint16_t len);
void uart_TR_init_dma(int baud_rate);
void uart_send_data_dma(const uint8_t *data, size_t len);
void uart_receive_task_dma(void *pvParameters);
void uart_TR_send_Task(void *pvParameters);









#endif // UART_TR_H
