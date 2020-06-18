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
#include <float.h>
#include <platform.h>
#include <printf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysctl.h>
#include <unistd.h>
#include "board_config.h"
#include "bsp.h"
#include "dvp.h"
#include "fpioa.h"
#include "gpio.h"
#include "gpiohs.h"
#include "image_process.h"
#include "kpu.h"
#include "lcd.h"
#include "nt35310.h"
#include "ov2640.h"
#include "plic.h"
#include "spi.h"
#include "uarths.h"
#include "utils.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

#define CLASS10 1
#define min(a, b) (((a) < (b)) ? (a) : (b))

#define PROB_THRESH (0.7f)

#define PLL0_OUTPUT_FREQ 1000000000UL
#define PLL1_OUTPUT_FREQ 400000000UL
#define PLL2_OUTPUT_FREQ 45158400UL

volatile uint8_t ircut_value = 0x01;
volatile uint8_t r_ircut_value = 0x00;

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image, crop_image;

extern const unsigned char gImage_image[] __attribute__((aligned(128)));
extern const unsigned char daisy_image[] __attribute__((aligned(128)));
extern const unsigned char dandelion_image[] __attribute__((aligned(128)));
extern const unsigned char roses_image[] __attribute__((aligned(128)));
extern const unsigned char sunflowers_image[] __attribute__((aligned(128)));
extern const unsigned char tulip_image[] __attribute__((aligned(128)));
static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));
kpu_model_context_t task1;

INCBIN(model, "class.kmodel");

static void ai_done(void *userdata)
{
    g_ai_done_flag = 1;

    float *features;
    size_t count;
    kpu_get_output(&task1, 0, (uint8_t **)&features, &count);
    count /= sizeof(float);

    size_t i;
    for(i = 0; i < count; i++)
    {
        if(i % 64 == 0)
            printf("\n");
        printf("%f, ", features[i]);
    }

    printf("\n");
}

static int dvp_irq(void *ctx)
{
    if(dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    } else
    {
        dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

static void io_init(void)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(OV_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(OV_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(OV_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(OV_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(OV_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(OV_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(OV_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(OV_SCCB_SDA_PIN, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(LCD_DC_PIN, FUNC_GPIOHS0 + LCD_DC_IO);
    fpioa_set_function(LCD_CS_PIN, FUNC_SPI0_SS3);
    fpioa_set_function(LCD_RW_PIN, FUNC_SPI0_SCLK);
    fpioa_set_function(LCD_RST_PIN, FUNC_GPIOHS0 + LCD_RST_IO);

    sysctl_set_spi0_dvp_data(1);

    // LCD Backlight
    fpioa_set_function(LCD_BLIGHT_PIN, FUNC_GPIOHS0 + LCD_BLIGHT_IO);
    gpiohs_set_drive_mode(LCD_BLIGHT_IO, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_BLIGHT_IO, GPIO_PV_LOW);
}

static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}

void rgb888_to_lcd(uint8_t *src, uint16_t *dest, size_t width, size_t height)
{
    size_t i, chn_size = width * height;
    for(size_t i = 0; i < width * height; i++)
    {
        uint8_t r = src[i];
        uint8_t g = src[chn_size + i];
        uint8_t b = src[chn_size * 2 + i];

        uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
        size_t d_i = i % 2 ? (i - 1) : (i + 1);
        dest[d_i] = rgb;
    }
}

void lcd_ram_draw_rgb888(uint8_t *src, uint16_t *dest, size_t width, size_t height, size_t x_off, size_t y_off, size_t stride)
{
    size_t x, y, chn_size = width * height;
    for(size_t y = 0; y < min(height, 240); y++)
    {
        for(size_t x = 0; x < width; x++)
        {
            size_t i = y * width + x;
            uint8_t r = src[i];
            uint8_t g = src[chn_size + i];
            uint8_t b = src[chn_size * 2 + i];

            uint16_t rgb = ((r & 0b11111000) << 8) | ((g & 0b11111100) << 3) | (b >> 3);
            i = (y + y_off) * stride + x + x_off;
            size_t d_i = i % 2 ? (i - 1) : (i + 1);
            dest[d_i] = rgb;
        }
    }
}

int argmax(float *src, size_t count)
{
    float max = FLT_MIN;
    size_t i, max_i = 0;
    for(i = 0; i < count; i++)
    {
        if(src[i] > max)
        {
            max = src[i];
            max_i = i;
        }
    }

    return max_i;
}

int main()
{
    uint16_t mx_i;
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL2, 45158400UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    uarths_init();
    plic_init();
    io_set_power();
    io_init();

    /* LCD init */
    printf("LCD init\r\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    lcd_clear(BLACK);
    
    /* DVP init */
    printf("DVP init\r\n");
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov2640_init();

    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    crop_image.pixel = 3;
    crop_image.width = 224;
    crop_image.height = 224;
    image_init(&crop_image);
    dvp_set_ai_addr((uint32_t)kpu_image.addr, (uint32_t)(kpu_image.addr + 320 * 240), (uint32_t)(kpu_image.addr + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP interrupt config */
    printf("DVP interrupt config\r\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);
    /* init model */
    if(kpu_load_kmodel(&task1, model_data) != 0)
    {
        printf("Cannot load kmodel.\r\n");
        exit(-1);
    }
    sysctl_enable_irq();

    /* system start */
    printf("System start\r\n");
    while(1)
    {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while(g_dvp_finish_flag == 0)
            ;

        image_crop(&kpu_image, &crop_image, 48, 8);

        g_ai_done_flag = 0;

        if(kpu_run_kmodel(&task1, crop_image.addr, 5, ai_done, NULL) != 0)
        {
            printf("Cannot run kmodel.\r\n");
            exit(-1);
        }
        while(!g_ai_done_flag)
            ;

        float *features;
        size_t output_size;
        kpu_get_output(&task1, 0, &features, &output_size);
        size_t cls = argmax(features, 5);

        const char *text = NULL;
        switch(cls)
        {
            case 0: {
                text = "daisy";
            }
            break;
            case 1:
                text = "dandelion";
                break;
            case 2:
                text = "roses";
                break;
            case 3:
                text = "sunflowers";
                break;
            case 4:
                text = "tulip";
                break;
        }
        printf("hello world\r\n");
        /* display pic*/
        if(features[cls] > PROB_THRESH)
            ram_draw_string(display_image.addr, 150, 20, text, RED);
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);
    }
}
