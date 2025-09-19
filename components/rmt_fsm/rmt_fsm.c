#include "rmt_fsm.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"
#include "uart_TR.h"
extern rc_data_t rc_data;
void nvs_init_custom(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
}

void nvs_save_rc_robot(uint8_t rc_mode, uint16_t robot_id)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    // 打开 NVS 命名空间 "storage"，写模式
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }
    // 写入 rc_mode
    err = nvs_set_u8(my_handle, "rc_mode", rc_mode);
    if (err != ESP_OK) ESP_LOGE("NVS", "Failed to write rc_mode");
    // 写入 robot_id
    err = nvs_set_u16(my_handle, "robot_id", robot_id);
    if (err != ESP_OK) ESP_LOGE("NVS", "Failed to write robot_id");
    // 提交写入
    err = nvs_commit(my_handle);
    if (err != ESP_OK) ESP_LOGE("NVS", "Failed to commit NVS");
    nvs_close(my_handle);
}

void nvs_read_rc_robot(uint8_t *rc_mode, uint8_t *robot_id)
{
    nvs_handle_t my_handle;
    esp_err_t err;
    err = nvs_open("storage", NVS_READONLY, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Error (%s) opening NVS handle!", esp_err_to_name(err));
        return;
    }
    err = nvs_get_u8(my_handle, "rc_mode", rc_mode);
    if (err != ESP_OK) *rc_mode = 0; // 默认值
    err = nvs_get_u16(my_handle, "robot_id", robot_id);
    if (err != ESP_OK) *robot_id = 0; // 默认值
    nvs_close(my_handle);
}

rmt_fsm_state_t current_state;

void rmt_fsm_init(void)
{   
    nvs_init_custom();
    nvs_read_rc_robot(&rc_data.rc_mode, &rc_data.robot_id );
    ESP_LOGI("RMT_FSM","rc_mode=%x, robot_id=%x\n", rc_data.rc_mode, rc_data.robot_id);
    current_state = RMT_FSM_IDLE;
}

void rmt_fsm_task(void *pvParameters)
{
    switch (current_state)
    {
        case RMT_FSM_INIT:
            // Handle initialization state
            ESP_LOGI("RMT_FSM", "State: INIT");

            current_state = RMT_FSM_ACTIVE;
            break;

        case RMT_FSM_IDLE:
            // Handle idle state
             // 等待配置事件，配置后进入INIT状态，不然就依据现有
            ESP_LOGI("RMT_FSM", "State: IDLE");
            // Transition to active state or wait for events
            current_state = RMT_FSM_INIT;
            break;

        case RMT_FSM_ACTIVE:
            // Handle active state
           
            ESP_LOGI("RMT_FSM", "State: ACTIVE");
            break;

        case RMT_FSM_ERROR:
            // Handle error state
            ESP_LOGE("RMT_FSM", "State: ERROR");
            current_state = RMT_FSM_INIT; // Reset to init state on error
            break;

        default:
            ESP_LOGE("RMT_FSM", "Unknown State");
            break;
    }
    
}

