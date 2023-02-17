#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "bsp.h"
#include "sysctl.h"
#include "plic.h"
#include "utils.h"
#include "gpiohs.h"
#include "fpioa.h"
#include "lcd.h"
#include "nt35310.h"
#include "dvp.h"
#include "ov5640.h"
#include "ov2640.h"
#include "gc0328.h"
#include "uarths.h"
#include "kpu.h"
#include "region_layer.h"
#include "image_process.h"
#include "w25qxx.h"
#include "board_config.h"

#define LOAD_KMODEL_FROM_FLASH 0

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (540 * 1024)
#define INFER_SIZE  (124 * 1024)
uint8_t model_data[KMODEL_SIZE];
uint8_t infer_data[INFER_SIZE];
#else
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

INCBIN(model, "uint8_mnist_cnn_model.kmodel");
INCBIN(infer, "infer.bin");
#endif

#define THRESH 0.7

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, crop_image, analysis_image, infer_image, display_image;
kpu_model_context_t obj_detect_task;


static void ram_draw_char(uint32_t *ptr, uint16_t x, uint16_t y, char c, uint16_t color)
{
    extern uint8_t const ascii0816[];

    uint8_t i, j, data;
    uint16_t *addr;

    for(i = 0; i < 16; i++)
    {
        addr = ((uint16_t *)ptr) + y * (320) + x;
        data = ascii0816[c * 16 + i];
        for(j = 0; j < 8; j++)
        {
            if(data & 0x80)
            {
                if((x + j) & 1)
                    *(addr - 1) = color;
                else
                    *(addr + 1) = color;
            }
            data <<= 1;
            addr++;
        }
        y++;
    }
}

static void ram_draw_string(uint32_t *ptr, uint16_t x, uint16_t y, char *str, uint16_t color)
{
    while(*str)
    {
        ram_draw_char(ptr, x, y, *str, color);
        str++;
        x += 8;
    }
}

static void softmax(float *x, float *dx, uint32_t len)
{
    float max_value = x[0];
    for(uint32_t i = 0; i < len; i++)
    {
        if(max_value < x[i])
        {
            max_value = x[i];
        }
    }
    for(uint32_t i = 0; i < len; i++)
    {
        x[i] -= max_value;
        x[i] = expf(x[i]);
    }
    float sum_value = 0.0f;
    for(uint32_t i = 0; i < len; i++)
    {
        sum_value += x[i];
    }
    for(uint32_t i = 0; i < len; i++)
    {
        dx[i] = x[i] / sum_value;
    }
}

static uint32_t argmax(float *prob, uint32_t len)
{
    float max = prob[0];
    uint32_t max_index = 0;
    for(uint32_t i = 1; i < len; i++)
    {
        if(prob[i] >= max)
        {
            max_index = i;
            max = prob[i];
        }
    }
    return max_index;
}

static void draw_edge(uint32_t *gram, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = 104;
    y1 = 64;
    x2 = 216;
    y2 = 176;

    if(x1 <= 0)
        x1 = 1;
    if(x2 >= 319)
        x2 = 318;
    if(y1 <= 0)
        y1 = 1;
    if(y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    for(uint32_t i = 0; i < 56; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        addr1++;
        addr3++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    for(uint32_t i = 0; i < 112; i++)
    {
        *addr1 = data;
        *addr2 = data;
        addr1 += 160;
        addr2 += 160;
    }
}

static void ai_done(void *ctx)
{
    g_ai_done_flag = 1;
}

static int dvp_irq(void *ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }
    return 0;
}

static void io_mux_init(void)
{
#if (BOARD_VERSION == BOARD_V1_2_LE)
	/* Init DVP IO map and function settings */
	fpioa_set_function(DVP_RST_PIN, FUNC_CMOS_RST);
	fpioa_set_function(DVP_PWDN_PIN, FUNC_CMOS_PWDN);
	fpioa_set_function(DVP_XCLK_PIN, FUNC_CMOS_XCLK);
	fpioa_set_function(DVP_VSYNC_PIN, FUNC_CMOS_VSYNC);
	fpioa_set_function(DVP_HREF_PIN, FUNC_CMOS_HREF);
	fpioa_set_function(DVP_PCLK_PIN, FUNC_CMOS_PCLK);
	fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
	fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_SCCB_SDA);
	sysctl_set_spi0_dvp_data(1);

	/* Init LCD IO map and function settings */
	fpioa_set_function(LCD_DC_PIN, FUNC_GPIOHS0 + LCD_DC_IO);
	fpioa_set_function(LCD_CS_PIN, FUNC_SPI0_SS3);
	fpioa_set_function(LCD_RW_PIN, FUNC_SPI0_SCLK);
    fpioa_set_function(LCD_BLIGHT_PIN, FUNC_GPIOHS0 + LCD_BLIGHT_IO);
    gpiohs_set_drive_mode(LCD_BLIGHT_IO, GPIO_DM_OUTPUT);
    gpiohs_set_pin(LCD_BLIGHT_IO, GPIO_PV_LOW);

	fpioa_set_function(LCD_RST_PIN, FUNC_GPIOHS0 + LCD_RST_IO);
	gpiohs_set_drive_mode(LCD_RST_IO, GPIO_DM_OUTPUT);
	gpiohs_set_pin(LCD_RST_IO, GPIO_PV_HIGH);
	
#elif (BOARD_VERSION == BOARD_V1_3)

	/* Init DVP IO map and function settings */
	fpioa_set_function(DVP_PWDN_PIN, FUNC_CMOS_PWDN);
	fpioa_set_function(DVP_XCLK_PIN, FUNC_CMOS_XCLK);
	fpioa_set_function(DVP_VSYNC_PIN, FUNC_CMOS_VSYNC);
	fpioa_set_function(DVP_HREF_PIN, FUNC_CMOS_HREF);
	fpioa_set_function(DVP_PCLK_PIN, FUNC_CMOS_PCLK);
	fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
	fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_SCCB_SDA);
	sysctl_set_spi0_dvp_data(1);	

	/* Init LCD IO map and function settings */
	fpioa_set_function(LCD_DC_PIN, FUNC_GPIOHS0 + LCD_DC_IO);
	fpioa_set_function(LCD_CS_PIN, FUNC_SPI0_SS3);
	fpioa_set_function(LCD_RW_PIN, FUNC_SPI0_SCLK);
	fpioa_set_function(LCD_BLIGHT_PIN, FUNC_GPIOHS0 + LCD_BLIGHT_IO);
	gpiohs_set_drive_mode(LCD_BLIGHT_IO, GPIO_DM_OUTPUT);	
	gpiohs_set_pin(LCD_BLIGHT_IO, GPIO_PV_LOW);
	
	fpioa_set_function(LCD_RST_PIN, FUNC_GPIOHS0 + LCD_RST_IO);
	gpiohs_set_drive_mode(LCD_RST_IO, GPIO_DM_OUTPUT);	
	gpiohs_set_pin(LCD_RST_IO, GPIO_PV_HIGH);

#elif (BOARD_VERSION == BOARD_KD233)
    /* Init DVP IO map and function settings */
    fpioa_set_function(DVP_RST_PIN, FUNC_CMOS_RST);
    fpioa_set_function(DVP_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(DVP_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(DVP_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(DVP_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(DVP_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_SCCB_SDA);
    sysctl_set_spi0_dvp_data(1);

    /* Init SPI IO map and function settings */
    fpioa_set_function(LCD_DC_PIN, FUNC_GPIOHS0 + LCD_DC_IO);
    fpioa_set_function(LCD_CS_PIN, FUNC_SPI0_SS3);
    fpioa_set_function(LCD_RW_PIN, FUNC_SPI0_SCLK);

#endif
}

static void io_set_power(void)
{
#if (BOARD_VERSION == BOARD_V1_2_LE) || (BOARD_VERSION == BOARD_V1_3)
	/* Set dvp and spi pin to 1.8V */
	sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V33);
	sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V33);
	sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V33);
	sysctl_set_power_mode(SYSCTL_POWER_BANK3, SYSCTL_POWER_V33);
	sysctl_set_power_mode(SYSCTL_POWER_BANK4, SYSCTL_POWER_V33);
	sysctl_set_power_mode(SYSCTL_POWER_BANK5, SYSCTL_POWER_V33);
	
	/* Set dvp and spi pin to 1.8V */
	sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
	sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);	

#elif 	(BOARD_VERSION == BOARD_KD233)
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
#endif
}

int main(void)
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    uarths_init();
    io_set_power();
    io_mux_init();
    plic_init();
    /* flash init */
    printf("flash init\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();
	
#if LOAD_KMODEL_FROM_FLASH
    w25qxx_read_data(0xA00000, model_data, KMODEL_SIZE, W25QXX_QUAD_FAST);
    w25qxx_read_data(0xB00000, infer_data, INFER_SIZE, W25QXX_QUAD_FAST);
#endif
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(LCD_XYZ);

    lcd_clear(BLACK);
    lcd_draw_string(136, 70, "DEMO", WHITE);
    lcd_draw_string(104, 150, "MNIST", WHITE);

	display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_display_addr((uint32_t)display_image.addr);
	/*dvp_set_output_enable(1, 1);*/

    /* init obj detect model */
    if(kpu_load_kmodel(&obj_detect_task, model_data) != 0)
    {
        printf("\nmodel init error\n");
        while(1)
            ;
    }
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    
    printf("System start\n");
	while(1) {
    	for(uint16_t i = 0; i < 10; i++)
	    {
	        memset(display_image.addr, 0, 320 * 240 * 2);
			static char display_string[64];
	        float mnist[10] = {0};

	        g_ai_done_flag = 0;
	        kpu_run_kmodel(&obj_detect_task, &infer_data[i * 112 * 112], DMAC_CHANNEL5, ai_done, NULL);
	        while(!g_ai_done_flag)
	            ;
	        float *output;
	        size_t output_size;
	        kpu_get_output(&obj_detect_task, 0, (uint8_t **)&output, &output_size);
	        softmax(output, mnist, 10);
	        uint32_t index = argmax(mnist, 10);
	        draw_edge(display_image.addr, RED);
	        if(mnist[index] >= THRESH)
	        {
	            sprintf(display_string, "This number is %d.", index);
	        } else
	        {
	            sprintf(display_string, "Please show me a number in the box.");
	        }
	        uint16_t *p = display_image.addr;
	        for(uint32_t hh = 64; hh < 176; hh+=2)
	        {
	            for(uint32_t ww = 104; ww < 216; ww+=2)
	            {
	                uint32_t disp_index = hh * display_image.width + ww;
	                uint32_t data_index = (hh - 64) * 112 + ww - 104 + 1;
	                uint16_t data = 0;
	                uint8_t *p_data = infer_data + i * 112 * 112;
	                data = ((p_data[data_index] >> 3) << 11) + ((p_data[data_index] >> 2) << 5) + ((p_data[data_index] >> 3) << 0);
	                p[disp_index] = data;
	                disp_index += 1;
	                data_index -= 1;
	                data = ((p_data[data_index] >> 3) << 11) + ((p_data[data_index] >> 2) << 5) + ((p_data[data_index] >> 3) << 0);
	                p[disp_index] = data;
	            }
	        }
	        ram_draw_string(display_image.addr, 0, 0, display_string, RED);
	        /* display pic*/
	        lcd_draw_picture(0, 0, 320, 240, display_image.addr);
	        g_dvp_finish_flag = 0;
	        msleep(2000);
	    }
	}
}
