#include "pca9555.h"
#include "uart_TR.h"



static const char *TAG = "PCA9555";
extern rc_data_t rc_data;
// 初始化I2C控制器
static esp_err_t i2c_master_init(void) {
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    esp_err_t err = i2c_param_config(I2C_MASTER_NUM, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "i2c_param_config failed: %s", esp_err_to_name(err));
        return err;
    }
    return i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);
}

// 写入寄存器
esp_err_t pca9555_write_register(uint8_t addr, PCA9555_REGISTER reg, uint8_t value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Write failed (reg 0x%02X): %s", reg, esp_err_to_name(err));
    }
    return err;
}

// 读取寄存器
esp_err_t pca9555_read_register(uint8_t addr, PCA9555_REGISTER reg, uint8_t *value) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd); // Repeated start
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, true);
    i2c_master_read_byte(cmd, value, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t err = i2c_master_cmd_begin(I2C_MASTER_NUM, cmd, pdMS_TO_TICKS(I2C_MASTER_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Read failed (reg 0x%02X): %s", reg, esp_err_to_name(err));
    }
    return err;
}

// 设置引脚模式 (输入/输出)
esp_err_t pca9555_set_pin_mode(uint8_t addr, PCA9555_PORT port, uint8_t pin, bool is_output) {
    if (pin > 7) {
        ESP_LOGE(TAG, "Invalid pin number %d", pin);
        return ESP_ERR_INVALID_ARG;
    }

    PCA9555_REGISTER reg = (port == PCA9555_PORT_0) ? PCA9555_DIRECTION_0 : PCA9555_DIRECTION_1;
    uint8_t data;
    
    esp_err_t err = pca9555_read_register(addr, reg, &data);
    if (err != ESP_OK) return err;
    
    if (is_output) {
        data &= ~(1 << pin); // 0=输出
    } else {
        data |= (1 << pin);  // 1=输入
    }
    
    return pca9555_write_register(addr, reg, data);
}

// 设置引脚电平
esp_err_t pca9555_set_pin_level(uint8_t addr, PCA9555_PORT port, uint8_t pin, bool level) {
    if (pin > 7) {
        ESP_LOGE(TAG, "Invalid pin number %d", pin);
        return ESP_ERR_INVALID_ARG;
    }

    PCA9555_REGISTER reg = (port == PCA9555_PORT_0) ? PCA9555_OUTPUT_0 : PCA9555_OUTPUT_1;
    uint8_t data;
    
    esp_err_t err = pca9555_read_register(addr, reg, &data);
    if (err != ESP_OK) return err;
    
    if (level) {
        data |= (1 << pin);  // 设置高电平
    } else {
        data &= ~(1 << pin); // 设置低电平
    }
    
    return pca9555_write_register(addr, reg, data);
}

// 初始化PCA9555
void pca9555_init(void) {
    if (i2c_master_init() != ESP_OK) {
        ESP_LOGE(TAG, "I2C initialization failed");
        return;
    }
    // 初始化所有端口为输入模式
    for (uint8_t port = 0; port <= 1; port++) {
        for (uint8_t pin = 0; pin < 8; pin++) {
            pca9555_set_pin_mode(PCA9555_ADDRESS, port, pin, false);
        }
    }
    //极性问题
    pca9555_write_register(PCA9555_ADDRESS, 0x04, 0x00);
    pca9555_write_register(PCA9555_ADDRESS, 0x05, 0x00);
    vTaskDelay(100 / portTICK_PERIOD_MS); // 确保初始化完成
    xTaskCreate(pca9555_scan_task, "pca9555_scan_task", 4086, NULL, 5, NULL); // 创建扫描任务
    rc_data.key_state  = 0xffff; // 初始化按键状态
}

QueueHandle_t key_event_queue = NULL;

void Fun_short_press(int key_id) {
    if (key_event_queue) {
        key_event_q_t evt = { .key_id = key_id, .event_type = KEY_EVENT_SHORT };
        xQueueSend(key_event_queue, &evt, 0); // 非阻塞发送
    }
}

void Fun_long_press(int key_id) {
    if (key_event_queue) {
        key_event_q_t evt = { .key_id = key_id, .event_type = KEY_EVENT_LONG };
        xQueueSend(key_event_queue, &evt, 0); // 非阻塞发送
    }
}

void Fun_double_click(int key_id) {
    if (key_event_queue) {
        key_event_q_t evt = { .key_id = key_id, .event_type = KEY_EVENT_DOUBLE };
        xQueueSend(key_event_queue, &evt, 0); // 非阻塞发送
    }
}



// 端口扫描任务
void pca9555_scan_task(void *pvParameters) {
    uint8_t port0_data, port1_data;
    static key_info_t keys[VAILD_KEY_NUM];
    if (key_event_queue == NULL) {
        key_event_queue = xQueueCreate(KEY_EVENT_QUEUE_LEN, sizeof(key_event_q_t));
        if (key_event_queue == NULL) {
            ESP_LOGE("KEY", "Failed to create key event queue!");
        }
    }
    while (1) {
        if (pca9555_read_register(PCA9555_ADDRESS, PCA9555_INPUT_0, &port0_data) == ESP_OK &&
            pca9555_read_register(PCA9555_ADDRESS, PCA9555_INPUT_1, &port1_data) == ESP_OK) {
            rc_data.key_state = ((port0_data<<8)|port1_data)^0xffff;
        }
        process_key_events(rc_data.key_state, keys,VAILD_KEY_NUM);
        vTaskDelay(pdMS_TO_TICKS(SCAN_INTERVAL_MS)); // 每秒扫描一次
    }
    
    vTaskDelete(NULL);
}




bool key_check_long_press(int i, key_info_t *keys) {
    return (keys[i].counter >= LONG_PRESS_MS);
}
bool key_check_double_click(int i, key_info_t *keys) {
    if (keys[i].waiting_double && keys[i].last_release_time < DOUBLE_CLICK_MS) {
        keys[i].waiting_double = false;  // 已经触发，不再等待
        return true;
    }
    return false;
}
bool key_check_short_press(int i, key_info_t *keys) {
    // 必须不是长按
    return (keys[i].counter < LONG_PRESS_MS);
}

void process_key_events(uint16_t key_raw, key_info_t *keys,uint16_t key_num) {
    for (int i = 0; i < key_num; i++) {
        uint8_t pressed = (key_raw >> i) & 0x01;

        if (pressed) {
            if (keys[i].state == 0) {
                keys[i].state = 1;
                keys[i].counter = 0;
            } else {
                keys[i].counter += SCAN_INTERVAL_MS;
            }
        } else {
            if (keys[i].state == 1) {
                // 松开
                if (key_check_long_press(i, keys)) {
                   Fun_long_press(i);
                } else if (key_check_double_click(i, keys)) {
                   Fun_double_click(i);
                } else {
                    keys[i].waiting_double = true;
                    keys[i].last_release_time = 0;
                }
                keys[i].state = 0;
            }

            // 单击检测（双击超时）
            if (keys[i].waiting_double) {
                keys[i].last_release_time += SCAN_INTERVAL_MS;
                if (keys[i].last_release_time >= DOUBLE_CLICK_MS) {
                    if (key_check_short_press(i, keys)) {
                        Fun_short_press(i);
                    }
                    keys[i].waiting_double = false;
                }
            }
        }
    }
}