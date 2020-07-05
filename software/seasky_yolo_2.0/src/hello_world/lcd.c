/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <math.h>
#include <string.h>
#include <unistd.h>
#include "board_config.h"
#include "font.h"
#include "lcd.h"
#include "nt35310.h"

static lcd_ctl_t lcd_ctl;

void lcd_polling_enable(void)
{
    lcd_ctl.mode = 0;
}

void lcd_interrupt_enable(void)
{
    lcd_ctl.mode = 1;
}

void lcd_init(void)
{
    uint8_t data = 0;

    tft_hard_init();
    /*soft reset*/
    tft_write_command(SOFTWARE_RESET);
    usleep(100000);
    /*exit sleep*/
    tft_write_command(SLEEP_OFF);
    usleep(100000);
    /*pixel format*/
    tft_write_command(PIXEL_FORMAT_SET);
    data = 0x55;
    tft_write_byte(&data, 1);
    lcd_set_direction(DIR_XY_LRUD);

    /*display on*/
    tft_write_command(DISPALY_ON);
    lcd_polling_enable();
}

void lcd_set_direction(lcd_dir_t dir)
{
#if BOARD_LICHEEDAN
#else
    dir |= 0x08;
#endif
    lcd_ctl.dir = dir;
    if(dir & DIR_XY_MASK)
    {
        lcd_ctl.width = LCD_Y_MAX - 1;
        lcd_ctl.height = LCD_X_MAX - 1;
    } else
    {
        lcd_ctl.width = LCD_X_MAX - 1;
        lcd_ctl.height = LCD_Y_MAX - 1;
    }

    tft_write_command(MEMORY_ACCESS_CTL);
    tft_write_byte((uint8_t *)&dir, 1);
}

void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    uint8_t data[4] = {0};

    data[0] = (uint8_t)(x1 >> 8);
    data[1] = (uint8_t)(x1);
    data[2] = (uint8_t)(x2 >> 8);
    data[3] = (uint8_t)(x2);
    tft_write_command(HORIZONTAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    data[0] = (uint8_t)(y1 >> 8);
    data[1] = (uint8_t)(y1);
    data[2] = (uint8_t)(y2 >> 8);
    data[3] = (uint8_t)(y2);
    tft_write_command(VERTICAL_ADDRESS_SET);
    tft_write_byte(data, 4);

    tft_write_command(MEMORY_WRITE);
}

void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color)
{
    lcd_set_area(x, y, x, y);
    tft_write_half(&color, 1);
}

void lcd_draw_char(uint16_t x, uint16_t y, char c, uint16_t color)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t data = 0;

    for(i = 0; i < 16; i++)
    {
        data = ascii0816[c * 16 + i];
        for(j = 0; j < 8; j++)
        {
            if(data & 0x80)
                lcd_draw_point(x + j, y, color);
            data <<= 1;
        }
        y++;
    }
}

void lcd_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color)
{
    while(*str)
    {
        lcd_draw_char(x, y, *str, color);
        str++;
        x += 8;
    }
}

void lcd_ram_draw_string(char *str, uint32_t *ptr, uint16_t font_color, uint16_t bg_color)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t data = 0;
    uint8_t *pdata = NULL;
    uint16_t width = 0;
    uint32_t *pixel = NULL;

    width = 4 * strlen(str);
    while(*str)
    {
        pdata = (uint8_t *)&ascii0816[(*str) * 16];
        for(i = 0; i < 16; i++)
        {
            data = *pdata++;
            pixel = ptr + i * width;
            for(j = 0; j < 4; j++)
            {
                switch(data >> 6)
                {
                    case 0:
                        *pixel = ((uint32_t)bg_color << 16) | bg_color;
                        break;
                    case 1:
                        *pixel = ((uint32_t)bg_color << 16) | font_color;
                        break;
                    case 2:
                        *pixel = ((uint32_t)font_color << 16) | bg_color;
                        break;
                    case 3:
                        *pixel = ((uint32_t)font_color << 16) | font_color;
                        break;
                    default:
                        *pixel = 0;
                        break;
                }
                data <<= 2;
                pixel++;
            }
        }
        str++;
        ptr += 4;
    }
}

void lcd_clear(uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;

    lcd_set_area(0, 0, lcd_ctl.width, lcd_ctl.height);
    tft_fill_data(&data, LCD_X_MAX * LCD_Y_MAX / 2);
}

void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color)
{
    uint32_t data_buf[640] = {0};
    uint32_t *p = data_buf;
    uint32_t data = color;
    uint32_t index = 0;

    data = (data << 16) | data;
    for(index = 0; index < 160 * width; index++)
    {
        *p++ = data;
    }
    lcd_set_area(x1, y1, x2, y1 + width - 1);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y2 - width + 1, x2, y2);
    tft_write_word(data_buf, ((x2 - x1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x1, y1, x1 + width - 1, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
    lcd_set_area(x2 - width + 1, y1, x2, y2);
    tft_write_word(data_buf, ((y2 - y1 + 1) * width + 1) / 2, 0);
}

uint16_t tom_abs(uint16_t a, uint16_t b)
{
    if(a - b < 0)
        return (b - a);
    else
        return (a - b);
}
void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr)
{
    lcd_set_area(x1, y1, x1 + width - 1, y1 + height - 1);
    tft_write_word(ptr, width * height / 2, lcd_ctl.mode ? 2 : 0);
}

void num_String(int x, char *s) // int x:数字数组；char *s：数字字符串数组
{
    int v, xx, i, j, n = 0;
    char c;
    xx = x; // 求数值x的绝对值
    while(1)
    {
        v = xx % 10;    // 对获取绝对值x的数值xx 求余数
        s[n] = v + '0'; // 数值转换为字符-存入到数组中
        n++;            // 位移以此存储多个余数存储

        xx = (xx - v) / 10; // 求整
        if(xx == 0)
            break;
    }

    if(x < 0) // 如果int型中数值为负数
    {
        s[n] = '-'; // 将负号标识赋给char型字符串数组s[]
        n++;        // 位移赋值
    }

    for(i = 0; i < n / 2; i++)
    {
        c = s[i];            //赋值给char c
        s[i] = s[n - i - 1]; // 一共n个字符
        s[n - i - 1] = c;
    }
    s[n] = '\0'; // 字符串结束标识符
}
