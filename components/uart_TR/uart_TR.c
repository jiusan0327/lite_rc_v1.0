
#include "uart_TR.h"
#include "esp_log.h"
#include "user_ui.h"
rc_data_t rc_data;
#define TAG                "UART_TR"
void uart_TR_init_dma(int baud_rate) {
    // UART配置
    uart_config_t uart_config = {
        .baud_rate = baud_rate,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // 安装UART驱动（启用DMA）
    ESP_ERROR_CHECK(uart_driver_install(
        UART_PORT_NUM,
        BUF_SIZE * 2,  // DMA接收缓冲区大小（双缓冲）
        BUF_SIZE * 2,  // DMA发送缓冲区大小
        DMA_QUEUE_LENGTH, // DMA事件队列长度
        NULL,          // 不使用事件队列
        0             // 不分配中断标志
    ));
    // 配置参数
    ESP_ERROR_CHECK(uart_param_config(UART_PORT_NUM, &uart_config));
    // 设置引脚
    ESP_ERROR_CHECK(uart_set_pin(
        UART_PORT_NUM,
        TXD_PIN,
        RXD_PIN,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE
    ));
    // 配置引脚
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = 1ULL<<Ling_TR_CS_PIN;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    Ling_TR_CS_LOW;
    // 启用DMA模式（可选，默认已启用）
    uart_set_mode(UART_PORT_NUM, UART_MODE_UART);
    vTaskDelay(100 / portTICK_PERIOD_MS); // 确保UART初始化完成
    xTaskCreate(uart_TR_send_Task, "uart_TR_send_Task", 2048, NULL, 5, NULL); // 创建发送任务
    xTaskCreate(uart_receive_task_dma, "uart_receive_task_dma", 2048, NULL, 5, NULL); // 创建接收任务

}

void uart_send_data_dma(const uint8_t *data, size_t len) {
    // DMA方式发送数据
    int sent = uart_write_bytes(UART_PORT_NUM, (const char *)data, len);
    if (sent < 0) {
        ESP_LOGE(TAG, "Send failed");
    }
}
static bool setting_ok = false;
void uart_receive_task_dma(void *pvParameters) {
    static uint8_t data[RD_BUF_SIZE];
    while (1) {
        // 非阻塞读取（DMA缓冲）
        int len = uart_read_bytes(
            UART_PORT_NUM,
            data,
            RD_BUF_SIZE,
            pdMS_TO_TICKS(100)  // 超时100ms
        );

        if (len > 0) {
            if(data[0]==0xAA && data[1]==0x5B)
            {
                setting_ok = true;
            }
            // ESP_LOGI(TAG, "Received %d bytes:", len);
            // for (int i = 0; i < len; i++) {
            //     printf("%02X ", data[i]);
            // }
            // printf("\n");
        }
    }
   
}
bool is_setting_ok(void)
{
    return setting_ok;
}

void clear_setting_flag(void)
{
    setting_ok = false;
}
void uart_TR_deinit(void) {
    uart_driver_delete(UART_PORT_NUM);
}

void uart_TR_send_Task(void *pvParameters) {
    uint16_t buf_size = 0;
    buf_size = sizeof(rc_data)+4; // 4字节头尾 
    uint8_t head_h = 0xAA;
    uint8_t head_l = 0x55;
    uint8_t crc_h = 0x00;
    uint8_t crc_l = 0x00;
    while (1) {
        uint8_t protobuf[buf_size];
        protobuf[0] = head_h;
        protobuf[1] = head_l;
        memcpy(&protobuf[2], &rc_data, sizeof(rc_data));
        uint16_t crc = crc16_ccitt((const uint8_t *)&rc_data, sizeof(rc_data));
        crc_h = (crc >> 8) & 0xFF;
        crc_l = crc & 0xFF;
        protobuf[buf_size - 2] = crc_h;
        protobuf[buf_size - 1] = crc_l;
        if(!send_settings())
        {
           uart_send_data_dma(protobuf, buf_size);
        }
        vTaskDelay(50 / portTICK_PERIOD_MS); // 50ms发送一次        
    }
}

uint16_t crc16_ccitt(const uint8_t *buf, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++)
    {
        crc ^= (uint16_t)buf[i] << 8;
        for (uint8_t j = 0; j < 8; j++)
        {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

uint8_t checksum(uint8_t *data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return sum;
}
//设置模式
void uart_TR_Set_Mode(void)
{
    vTaskDelay(200 / portTICK_PERIOD_MS); 
    gpio_reset_pin(Ling_TR_SET_PIN); 
    gpio_set_direction(Ling_TR_SET_PIN, GPIO_MODE_OUTPUT); 
    Ling_TR_SET_LOW;
    vTaskDelay(200 / portTICK_PERIOD_MS); 
}

void uart_TR_Trans_Mode(void)
{
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    Ling_TR_SET_HIGH;
    gpio_reset_pin(Ling_TR_SET_PIN); 
    gpio_set_direction(Ling_TR_SET_PIN, GPIO_MODE_INPUT); 
    vTaskDelay(200 / portTICK_PERIOD_MS); 
}

//主机发送0xaa+0x5a+0000+组网 ID+0x00+RF 发射功率+0x00+串口速率 +0x00+RF 信道选择+0x00+0x00+0x12（字节长度）+0x00+和校验字节
void uart_TR_config(Ling_TR_CMD_e cmd, uint16_t robot_id, Ling_TR_Baud_e uart_baud, Ling_TR_RF_Channel_e rf_channel,Ling_TR_RF_Power_e rf_power_level)
{
    

    uart_TR_Set_Mode();
    uint8_t config_data[18];
    config_data[0] = 0xAA; // 帧头1
    config_data[1] = 0x5A; // 帧头2
    config_data[2] = 0x00; // 保留
    config_data[3] = 0x00; // 保留
    config_data[4] = (robot_id >> 8) & 0xFF; // 组网ID高字节
    config_data[5] = robot_id & 0xFF;        // 组网ID低字节
    config_data[6] = 0x00;                   // 保留
    config_data[7] = rf_power_level;               // RF发射功率
    config_data[8] = 0x00;                   // 保留
    config_data[9] = uart_baud;              // 串口速率
    config_data[10] = 0x00;  
    config_data[11] = rf_channel;             // RF信道选择
    config_data[12] = 0x00;                   // 保留
    config_data[13] = 0x00;                   // 保留
    config_data[14] = 0x00;                  // 数据长度
    config_data[15] = 0x12;                   // 保留
    config_data[16] = 0x00;                   
    ESP_LOGI(TAG, "Config Data:");
    uint8_t crc = 0;
    crc = checksum(config_data, 17);
    config_data[17] = crc;
    for (int i = 0; i < sizeof(config_data); i++) {
        printf("%02X ", config_data[i]);
    }
    printf("\n");
    // 发送配置数据包
    uart_send_data_dma(config_data, sizeof(config_data));
    uart_TR_Trans_Mode();
}
