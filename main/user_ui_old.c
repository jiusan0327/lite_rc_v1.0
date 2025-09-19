#include "user_ui.h"
#include "rmt_fsm.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "pca9555.h"
#include "uart_TR.h"
#include "rmt_fsm.h"
static uint8_t ui_page = 0; // Current UI page
static bool settings_sent = false; // Flag to indicate if settings have been sent

extern rc_data_t rc_data;
void action_exit(void);
MenuItem menu[] = {
    {"Robot ID", MENU_ITEM_TYPE_UINT16,&rc_data.robot_id , 0, 0xFFFF, NULL},
    {"Mode",     MENU_ITEM_TYPE_UINT8,  &rc_data.rc_mode,  0, 1,   NULL},
    {"Exit",     MENU_ITEM_TYPE_ACTION, NULL,      0, 0,    action_exit}
};
void page1(void);
void page2(void);
void page3(void);
void page4(void);
void action_exit(void) {
    LCD_Clear(COLOR_BLACK);
    settings_sent = true; // Reset the flag when exiting
    ui_page = 2;
    rc_data.rc_mode = menu[1].value;
    rc_data.robot_id = menu[0].value;
    
    uart_TR_config(SET_PARAM, rc_data.robot_id, BAUD_9600, RF_CHANNEL_433_92MHz, RF_POWER_20dBm);
    page4(); // 显示设置中页面
}








uint8_t menu_length = sizeof(menu) / sizeof(menu[0]);
uint8_t current_index = 0; // 当前选中的菜单项

void menu_draw(void) {

    for (uint8_t i = 0; i < menu_length; i++) {
        uint16_t y = 5 + i * 20;
        uint16_t color = (i == current_index) ? COLOR_YELLOW : COLOR_WHITE;

        LCD_ShowString(10, y, (uint8_t*)menu[i].name, color, COLOR_BLACK, 16, 0);

        if (menu[i].type == MENU_ITEM_TYPE_UINT16) {
            LCD_ShowIntNum(100, y, *(uint16_t*)menu[i].value, 4, color, COLOR_BLACK, 16);
        } else if (menu[i].type == MENU_ITEM_TYPE_UINT8) {
            LCD_ShowIntNum(100, y, *(uint16_t*)menu[i].value, 2, color, COLOR_BLACK, 16);
        }
    }
}





void menu_up(void)
{
  current_index --;
  if(current_index <= 0) current_index =0;
  
}
void menu_down(void)
{
    current_index ++;
    if(current_index >= menu_length) current_index = 0;
    
}




void page1(void)
{
    LCD_Clear(COLOR_BLACK);
    LCD_ShowString(10, 5, (const uint8_t *)"Robot ID:", COLOR_WHITE, COLOR_BLACK, 16, 0);
    LCD_ShowString(10, 25, (const uint8_t *)"SQ:", COLOR_WHITE, COLOR_BLACK, 16, 0);
    LCD_ShowString(80, 25, (const uint8_t *)"Mode:", COLOR_WHITE, COLOR_BLACK, 16, 0);
    LCD_ShowString(10, 45, (const uint8_t *)"Robot Volt:", COLOR_WHITE, COLOR_BLACK, 16, 0);
    LCD_ShowString(10, 65, (const uint8_t *)"set > press select", COLOR_YELLOW, COLOR_BLACK, 12, 0);
}   

void page2(void)
{
    LCD_Clear(COLOR_BLACK);
    LCD_ShowString(10, 25, (const uint8_t *)"SET SUCCESS", COLOR_WHITE, COLOR_BLACK, 16, 0);
}

void page3(void)
{
    LCD_Clear(COLOR_BLACK);
    LCD_ShowString(10, 25, (const uint8_t *)"SET FAILED", COLOR_WHITE, COLOR_BLACK, 16, 0);
}
void page4(void)
{
    LCD_Clear(COLOR_BLACK);
    LCD_ShowString(10, 25, (const uint8_t *)"SETTING", COLOR_WHITE, COLOR_BLACK, 16, 0);
}
void user_ui_init(void)
{
    LCD_Init(); // Initialize the LCD display
    LCD_Clear(COLOR_BLACK); 
    xTaskCreate(user_ui_task, "user_ui_task", 4096, NULL, 5, NULL);
}



// LCD_ShowString(10, 30, (const uint8_t *)"ble:", 0x0000, 0xFFFF, 16, 0); // Display string "Hello" at (10,30)
// LCD_ShowString(10, 50, (const uint8_t *)"rx:", 0x0000, 0xFFFF, 16, 0); // Display string "Hello" at (10,30)
extern QueueHandle_t key_event_queue;
void user_ui_task(void *pvParameters)
{
    key_event_q_t evt;
    page1();
    while (1)
    {   
        if( ui_page ==  2)
        {
            if(is_setting_ok())
            {
                page2();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                page1();
                clear_setting_flag();
                nvs_save_rc_robot(rc_data.rc_mode, rc_data.robot_id);
                ui_page = 0;
            }else{
                page3();
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                nvs_read_rc_robot(&rc_data.rc_mode, &rc_data.robot_id);
                page1();
                ui_page = 0;
            }
            settings_sent = false;
        }



        if (key_event_queue && xQueueReceive(key_event_queue, &evt, pdMS_TO_TICKS(50))) {
            // 根据事件类型处理 UI
            switch (evt.event_type) {
                case KEY_EVENT_SHORT:
                    if(ui_page == 0) break; // 非设置界面不处理
                    ESP_LOGI("UI", "UI handle SHORT for key %d", evt.key_id);
                    if(evt.key_id == UP_MASk){
                        menu_up();
                        menu_draw();
                    }else if(evt.key_id == DOWN_MASK){
                        menu_down();
                        menu_draw();
                    }else if(evt.key_id == LEFT_MASK){
                        if(menu[current_index].type == MENU_ITEM_TYPE_UINT8){
                            (*(uint8_t*)menu[current_index].value)--;
                            if(*(uint8_t*)menu[current_index].value < menu[current_index].min){
                                *(uint8_t*)menu[current_index].value = menu[current_index].max;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            menu_draw();
                        }else if(menu[current_index].type == MENU_ITEM_TYPE_UINT16){
                            (*(uint16_t*)menu[current_index].value)--;
                            if(*(uint16_t*)menu[current_index].value < menu[current_index].min){
                                *(uint16_t*)menu[current_index].value = menu[current_index].max;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            menu_draw();
                        }else if(menu[current_index].type == MENU_ITEM_TYPE_ACTION){
                            if(menu[current_index].action) menu[current_index].action();
                        }
                    }else if (evt.key_id == RIGHT_MASK){
                        if(menu[current_index].type == MENU_ITEM_TYPE_UINT8){
                            (*(uint8_t*)menu[current_index].value)++;
                            if(*(uint8_t*)menu[current_index].value > menu[current_index].max){
                                *(uint8_t*)menu[current_index].value = menu[current_index].min;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            menu_draw();
                        }else if(menu[current_index].type == MENU_ITEM_TYPE_UINT16){
                            (*(uint16_t*)menu[current_index].value)++;
                            if(*(uint16_t*)menu[current_index].value > menu[current_index].max){
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            
                            menu_draw();
                        }else if(menu[current_index].type == MENU_ITEM_TYPE_ACTION){
                            if(menu[current_index].action) menu[current_index].action();
                            
                        }
                    }
                    break;
                case KEY_EVENT_LONG:
                    ESP_LOGI("UI", "UI handle LONG for key %d", evt.key_id);
                    if(evt.key_id == SELECT_MASK){
                        LCD_Clear(COLOR_BLACK);
                        menu_draw();
                        ui_page = 1;
                    }
                    // TODO: 处理UI逻辑
                    break;
                case KEY_EVENT_DOUBLE:
                    ESP_LOGI("UI", "UI handle DOUBLE for key %d", evt.key_id);
                    // TODO: 处理UI逻辑
                     if(ui_page == 0) break; 
                     if(evt.key_id == LEFT_MASK){
                         if(menu[current_index].type == MENU_ITEM_TYPE_UINT16){
                            (*(uint16_t*)menu[current_index].value) -= 50;
                            if(*(uint16_t*)menu[current_index].value < menu[current_index].min){
                                *(uint16_t*)menu[current_index].value = menu[current_index].max;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            menu_draw();
                        }
                    }else if (evt.key_id == RIGHT_MASK){
                      if(menu[current_index].type == MENU_ITEM_TYPE_UINT16){
                            (*(uint16_t*)menu[current_index].value)+=50;
                            if(*(uint16_t*)menu[current_index].value > menu[current_index].max){
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }else if (*(uint16_t*)menu[current_index].value < menu[current_index].min)
                            {
                                *(uint16_t*)menu[current_index].value = menu[current_index].min;
                            }
                            menu_draw();
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }
}





bool send_settings(void)
{
    return settings_sent;
}