#include <stdio.h>
#include "ADC.h"
#include "esp_adc/adc_continuous.h"
#include <string.h>
#include <math.h>
#include "uart_TR.h"
extern rc_data_t rc_data;
static const char *TAG = "JOYSTICK";
// 摇杆数据结构
typedef struct {
    int32_t filter_sum;      // 滤波累加值
    uint16_t sample_count;   // 当前采样计数
    int16_t filtered_val;    // 滤波后值
    float normalized_val;    // 归一化值[-1.0,1.0]
} joystick_channel_t;

static joystick_channel_t joystick_data[4] = {0};
static adc_continuous_handle_t adc_handle = NULL;

// 初始化ADC连续采样
esp_err_t joystick_init(void)
{
    // 配置ADC句柄
    adc_continuous_handle_cfg_t handle_cfg = {
        .max_store_buf_size = MAX_STORE_BUF_SIZE,
        .conv_frame_size = READ_LEN,
    };
   adc_continuous_new_handle(&handle_cfg, &adc_handle);

    // 配置ADC通道
    adc_digi_pattern_config_t adc_pattern[4] = {0};
    const adc_channel_t channels[] = {
        ADC_RIGHT_X_PIN, ADC_RIGHT_Y_PIN, 
        ADC_LEFT_X_PIN, ADC_LEFT_Y_PIN
    };
    
    for (int i = 0; i < 4; i++) {
        adc_pattern[i] = (adc_digi_pattern_config_t){
            .atten = ADC_ATTEN,
            .channel = channels[i],
            .unit = ADC_UNIT_1,  // 使用ADC1避免与WiFi/蓝牙冲突
            .bit_width = ADC_BIT_WIDTH,
        };
    }

    // ADC连续采样配置
    adc_continuous_config_t config = {
        .pattern_num = 4,
        .adc_pattern = adc_pattern,
        .sample_freq_hz = 20 * 1000,  // 20kHz总采样率
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    };
    adc_continuous_config(adc_handle, &config);

    return ESP_OK;
}

// 处理ADC数据
static void process_adc_data(uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i += SOC_ADC_DIGI_RESULT_BYTES) {
        adc_digi_output_data_t *p = (adc_digi_output_data_t*)&data[i];
        uint8_t ch = p->type2.channel;
        uint16_t val = p->type2.data;

        // 确定通道索引
        int idx = -1;
        if (ch == (ADC_RIGHT_X_PIN & 0x7)) idx = 0;
        else if (ch == (ADC_RIGHT_Y_PIN & 0x7)) idx = 1;
        else if (ch == (ADC_LEFT_X_PIN & 0x7)) idx = 2;
        else if (ch == (ADC_LEFT_Y_PIN & 0x7)) idx = 3;

        if (idx >= 0) {
            // 滑动平均滤波
            joystick_data[idx].filter_sum += val;
            joystick_data[idx].sample_count++;
            
            // 完成一组采样
            if (joystick_data[idx].sample_count >= SAMPLE_NUM) {
                // 计算平均值并中心归零
                joystick_data[idx].filtered_val = (joystick_data[idx].filter_sum / SAMPLE_NUM) - 2048;
                // 归一化到[-1.0, 1.0]
                joystick_data[idx].normalized_val = joystick_data[idx].filtered_val / 2048.0f;
                
                // 重置滤波状态
                joystick_data[idx].filter_sum = 0;
                joystick_data[idx].sample_count = 0;
            }
        }
    }
}

// 摇杆任务
void joystick_task(void *pvParameters)
{
    uint8_t result[READ_LEN] = {0};
    uint32_t ret_num = 0;
    
    // 初始化ADC
    if (joystick_init() != ESP_OK) {
        ESP_LOGE(TAG, "ADC init failed!");
        vTaskDelete(NULL);
        return;
    }
    
    // 启动ADC
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
    ESP_LOGI(TAG, "Joystick ADC started");

    while (1) {
        // 读取ADC数据
        esp_err_t ret = adc_continuous_read(adc_handle, result, READ_LEN, &ret_num, 100);
        if (ret == ESP_OK && ret_num > 0) {
            process_adc_data(result, ret_num);
            
            // 调试输出（实际应用中可降低频率或移除）
            static uint32_t count = 0;
            if (++count % 10 == 0) {  // 每10次采样输出一次
                // ESP_LOGI(TAG, "R_X:%.2f R_Y:%.2f L_X:%.2f L_Y:%.2f",
                //         joystick_data[0].normalized_val,
                //         joystick_data[1].normalized_val,
                //         joystick_data[2].normalized_val,
                //         joystick_data[3].normalized_val);
                for (size_t i = 0; i < 4; i++)
                {
                    rc_data.ch[i] = joystick_data[i].filtered_val;
                }
            }
        } else if (ret == ESP_ERR_TIMEOUT) {
            // 无数据时短暂延时
            vTaskDelay(pdMS_TO_TICKS(1));
        } else {
            ESP_LOGE(TAG, "ADC read error: 0x%x", ret);
        }
    }
}

// 获取摇杆归一化值
float joystick_get_value(uint8_t ch) 
{
    if (ch >= 4) return 0.0f;
    return joystick_data[ch].normalized_val;
}
