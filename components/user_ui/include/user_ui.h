#ifndef __USER_UI_H__
#define __USER_UI_H__
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "st7735s.h"
typedef enum {
    MENU_ITEM_TYPE_UINT8,
    MENU_ITEM_TYPE_UINT16,
    MENU_ITEM_TYPE_ACTION
} MenuItemType;




typedef struct {
    const char *name;       // 菜单名称
    MenuItemType type;      // 数据类型
    void *value;            // 指向实际数据的指针
    uint16_t min;           // 最小值（用于数字）
    uint16_t max;           // 最大值（用于数字）
    void (*action)(void);   // 如果是动作项（如退出），执行的回调
} MenuItem;


#define UP_MASk        13
#define DOWN_MASK      12
#define LEFT_MASK      14
#define RIGHT_MASK     15
#define SELECT_MASK     4

void user_ui_init(void);
void user_ui_task(void *pvParameters);
bool send_settings(void);
#endif // __USER_UI_H__