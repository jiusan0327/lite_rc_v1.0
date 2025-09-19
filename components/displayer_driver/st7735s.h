#ifndef _ST7735S_H_
#define _ST7735S_H_

#include <stdint.h>
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_check.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <memory.h>

#define ST7735_PIN_NUM_SCLK           (GPIO_NUM_36)
#define ST7735_PIN_NUM_MOSI           (GPIO_NUM_35)
#define ST7735_PIN_NUM_LCD_RES         (GPIO_NUM_38)
#define ST7735_PIN_NUM_LCD_DC        (GPIO_NUM_37)
#define ST7735_PIN_NUM_LCD_CS         (GPIO_NUM_41)
#define ST7735_PIN_NUM_BK_LIGHT       (GPIO_NUM_39)
#define ST7735_PIN_NUM_LEDA         (GPIO_NUM_15)

#define USE_HORIZONTAL 2  //0 or 1 for 80*160, 2 or 3 for 160*80


#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define LCD_W 80
#define LCD_H 160

#else
#define LCD_W 160
#define LCD_H 80
#endif

#define LCD_LEDA_Clr gpio_set_level(ST7735_PIN_NUM_LEDA,0)
#define LCD_LEDA_Set gpio_set_level(ST7735_PIN_NUM_LEDA,1)

#define LCD_SCLK_Clr gpio_set_level(ST7735_PIN_NUM_SCLK,0)//SCL=SCLK
#define LCD_SCLK_Set gpio_set_level(ST7735_PIN_NUM_SCLK,1)

#define LCD_MOSI_Clr gpio_set_level(ST7735_PIN_NUM_MOSI,0)//SDA=MOSI
#define LCD_MOSI_Set gpio_set_level(ST7735_PIN_NUM_MOSI,1)

#define LCD_RES_Clr  gpio_set_level(ST7735_PIN_NUM_LCD_RES,0)//RES
#define LCD_RES_Set  gpio_set_level(ST7735_PIN_NUM_LCD_RES,1)

#define LCD_DC_Clr   gpio_set_level(ST7735_PIN_NUM_LCD_DC,0)//DC
#define LCD_DC_Set   gpio_set_level(ST7735_PIN_NUM_LCD_DC,1)
 		     
#define LCD_CS_Clr   gpio_set_level(ST7735_PIN_NUM_LCD_CS,0)//CS
#define LCD_CS_Set   gpio_set_level(ST7735_PIN_NUM_LCD_CS,1)

#define LCD_BLK_Clr  gpio_set_level(ST7735_PIN_NUM_BK_LIGHT,0)//BLK
#define LCD_BLK_Set  gpio_set_level(ST7735_PIN_NUM_BK_LIGHT,1)


#define RGB565(r,g,b)  ( ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3) )
#define COLOR_WHITE   0xFFFF  // RGB565(255,255,255)  
#define COLOR_BLACK   0x0000  
#define COLOR_RED     RGB565(255,0,0)
#define COLOR_GREEN   RGB565(0,255,0)
#define COLOR_BLUE    RGB565(19,104,173)
#define COLOR_YELLOW  RGB565(255,255,0)
#define COLOR_CYAN    RGB565(0,255,255)
#define COLOR_MAGENTA RGB565(255,0,255)
#define COLOR_GRAY    RGB565(128,128,128)

void LCD_GPIO_Init(void);
void LCD_Writ_Bus(uint8_t dat);
void LCD_WR_DATA8(uint8_t dat);
void LCD_WR_DATA(uint16_t dat);
void LCD_WR_REG(uint8_t dat);
void LCD_Address_Set(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2);
void LCD_Init(void);

void LCD_Fill(uint16_t xsta,uint16_t ysta,uint16_t xend,uint16_t yend,uint16_t color);
void LCD_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void LCD_DrawLine(uint16_t x1,uint16_t y1,uint16_t x2,uint16_t y2,uint16_t color);
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2,uint16_t color);
void Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r,uint16_t color);
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);
void LCD_ShowString(uint16_t x,uint16_t y,const uint8_t *p,uint16_t fc,uint16_t bc,uint8_t sizey,uint8_t mode);
void LCD_ShowIntNum(uint16_t x,uint16_t y,uint16_t num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey);
void LCD_ShowFloatNum1(uint16_t x,uint16_t y,float num,uint8_t len,uint16_t fc,uint16_t bc,uint8_t sizey);
void LCD_Clear(uint16_t color);
#endif // ST7735S_H