#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "lcd.h"
#include "board_config.h"
#include "w25qxx.h"
#if OV5640
#include "ov5640.h"
#endif
#if OV2640
#include "ov2640.h"
#endif
#include "plic.h"
#include "sysctl.h"
#include "uarths.h"
#include "nt35310.h"
#include "utils.h"
#include "kpu.h"
#include "region_layer.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
#include "iomem.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

#define CLASS_NUMBER 20

#define LOAD_KMODEL_FROM_FLASH 0
#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (1351592)
uint8_t *model_data;
#else
INCBIN(model, "yolo.kmodel");
#endif

kpu_model_context_t task;
static region_layer_t detect_rl;

volatile uint8_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag = 0;
volatile uint8_t g_ram_mux = 0;

static void ai_done(void *ctx)
{
    g_ai_done_flag = 1;
}

uint32_t *g_lcd_gram0;
uint32_t *g_lcd_gram1;
uint8_t *g_ai_buf;

#define ANCHOR_NUM 5

float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};

static int on_irq_dvp(void *ctx)
{
    if (dvp_get_interrupt(DVP_STS_FRAME_FINISH))
    {
        /* switch gram */
        dvp_set_display_addr(g_ram_mux ? (uint32_t)g_lcd_gram0 : (uint32_t)g_lcd_gram1);

        dvp_clear_interrupt(DVP_STS_FRAME_FINISH);
        g_dvp_finish_flag = 1;
    }
    else
    {
        if (g_dvp_finish_flag == 0)
            dvp_start_convert();
        dvp_clear_interrupt(DVP_STS_FRAME_START);
    }

    return 0;
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
        {"aeroplane", GREEN},
        {"bicycle", GREEN},
        {"bird", GREEN},
        {"boat", GREEN},
        {"bottle", 0xF81F},
        {"bus", GREEN},
        {"car", GREEN},
        {"cat", GREEN},
        {"chair", 0xFD20},
        {"cow", GREEN},
        {"diningtable", GREEN},
        {"dog", GREEN},
        {"horse", GREEN},
        {"motorbike", GREEN},
        {"person", 0xF800},
        {"pottedplant", GREEN},
        {"sheep", GREEN},
        {"sofa", GREEN},
        {"train", GREEN},
        {"tvmonitor", 0xF9B6}};
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
    for (index = 1; index < CLASS_NUMBER; index++)
    {
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
    /* 设置时钟 */
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    uarths_init();

    io_mux_init();
    io_set_power();
    plic_init();

    /* 初始化flash */
    printf("flash init\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

    g_lcd_gram0 = (uint32_t *)iomem_malloc(320 * 240 * 2);
    g_lcd_gram1 = (uint32_t *)iomem_malloc(320 * 240 * 2);
    g_ai_buf = (uint8_t *)iomem_malloc(320 * 240 * 3);

#if LOAD_KMODEL_FROM_FLASH
    model_data = (uint8_t *)malloc(KMODEL_SIZE + 255);
    uint8_t *model_data_align = (uint8_t *)(((uintptr_t)model_data + 255) & (~255));
    w25qxx_read_data(0xC00000, model_data_align, KMODEL_SIZE, W25QXX_QUAD_FAST);
#else
    uint8_t *model_data_align = model_data;
#endif

    lable_init();
    /* LCD 初始化 */
    printf("LCD init\n");
    lcd_init();
    /* DVP 初始化 */
    printf("DVP init\n");
#if OV5640
    dvp_init(16);
    dvp_set_xclk_rate(50000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov5640_init();
#else
    dvp_init(8);
    dvp_set_xclk_rate(24000000);
    dvp_enable_burst();
    dvp_set_output_enable(0, 1);
    dvp_set_output_enable(1, 1);
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    ov2640_init();
#endif

    dvp_set_ai_addr((uint32_t)g_ai_buf, (uint32_t)(g_ai_buf + 320 * 240), (uint32_t)(g_ai_buf + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)g_lcd_gram0);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();

    /* DVP interrupt config */
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, on_irq_dvp, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

    sysctl_enable_irq();

    /* system start */
    printf("system start\n");
    g_ram_mux = 0;
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);

    /* init kpu */
    if (kpu_load_kmodel(&task, model_data_align) != 0)
    {
        printf("\nmodel init error\n");
        lcd_draw_string(94, 160, "model init error!", RED);
        while (1)
            ;
    }
    lcd_draw_string(90, 160, "model init success.", GREEN);

    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.7;
    detect_rl.nms_value = 0.3;
    region_layer_init(&detect_rl, 10, 7, 125, 320, 240);

    uint64_t time_last = sysctl_get_time_us();
    uint64_t time_now = sysctl_get_time_us();
    int time_count = 0;
    while (1)
    {
        /* dvp finish*/
        while (g_dvp_finish_flag == 0)
            ;

        /* start to calculate */
        kpu_run_kmodel(&task, g_ai_buf, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag)
            ;
        g_ai_done_flag = 0;

        float *output;
        size_t output_size;
        kpu_get_output(&task, 0, (uint8_t **)&output, &output_size);
        detect_rl.input = output;

        /* start region layer */
        region_layer_run(&detect_rl, NULL);

        /* display pic*/
        g_ram_mux ^= 0x01;
        lcd_draw_picture(0, 0, 320, 240, g_ram_mux ? g_lcd_gram0 : g_lcd_gram1);
        g_dvp_finish_flag = 0;

        /* draw boxs */
        region_layer_draw_boxes(&detect_rl, drawboxes);
        time_count++;
        if (time_count % 100 == 0)
        {
            time_now = sysctl_get_time_us();
            printf("SPF:%fms\n", (time_now - time_last) / 1000.0 / 100);
            time_last = time_now;
        }
    }
    return 0;
}
