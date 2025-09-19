#include "st7735s.h"
#include "lcd_font.h"
#include <math.h> 
void LCD_GPIO_Init(void)
{
    gpio_config_t bk_gpio_config = {
        .mode = GPIO_MODE_OUTPUT,
        .pin_bit_mask = (1ULL<<ST7735_PIN_NUM_SCLK)|(1ULL<<ST7735_PIN_NUM_MOSI)|(1ULL<<ST7735_PIN_NUM_LCD_RES)|
                       (1ULL<<ST7735_PIN_NUM_LCD_DC)|(1ULL<<ST7735_PIN_NUM_LCD_CS)|(1ULL<<ST7735_PIN_NUM_BK_LIGHT),

        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&bk_gpio_config));
}
void LCD_Writ_Bus(uint8_t dat)
{
    uint8_t i;
	LCD_CS_Clr;
	for(i=0;i<8;i++)
	{			  
		LCD_SCLK_Clr;
		if(dat&0x80)
		{
		   LCD_MOSI_Set;
		}
		else
		{
		   LCD_MOSI_Clr;
		}
		LCD_SCLK_Set;
		dat<<=1;
	}	
  LCD_CS_Set;	
}
void LCD_WR_DATA8(uint8_t dat)
{
    LCD_Writ_Bus(dat);
}
void LCD_WR_DATA(uint16_t dat)
{
    LCD_Writ_Bus(dat>>8);
	LCD_Writ_Bus(dat);
}
void LCD_WR_REG(uint8_t dat)
{
    LCD_DC_Clr;
	LCD_Writ_Bus(dat);
	LCD_DC_Set;
}
void LCD_Address_Set(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) 
{
    // 根据方向模式计算偏移量
    const uint16_t x_offset = (USE_HORIZONTAL < 2) ? 24 : 0;
    const uint16_t y_offset = (USE_HORIZONTAL < 2) ? 0 : 24;

    // 设置列地址 (0x2A)
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x1 + x_offset);
    LCD_WR_DATA(x2 + x_offset);

    // 设置行地址 (0x2B)
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y1 + y_offset);
    LCD_WR_DATA(y2 + y_offset);

    // 准备写入数据 (0x2C)
    LCD_WR_REG(0x2C);
}
void LCD_Init(void) {
    // 1. 硬件初始化
    LCD_GPIO_Init();
    LCD_LEDA_Clr;
    // 复位序列
    LCD_RES_Clr;
    vTaskDelay(pdMS_TO_TICKS(20)); 
    LCD_RES_Set;
    vTaskDelay(pdMS_TO_TICKS(100)); 
    LCD_BLK_Clr; // 关闭背光
    vTaskDelay(pdMS_TO_TICKS(100));

    // 2. 使用结构体数组定义初始化命令序列
    typedef struct {
        uint8_t cmd;
        const uint8_t *data;
        uint8_t data_len;
        uint16_t delay_ms;
    } lcd_cmd_t;

    const lcd_cmd_t init_sequence[] = {
        {0x11, NULL, 0, 120},    // Sleep out, 延迟120ms
        
        // 模式配置
        {0xB1, (uint8_t[]){0x05, 0x3C, 0x3C}, 3, 0},  // Normal mode
        {0xB2, (uint8_t[]){0x05, 0x3C, 0x3C}, 3, 0},  // Idle mode
        {0xB3, (uint8_t[]){0x05, 0x3C, 0x3C, 0x05, 0x3C, 0x3C}, 6, 0}, // Partial mode
        {0xB4, (uint8_t[]){0x03}, 1, 0},       // Dot inversion
        
        // 电源配置
        {0xC0, (uint8_t[]){0xAB, 0x0B, 0x04}, 3, 0},  // AVDD GVDD
        {0xC1, (uint8_t[]){0xC5}, 1, 0},       // VGH VGL
        {0xC2, (uint8_t[]){0x0D, 0x00}, 2, 0}, // Normal Mode
        {0xC3, (uint8_t[]){0x8D, 0x6A}, 2, 0}, // Idle
        {0xC4, (uint8_t[]){0x8D, 0xEE}, 2, 0}, // Partial+Full
        {0xC5, (uint8_t[]){0x0F}, 1, 0},       // VCOM
        
        // Gamma校正
        {0xE0, (uint8_t[]){0x07, 0x0E, 0x08, 0x07, 0x10, 0x07, 0x02, 0x07, 
                          0x09, 0x0F, 0x25, 0x36, 0x00, 0x08, 0x04, 0x10}, 16, 0}, // Positive
        {0xE1, (uint8_t[]){0x0A, 0x0D, 0x08, 0x07, 0x0F, 0x07, 0x02, 0x07,
                          0x09, 0x0F, 0x25, 0x35, 0x00, 0x09, 0x04, 0x10}, 16, 0}, // Negative
        
        // MADCTL设置 显示方向
        {0xB6, (uint8_t[]){0x08, 0x82, 0x27}, 3, 0}, // Display control
        {0xFC, (uint8_t[]){0x80}, 1, 0},
        {0x3A, (uint8_t[]){0x05}, 1, 0},       // Pixel format
        {0x36, (uint8_t[]){USE_HORIZONTAL == 0 ? 0x08 : 
                          USE_HORIZONTAL == 1 ? 0xC8 :
                          USE_HORIZONTAL == 2 ? 0x78 : 0xA8}, 1, 0}, // MADCTL
        {0x21, NULL, 0, 0},           // Display inversion
        {0x29, NULL, 0, 0},           // Display on
        
        // 设置显示区域（可根据实际屏幕尺寸调整）
        // 列地址：X起始=0, X结束=131 (0x83)
        {0x2A, (uint8_t[]){0x00, 0x28, 0x00, 0x83}, 4, 0}, 

        // 行地址：Y起始=0, Y结束=161 (0xA1)
        {0x2B, (uint8_t[]){0x00, 0x00, 0x00, 0xA1}, 4, 0},
        {0x2C, NULL, 0, 0},           // Memory write
        
    };

    // 3. 执行初始化序列
    for (size_t i = 0; i < sizeof(init_sequence) / sizeof(init_sequence[0]); i++) {
        LCD_WR_REG(init_sequence[i].cmd);
        if (init_sequence[i].data_len > 0) {
            for (uint8_t j = 0; j < init_sequence[i].data_len; j++) {
                LCD_WR_DATA8(init_sequence[i].data[j]);
            }
        }
        if (init_sequence[i].delay_ms > 0) {
            vTaskDelay(pdMS_TO_TICKS(init_sequence[i].delay_ms));
        }
    }
    LCD_WR_REG(0x20);
    LCD_LEDA_Set; // 打开背光
}
void LCD_Fill(uint16_t xsta, uint16_t ysta, uint16_t xend, uint16_t yend, uint16_t color)
{
    uint32_t total_pixels = (xend - xsta) * (yend - ysta);

    LCD_Address_Set(xsta, ysta, xend - 1, yend - 1); 

    while (total_pixels--) {
        LCD_WR_DATA(color);
    }
}


void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	LCD_Address_Set(x,y,x,y); 
	LCD_WR_DATA(color);
} 



void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    while (1) {
        LCD_DrawPoint(x1, y1, color);
        if (x1 == x2 && y1 == y2) break;
        int e2 = err * 2;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx)  { err += dx; y1 += sy; }
    }
}


void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t color)
{
    // 确保坐标顺序正确
    if (x1 > x2) { uint16_t t = x1; x1 = x2; x2 = t; }
    if (y1 > y2) { uint16_t t = y1; y1 = y2; y2 = t; }

    // 上边
    LCD_Address_Set(x1, y1, x2, y1);
    for (uint16_t i = 0; i <= (x2 - x1); i++) LCD_WR_DATA(color);
    // 下边
    LCD_Address_Set(x1, y2, x2, y2);
    for (uint16_t i = 0; i <= (x2 - x1); i++) LCD_WR_DATA(color);
    // 左边
    LCD_Address_Set(x1, y1, x1, y2);
    for (uint16_t i = 0; i <= (y2 - y1); i++) LCD_WR_DATA(color);
    // 右边
    LCD_Address_Set(x2, y1, x2, y2);
    for (uint16_t i = 0; i <= (y2 - y1); i++) LCD_WR_DATA(color);
}

void Draw_Circle(uint16_t x0, uint16_t y0, uint8_t r, uint16_t color)
{
    int a = 0;
    int b = r;
    int d = 3 - (2 * r); // 决策变量

    while (a <= b)
    {
        // 八分对称画点
        LCD_DrawPoint(x0 + a, y0 + b, color);
        LCD_DrawPoint(x0 - a, y0 + b, color);
        LCD_DrawPoint(x0 + a, y0 - b, color);
        LCD_DrawPoint(x0 - a, y0 - b, color);
        LCD_DrawPoint(x0 + b, y0 + a, color);
        LCD_DrawPoint(x0 - b, y0 + a, color);
        LCD_DrawPoint(x0 + b, y0 - a, color);
        LCD_DrawPoint(x0 - b, y0 - a, color);

        if (d < 0)
        {
            d += (4 * a) + 6;
        }
        else
        {
            d += 4 * (a - b) + 10;
            b--;
        }
        a++;
    }
}



void LCD_ShowChar(uint16_t x, uint16_t y, uint8_t ch,
                  uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint8_t sizex = sizey / 2;
    uint16_t TypefaceNum = (sizex + 7) / 8 * sizey; // 每行字节数 × 高度
    const uint8_t *font_ptr;

    // 提前选好字库指针，避免循环里 if
    ch -= ' ';
    if(sizey == 12) font_ptr = ascii_1206[ch];
    else if(sizey == 16) font_ptr = ascii_1608[ch];
    else if(sizey == 24) font_ptr = ascii_2412[ch];
    else if(sizey == 32) font_ptr = ascii_3216[ch];
    else return;

    // 模式 0：一次性批量写入（最快）
    if(mode == 0)
    {
        LCD_Address_Set(x, y, x + sizex - 1, y + sizey - 1);
        uint16_t m = 0;

        for(uint16_t i = 0; i < TypefaceNum; i++)
        {
            uint8_t temp = font_ptr[i];
            for(uint8_t t = 0; t < 8; t++)
            {
                LCD_WR_DATA((temp & (1 << t)) ? fc : bc);
                if(++m == sizex) { m = 0; break; } // 到一行末尾
            }
        }
    }
    // 模式 1：逐点画（慢）
    else
    {
        uint16_t x0 = x;
        for(uint16_t i = 0; i < TypefaceNum; i++)
        {
            uint8_t temp = font_ptr[i];
            for(uint8_t t = 0; t < 8; t++)
            {
                if(temp & (1 << t)) LCD_DrawPoint(x, y, fc);
                x++;
                if(x - x0 == sizex) { x = x0; y++; break; }
            }
        }
    }
}


void LCD_ShowString(uint16_t x, uint16_t y, const uint8_t *p,
                    uint16_t fc, uint16_t bc, uint8_t sizey, uint8_t mode)
{
    uint16_t step = sizey >> 1; // 等效 sizey/2，更快
    uint8_t ch;

    while ((ch = *p++)) // 取字符并移动指针
    {
        LCD_ShowChar(x, y, ch, fc, bc, sizey, mode);
        x += step;
    }
}


void LCD_ShowIntNum(uint16_t x, uint16_t y, uint16_t num, uint8_t len,
                    uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint8_t sizex = sizey / 2;
    uint8_t enshow = 0;

    uint32_t div = 1;
    for (uint8_t i = 1; i < len; i++) {
        div *= 10;  // 计算最高位的除数
    }

    for (uint8_t t = 0; t < len; t++) {
        uint8_t digit = num / div; // 当前位数字
        num %= div;                // 去掉这一位
        div /= 10;                 // 除数缩小

        if (!enshow && t < (len - 1)) {
            if (digit == 0) {
                LCD_ShowChar(x + t * sizex, y, ' ', fc, bc, sizey, 0);
                continue;
            } else {
                enshow = 1;
            }
        }

        LCD_ShowChar(x + t * sizex, y, digit + '0', fc, bc, sizey, 0);
    }
}
float mypow(float base, float exponent) {
    return powf(base, exponent);
}
void LCD_ShowFloatNum1(uint16_t x, uint16_t y, float num,
                       uint8_t int_len, uint16_t fc, uint16_t bc, uint8_t sizey)
{
    uint16_t sizex = sizey >> 1; // 等效 sizey/2
    int32_t int_part = (int32_t)num;  // 整数部分
    uint8_t frac_part = (uint8_t)roundf((num - int_part) * 100) / 10; // 保留1位小数

    // 显示整数部分
    for (int i = int_len - 1; i >= 0; i--)
    {
        uint8_t digit = (int_part / (uint32_t)mypow(10, i)) % 10;
        LCD_ShowChar(x, y, digit + '0', fc, bc, sizey, 0);
        x += sizex;
    }

    // 显示小数点
    LCD_ShowChar(x, y, '.', fc, bc, sizey, 0);
    x += sizex;

    // 显示小数部分
    LCD_ShowChar(x, y, frac_part + '0', fc, bc, sizey, 0);
}

void LCD_Clear(uint16_t color) {
    // 设置全屏地址范围
    LCD_Address_Set(0, 0, LCD_W-1, LCD_H-1);
    
    // 批量填充颜色
    LCD_DC_Set;  // 进入数据模式
    for(uint32_t i = 0; i < (LCD_W * LCD_H); i++) {
        LCD_WR_DATA(color);  // 写入RGB565颜色
    }
}