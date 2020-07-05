#ifndef _LCD_H_
#define _LCD_H_

#include <stdint.h>

/* clang-format off */
#define LCD_X_MAX   (240)
#define LCD_Y_MAX   (320)

#define BLACK       0x0000^0XFFFF
#define NAVY        0x000F^0XFFFF
#define DARKGREEN   0x03E0^0XFFFF
#define DARKCYAN    0x03EF^0XFFFF
#define MAROON      0x7800^0XFFFF
#define PURPLE      0x780F^0XFFFF
#define OLIVE       0x7BE0^0XFFFF
#define LIGHTGREY   0xC618^0XFFFF
#define DARKGREY    0x7BEF^0XFFFF
#define BLUE        0x001F^0XFFFF
#define GREEN       0x07E0^0XFFFF
#define CYAN        0x07FF^0XFFFF
#define RED         0xF800^0XFFFF
#define MAGENTA     0xF81F^0XFFFF
#define YELLOW      0xFFE0^0XFFFF
#define WHITE       0xFFFF^0XFFFF
#define ORANGE      0xFD20^0XFFFF
#define GREENYELLOW 0xAFE5^0XFFFF
#define PINK        0xF81F^0XFFFF
#define USER_COLOR  0xAA55^0XFFFF
/* clang-format on */

typedef enum _lcd_dir
{
    DIR_XY_RLUD = 0x00,
    DIR_YX_RLUD = 0x20,
    DIR_XY_LRUD = 0x40,
    DIR_YX_LRUD = 0x60,
    DIR_XY_RLDU = 0x80,
    DIR_YX_RLDU = 0xA0,
    DIR_XY_LRDU = 0xC0,
    DIR_YX_LRDU = 0xE0,
    DIR_XY_MASK = 0x20,
    DIR_MASK = 0xE0,
} lcd_dir_t;

typedef struct _lcd_ctl
{
    uint8_t mode;
    uint8_t dir;
    uint16_t width;
    uint16_t height;
} lcd_ctl_t;

void lcd_polling_enable(void);
void lcd_interrupt_enable(void);
void lcd_init(void);
void lcd_clear(uint16_t color);
void lcd_set_direction(lcd_dir_t dir);
void lcd_set_area(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void lcd_draw_point(uint16_t x, uint16_t y, uint16_t color);
void lcd_draw_string(uint16_t x, uint16_t y, char *str, uint16_t color);
void lcd_draw_picture(uint16_t x1, uint16_t y1, uint16_t width, uint16_t height, uint32_t *ptr);
void lcd_draw_rectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t width, uint16_t color);
void lcd_ram_draw_string(char *str, uint32_t *ptr, uint16_t font_color, uint16_t bg_color);
void num_String(int x, char *s);
uint16_t tom_abs(uint16_t a, uint16_t b);

#endif
