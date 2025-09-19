#ifndef __PCA9555_H
#define __PCA9555_H  
#pragma once   
#include <stdint.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <memory.h>

#define PCA9555_ADDRESS  0b00100100 // Default I2C address for PCA9555 :010 0100
#define I2C_MASTER_SDA_IO           5
#define I2C_MASTER_SCL_IO           4
#define I2C_MASTER_NUM              I2C_NUM_0
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TIMEOUT_MS       1000



#define VAILD_KEY_NUM          16  // 按键数量
#define SCAN_INTERVAL_MS  50  // 扫描间隔
#define LONG_PRESS_MS    1000 // 长按时间阈值
#define SHORT_PRESS_MS   200 // 短按时间阈值
#define DOUBLE_CLICK_MS   300 // 双击时间间隔


typedef enum {
    PCA9555_PORT_0 = 0,
    PCA9555_PORT_1 = 1
} PCA9555_PORT;

typedef enum {
    PCA9555_DIRECTION_0 = 0x06,
    PCA9555_DIRECTION_1 = 0x07,
    PCA9555_OUTPUT_0 = 0x02,
    PCA9555_OUTPUT_1 = 0x03,
    PCA9555_INPUT_0 = 0x00,
    PCA9555_INPUT_1 = 0x01
} PCA9555_REGISTER;



typedef enum {
    KEY_EVENT_NONE = 0,   // 无事件
    KEY_EVENT_SHORT,      // 短按
    KEY_EVENT_LONG,       // 长按
    KEY_EVENT_LONG_HOLD,  // 长按保持（持续触发）
    KEY_EVENT_DOUBLE,     // 双击（可选）
    KEY_EVENT_RELEASE     // 松开
} key_event_t;


typedef struct {
    int key_id;            // 按键编号
    key_event_t event_type; // 事件类型
} key_event_q_t;

typedef struct {
    uint8_t state;       // 当前按下状态（0=未按，1=按下）
    uint16_t counter;    // 按下时间计数，单位 = SCAN_INTERVAL
    key_event_t event;   // 本次扫描得到的事件
    uint16_t last_release_time; // 上一次释放到现在的时间
    bool waiting_double;  // 是否在等待双击
} key_info_t;


#define KEY_EVENT_QUEUE_LEN 16
extern QueueHandle_t key_event_queue;
void pca9555_init(void);
esp_err_t pca9555_write_register(uint8_t addr, PCA9555_REGISTER reg, uint8_t value);
esp_err_t pca9555_read_register(uint8_t addr, PCA9555_REGISTER reg, uint8_t *value);
esp_err_t pca9555_set_pin_mode(uint8_t addr, PCA9555_PORT port, uint8_t pin, bool is_output);
esp_err_t pca9555_set_pin_level(uint8_t addr, PCA9555_PORT port, uint8_t pin, bool level);
esp_err_t pca9555_set_port_direction(uint8_t addr, PCA9555_PORT port, uint8_t direction);
esp_err_t pca9555_set_port_output(uint8_t addr, PCA9555_PORT port, uint8_t value);
void pca9555_scan_task(void *pvParameters);

void process_key_events(uint16_t key_raw, key_info_t *keys,uint16_t key_num);
#endif
