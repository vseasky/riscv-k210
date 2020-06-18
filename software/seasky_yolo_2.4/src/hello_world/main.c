#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "board_config.h"
#include "bsp.h"
#include "dvp.h"
#include "fpioa.h"
#include "gpiohs.h"
#include "iomem.h"
#include "kpu.h"
#include "lcd.h"
#include "nt35310.h"
#include "plic.h"
#include "region_layer.h"
#include "sysctl.h"
#include "uart.h"
#include "uarths.h"
#include "utils.h"
#include "w25qxx.h"
#if OV5640
#include "ov5640.h"
#endif
#if OV2640
#include "ov2640.h"
#endif

#include "image_process.h"
#include "uarths.h"
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

#define PLL0_OUTPUT_FREQ 800000000UL
#define PLL1_OUTPUT_FREQ 400000000UL

#define UART_NUM UART_DEVICE_3
#define CLASS_NUMBER 20

extern const unsigned char gImage_image[] __attribute__((aligned(128)));
static uint16_t lcd_gram[320 * 240] __attribute__((aligned(32)));

/*模型存放方式*/
#define LOAD_KMODEL_FROM_FLASH 0
#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (1351976)
#define LOAD_FLASH_ADDR 0xA00000 //存放地址
uint8_t *model_data;
#else
INCBIN(model, "yolo.kmodel");
#endif

kpu_model_context_t task;
static obj_info_t detect_info;
static region_layer_t detect_rl;

volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;

static int ai_done(void *ctx)
{
    g_ai_done_flag = 1;
    return 0;
}

//DVP中断
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
#define ANCHOR_NUM 5

float g_anchor[ANCHOR_NUM * 2] = {1.08, 1.19, 3.42, 4.41, 6.63, 11.38, 9.42, 5.11, 16.62, 10.52};

//初始化设备，使用宏定义区分board，详情修改 board_config.h
static void io_init(void)
{
#if BOARD_LICHEEDAN

    /*初始化DVP输入输出映射和功能设置*/
    fpioa_set_function(42, FUNC_CMOS_RST);
    fpioa_set_function(44, FUNC_CMOS_PWDN);
    fpioa_set_function(46, FUNC_CMOS_XCLK);
    fpioa_set_function(43, FUNC_CMOS_VSYNC);
    fpioa_set_function(45, FUNC_CMOS_HREF);
    fpioa_set_function(47, FUNC_CMOS_PCLK);
    fpioa_set_function(41, FUNC_SCCB_SCLK);
    fpioa_set_function(40, FUNC_SCCB_SDA);

    /*初始化串行接口输入输出映射和功能设置*/
    fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(36, FUNC_SPI0_SS3);
    fpioa_set_function(39, FUNC_SPI0_SCLK);
    fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM);

    sysctl_set_spi0_dvp_data(1);
#else
    /*初始化DVP输入输出映射和功能设置*/
    fpioa_set_function(11, FUNC_CMOS_RST);
    fpioa_set_function(13, FUNC_CMOS_PWDN);
    fpioa_set_function(14, FUNC_CMOS_XCLK);
    fpioa_set_function(12, FUNC_CMOS_VSYNC);
    fpioa_set_function(17, FUNC_CMOS_HREF);
    fpioa_set_function(15, FUNC_CMOS_PCLK);
    fpioa_set_function(10, FUNC_SCCB_SCLK);
    fpioa_set_function(9, FUNC_SCCB_SDA);

    /*初始化串行接口输入输出映射和功能设置*/
    fpioa_set_function(8, FUNC_GPIOHS0 + 2);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);

    sysctl_set_spi0_dvp_data(1);
    fpioa_set_function(26, FUNC_GPIOHS0 + 8);
    gpiohs_set_drive_mode(8, GPIO_DM_INPUT);
#endif
}
//初始化DVP PWOER
static void io_set_power(void)
{
#if BOARD_LICHEEDAN
    /*将dvp和spi引脚设为1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
#else
    /*将dvp和spi引脚设为1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
#endif
}

static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if(x1 <= 0)
        x1 = 1;
    if(x2 >= 319)
        x2 = 318;
    if(y1 <= 0)
        y1 = 1;
    if(y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for(uint32_t i = 0; i < 4; i++)
    {
        *addr1 = data;
        *(addr1 + 160) = data;
        *addr2 = data;
        *(addr2 + 160) = data;
        *addr3 = data;
        *(addr3 + 160) = data;
        *addr4 = data;
        *(addr4 + 160) = data;
        addr1++;
        addr2++;
        addr3++;
        addr4++;
    }
    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 2) / 2;
    addr3 = gram + (320 * (y2 - 8) + x1) / 2;
    addr4 = gram + (320 * (y2 - 8) + x2 - 2) / 2;
    for(uint32_t i = 0; i < 8; i++)
    {
        *addr1 = data;
        *addr2 = data;
        *addr3 = data;
        *addr4 = data;
        addr1 += 160;
        addr2 += 160;
        addr3 += 160;
        addr4 += 160;
    }
}

#if(CLASS_NUMBER > 1)
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

//初始化lable
static void lable_init(void)
{
#if(CLASS_NUMBER > 1)
    uint8_t index;

    class_lable[0].height = 16;
    class_lable[0].width = 8 * strlen(class_lable[0].str);
    class_lable[0].ptr = lable_string_draw_ram;
    lcd_ram_draw_string(class_lable[0].str, class_lable[0].ptr, BLACK, class_lable[0].color);
    for(index = 1; index < CLASS_NUMBER; index++)
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
    if(x1 >= 320)
        x1 = 319;
    if(x2 >= 320)
        x2 = 319;
    if(y1 >= 240)
        y1 = 239;
    if(y2 >= 240)
        y2 = 239;
#if(CLASS_NUMBER > 1)
    lcd_draw_rectangle(x1, y1, x2, y2, 2, class_lable[class].color);
    lcd_draw_picture(x1 + 1, y1 + 1, class_lable[class].width, class_lable[class].height, class_lable[class].ptr);
#else
    lcd_draw_rectangle(x1, y1, x2, y2, 2, RED);
#endif
}

static void send_data(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2, uint32_t class, float prob)
{
    if(x1 >= 320)
        x1 = 319;
    if(x2 >= 320)
        x2 = 319;
    if(y1 >= 240)
        y1 = 239;
    if(y2 >= 240)
        y2 = 239;

    char data[4];
    int n;
    n = sprintf(data, "%d\n", class);
    uart_send_data(UART_NUM, &data, 4);
}

int main(void)
{
    /*设置中央处理器和dvp时钟*/
    sysctl_pll_set_freq(SYSCTL_PLL0, PLL0_OUTPUT_FREQ);
    sysctl_pll_set_freq(SYSCTL_PLL1, PLL1_OUTPUT_FREQ);
    uarths_init();

    io_init();      //初始化DVP输入输出映射和功能设置
    io_set_power(); //初始化DVP POWER
    plic_init();    //初始化为外部中断
    /*flash初始化*/
    printf("flash init\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

#if LOAD_KMODEL_FROM_FLASH //如果kmodel存放于flash
    model_data = (uint8_t *)iomem_malloc(KMODEL_SIZE);
    w25qxx_read_data(LOAD_FLASH_ADDR, model_data, KMODEL_SIZE, W25QXX_QUAD_FAST);
#endif

    lable_init();

    /* LCD 初始化 */
    printf("LCD init\n");
    lcd_init();

    /*设置LCD显示方向*/
    lcd_set_direction(DIR_YX_RLDU);

    lcd_clear(BLACK);
    lcd_draw_string(110, 70, "SEASKY DEMO", WHITE);
    lcd_draw_string(110, 150, "hello yolo", WHITE);
    sleep(1);
    /* DVP 初始化 */
    printf("DVP init\n");

#if OV5640
    dvp_init(16);
    dvp_set_xclk_rate(12000000);
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
    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_ai_addr((uint32_t)kpu_image.addr, (uint32_t)(kpu_image.addr + 320 * 240), (uint32_t)(kpu_image.addr + 320 * 240 * 2));
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    /* DVP中断配置*/
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);             //设置中断号
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL); //添加中断服务函数
    plic_irq_enable(IRQN_DVP_INTERRUPT);                  //使能中断

    /*启用全局中断*/
    sysctl_enable_irq();

    /*系统启动*/
    printf("system start\n");

    /* 初始化 kpu */
    if(kpu_load_kmodel(&task, model_data) != 0)
    {
        printf("\nmodel init error\n");
        while(1)
            ;
    }
    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = g_anchor;
    detect_rl.threshold = 0.5;
    detect_rl.nms_value = 0.2;
    region_layer_init(&detect_rl, 10, 7, 125, kpu_image.width, kpu_image.height);

    /*启用全局中断 */
    sysctl_enable_irq();
    /*系统启动*/
    printf("System start\n");
    uint64_t time_last = sysctl_get_time_us();
    uint64_t time_now = sysctl_get_time_us();
    int time_count = 0;

    uart_init(UART_NUM);
    uart_configure(UART_NUM, 115200, 8, UART_STOP_1, UART_PARITY_NONE);

    char *hello = {"hello world!\n"};
    uart_send_data(UART_NUM, hello, strlen(hello));

    while(1)
    {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while(g_dvp_finish_flag == 0) //等待中断发生
            ;
        /* start to calculate */
        g_ai_done_flag = 0;
        kpu_run_kmodel(&task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while(!g_ai_done_flag) //等待
            ;
        float *output;
        size_t output_size;
        kpu_get_output(&task, 0, (uint8_t **)&output, &output_size);
        detect_rl.input = output;
        output_size /= sizeof(float);
        /* start region layer */
        region_layer_run(&detect_rl, &detect_info);
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);

        /* draw boxs */
        region_layer_draw_boxes(&detect_rl, drawboxes);
        region_layer_write_to_uart(&detect_rl, send_data);
    }
}
