#include <stdio.h>
#include <string.h>
#include "sysctl.h"
#include "plic.h"
#include "gpiohs.h"
#include "fpioa.h"
#include "lcd.h"
#include "dvp.h"
#include "board_config.h"

#if (BOARD_VERSION == BOARD_V1_2_LE)
#include "ov2640.h"
#elif (BOARD_VERSION == BOARD_V1_3)
#include "key.h"
#include "pwm.h"
#include "gc0328.h"
#include "tick.h"
#endif

#include "kpu.h"
#include "image_process.h"
#include "yolo_layer.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

volatile uint8_t ircut_value = 0x01;
volatile uint8_t r_ircut_value = 0x00;

#define CLASS_NUMBER 2

INCBIN(model, "tiny_yolo.kmodel");

#define ANCHOR_NUM 3
static float anchor0[ANCHOR_NUM * 2] = {48, 50, 81, 100, 205, 191};
static float anchor1[ANCHOR_NUM * 2] = {6, 8, 13, 15, 22, 34};
static obj_info_t detect_info;
static yolo_layer_t detect_rl_0, detect_rl_1;
kpu_model_context_t detect_task;
static image_t origin_image, kpu_image, display_image;

volatile uint8_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;

#if (BOARD_VERSION == BOARD_V1_3)
uint8_t g_camera_no = 0;

void camera_switch(void)
{
    g_camera_no = !g_camera_no;
    g_camera_no ? open_gc0328_0() : open_gc0328_1();

    int enable = g_camera_no ? 1 : 0;
    pwm_set_enable(1, 1, enable);
}
#endif 

static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
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

static void io_init(void)
{
    /* Init DVP IO map and function settings */
#if (BOARD_VERSION == BOARD_V1_2_LE)
    fpioa_set_function(DVP_RST_PIN, FUNC_CMOS_RST);
#endif
    fpioa_set_function(DVP_PWDN_PIN, FUNC_CMOS_PWDN);
    fpioa_set_function(DVP_XCLK_PIN, FUNC_CMOS_XCLK);
    fpioa_set_function(DVP_VSYNC_PIN, FUNC_CMOS_VSYNC);
    fpioa_set_function(DVP_HREF_PIN, FUNC_CMOS_HREF);
    fpioa_set_function(DVP_PCLK_PIN, FUNC_CMOS_PCLK);
    fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_SCCB_SCLK);
    fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_SCCB_SDA);

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

#if (BOARD_VERSION == BOARD_V1_3)
    /* KEY IO map and function settings */
    fpioa_set_function(KEY_PIN, FUNC_GPIOHS0 + KEY_IO);
    gpiohs_set_drive_mode(KEY_IO, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(KEY_IO, GPIO_PE_FALLING);
    gpiohs_irq_register(KEY_IO, 1, key_trigger, NULL);

     /* pwm IO.*/
    fpioa_set_function(LED_IR_PIN, FUNC_TIMER0_TOGGLE2 + 1 * 4);         
     /* Init PWM */
    pwm_init(1);
    pwm_set_frequency(1, 1, 3000, 0.3); 
    pwm_set_enable(1, 1, 0);
#endif
}

static void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}


#if (CLASS_NUMBER > 1)
typedef struct
{
    char *str;
    uint16_t color;
    uint16_t height;
    uint16_t width;
    uint32_t *ptr;
} class_lable_t;

class_lable_t class_lable[CLASS_NUMBER] =
{
    {"screw", GREEN},
    {"nut", RED}
};

static uint32_t lable_string_draw_ram[115 * 16 * 8 / 2];
#endif

static void lable_init(void)
{
#if (CLASS_NUMBER > 1)
    uint8_t index;

    class_lable[0].height = 16;
    class_lable[0].width = 8 * strlen(class_lable[0].str);
    class_lable[0].ptr = lable_string_draw_ram;
    lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
    for (index = 1; index < CLASS_NUMBER; index++) {
        class_lable[index].height = 16;
        class_lable[index].width = 8 * strlen(class_lable[index].str);
        class_lable[index].ptr = class_lable[index - 1].ptr + class_lable[index - 1].height * class_lable[index - 1].width / 2;
        lcd_ram_draw_string(class_lable[index].str, class_lable[index].ptr, BLACK, class_lable[index].color);
    }
#endif
}

static void drawboxes(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob)
{
    if (x1 >= 320)
        x1 = 319;
    if (x2 >= 320)
        x2 = 319;
    if (y1 >= 240)
        y1 = 239;
    if (y2 >= 240)
        y2 = 239;

#if (CLASS_NUMBER > 1)
    lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
    lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width, class_lable[class].height, class_lable[class].ptr);
#else
    lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
#endif
}

int main(void)
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    io_set_power();
    plic_init();
    io_init();
    lable_init();

    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(DIR_YX_RLDU);
    lcd_clear(BLACK);

    /* DVP init */
    printf("DVP init\n");
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
#if (BOARD_VERSION == BOARD_V1_2_LE)
    ov2640_init();
#elif (BOARD_VERSION == BOARD_V1_3)
    gc0328_init();
    open_gc0328_1();
#endif

    /* init kpu */
    if (kpu_load_kmodel(&detect_task, model_data) != 0)
    {
        printf("\nmodel init error\n");
        while (1);
    }
    origin_image.pixel = 3;
    origin_image.width = 320;
    origin_image.height = 240;
    image_init(&origin_image);
    kpu_image.pixel = 3;
    kpu_image.width = 256;
    kpu_image.height = 256;
    image_init(&kpu_image);
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);

    detect_rl_0.anchor_number = ANCHOR_NUM;
    detect_rl_0.anchor = anchor0;
    detect_rl_0.threshold = 0.5;
    detect_rl_0.nms_value = 0.2;
    detect_rl_0.image_width = 320;
    detect_rl_0.image_height = 240;

    detect_rl_1.anchor_number = ANCHOR_NUM;
    detect_rl_1.anchor = anchor1;
    detect_rl_1.threshold = 0.5;
    detect_rl_1.nms_value = 0.2;
    detect_rl_1.image_width = 320;
    detect_rl_1.image_height = 240;
    yolo_layer_init(&detect_rl_0, 8, 8, 21, 256, 256);
    yolo_layer_init(&detect_rl_1, 16, 16, 21, 256, 256);

    dvp_set_ai_addr((uint32_t)origin_image.addr, (uint32_t)(origin_image.addr + 320 * 240), (uint32_t)(origin_image.addr + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    /* enable global interrupt */
    sysctl_enable_irq();

#if (BOARD_VERSION == BOARD_V1_3)
    tick_init(TICK_NANOSECONDS);
#endif

    /* system start */
    printf("System start\n");

    while (1)
    {
#if (BOARD_VERSION == BOARD_V1_3)
        if (KEY_PRESS == key_get())
        {
            camera_switch();
        }
#endif

        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;
            
        image_resize(&origin_image, &kpu_image);
        g_ai_done_flag = 0;
        /* start to calculate */
        kpu_run_kmodel(&detect_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while(!g_ai_done_flag);

        float *output;
        size_t output_size;
        kpu_get_output(&detect_task, 0, &output, &output_size);
        detect_rl_0.input = output;
        yolo_layer_run(&detect_rl_0, NULL);

        /* display pic*/
        lcd_draw_picture(0, 0, 320, 240, display_image.addr);

        /* draw boxs */
        yolo_layer_draw_boxes(&detect_rl_0, drawboxes);
    }
}

