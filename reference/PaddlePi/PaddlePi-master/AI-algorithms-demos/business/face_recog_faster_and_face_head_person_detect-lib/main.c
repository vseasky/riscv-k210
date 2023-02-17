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
#include "pwm.h"
#include "ai.h"
#include "key.h"
#include "timer.h"


/*******fixed parameters*******/
#define FEATURE_LEN 192
/*******flexible parameters*******/
#define FACE_REGISTER_MODE FACE_APPEND
#define MAX_REGISTERED_FACE_NUMBER 100
#define FACE_RECGONITION_SCORE (80.5)

volatile uint8_t g_dvp_finish_flag;
static image_t display_image;
static image_t kpu_image, resized_image, st_image;
static key_point_t key_point;
static obj_info_t obj_detect_info;
static float registered_face_features[MAX_REGISTERED_FACE_NUMBER * FEATURE_LEN] = {0.0};
static uint32_t registered_face_number = 0;
char display_status[32];
uint8_t g_camera_no = 0;

license_t license[3] =
    {
        {.lic1_h = 0x7bd96ada41eca6d9,
         .lic1_l = 0xca89db8626cf10fe,
         .lic2_h = 0xf46183aa8e179d8a,
         .lic2_l = 0x3cbf5b109b6a1275},
        {.lic1_h = 0x6ab025e2503e792e,
         .lic1_l = 0x50c0f481abb98af4,
         .lic2_h = 0xf7979f3a4ccc62f2,
         .lic2_l = 0xc8bdf4d0763acd22},
        {.lic1_h = 0x4b2e47f06872b8fd,
         .lic1_l = 0xe7fceec9f3bd8baa,
         .lic2_h = 0xc81bb5c8cad98558,
         .lic2_l = 0x3102a276215f0c5f}};

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

static void saved_face_feature_init()
{
    memset(registered_face_features, 0.0, sizeof(float) * MAX_REGISTERED_FACE_NUMBER * FEATURE_LEN);
}

static int isr_tick(void *ctx)
{
    key_scan();
    return 0;
}

static void io_mux_init(void)
{
#if(BOARD_VERSION == BOARD_V1_2_LE)
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

#elif(BOARD_VERSION == BOARD_V1_3)

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

    /* pwm IO.*/
    fpioa_set_function(LED_IR_PIN, FUNC_TIMER0_TOGGLE2 + 1 * 4);
    /* Init PWM */
    pwm_init(1);
    pwm_set_frequency(1, 1, 3000, 0.3);
    pwm_set_enable(1, 1, 0);

#elif(BOARD_VERSION == BOARD_KD233)
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

    /* KEY IO map and function settings */
    fpioa_set_function(KEY_PIN, FUNC_GPIOHS0 + KEY_IO);
    gpiohs_set_drive_mode(KEY_IO, GPIO_DM_INPUT_PULL_UP);
    gpiohs_set_pin_edge(KEY_IO, GPIO_PE_BOTH);

    gpiohs_irq_register(KEY_IO, 1, key_triger, NULL);
}

static void io_set_power(void)
{
#if(BOARD_VERSION == BOARD_V1_2_LE) || (BOARD_VERSION == BOARD_V1_3)
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

#elif(BOARD_VERSION == BOARD_KD233)
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK0, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
#endif
}

static void irled(uint32_t onoff)
{
#if(BOARD_VERSION == BOARD_V1_3)
    int enable = onoff ? 1 : 0;
    pwm_set_enable(1, 1, enable);
#endif
}

static void camera_switch(void)
{
#if(BOARD_VERSION == BOARD_V1_3)

    g_camera_no = !g_camera_no;
    g_camera_no ? open_gc0328_0() : open_gc0328_1();

    int enable = g_camera_no ? 1 : 0;
    irled(enable);

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

static void draw_key_point(uint32_t *gram, key_point_t *key_point, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr;

    for(uint32_t i = 0; i < 5; i++)
    {
        addr = gram + (320 * (key_point->point[i].y - 8) + key_point->point[i].x) / 2;
        *addr = data;
        *(addr + 160) = data;
    }
}

int main(void)
{
    /* Set CPU and dvp clk */
    sysctl_pll_set_freq(SYSCTL_PLL0, 800000000UL);
    sysctl_pll_set_freq(SYSCTL_PLL1, 400000000UL);
    sysctl_clock_enable(SYSCTL_CLOCK_AI);
    saved_face_feature_init();
    uarths_init();
    io_set_power();

    plic_init();
    io_mux_init();
    /* flash init */
    printf("flash init [TEST]\n");
    w25qxx_init(3, 0);
    w25qxx_enable_quad_mode();

    /*irled_on();*/
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(LCD_XYZ);

    lcd_clear(BLACK);
    /* DVP init */
    printf("DVP init\n");
    dvp_init(CAMERA_REG_LENGTH);
    dvp_set_xclk_rate(CAMERA_XCLK_RATE);
    dvp_enable_burst();
    dvp_set_image_format(DVP_CFG_RGB_FORMAT);
    dvp_set_image_size(320, 240);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 0);
    dvp_disable_auto();
    printf("DVP interrupt config\n");
    plic_set_priority(IRQN_DVP_INTERRUPT, 1);
    plic_irq_register(IRQN_DVP_INTERRUPT, dvp_irq, NULL);
    plic_irq_enable(IRQN_DVP_INTERRUPT);

#if OV2640
    ov2640_init();
#elif OV5640
    ov5640_init();
#elif GC0328
    gc0328_init();
    open_gc0328_1();
#endif

    /*memory malloc.*/
    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 256;
    image_init(&kpu_image);
    dvp_set_ai_addr((uint32_t)kpu_image.addr + 320 * (256 * 0 + 8), (uint32_t)(kpu_image.addr + 320 * (256 * 1 + 8)), (uint32_t)(kpu_image.addr + 320 * (256 * 2 + 8)));

    dvp_set_output_enable(0, 1);

    resized_image.pixel = 3;
    resized_image.width = 128;
    resized_image.height = 128;
    image_init(&resized_image);

    st_image.pixel = 3;
    st_image.width = 64;
    st_image.height = 64;
    image_init(&st_image);

    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_set_output_enable(1, 1);

    /*ai init*/
    ai_init_license(license, 3);
    ai_init_model_address(0x200000, 0x400000, 0x600000, 0x800000, 0xa00000);
    ai_init_kpu_image_address(kpu_image.addr);
    ai_load_model_all();

    timer_init(0);
    timer_irq_register(0, 0, 0, 1, isr_tick, NULL);
    timer_set_interval(0, 0, 10000000);
    timer_set_enable(0, 0, 1);

    /* enable global interrupt */
    sysctl_enable_irq();

    /* system start */
    printf("System start\n");

    float margin = 0.0f, face_thresh = FACE_RECGONITION_SCORE;
    while(1)
    {
        uint8_t key_v = key_get();

        if(key_v == KEY_LONGPRESS)
        {
            printf("KEY_LONGPRESS hit\n");
            /* Del All */
            registered_face_number = 0;
            saved_face_feature_init();
        } else if(key_v == KEY_DOUBLE)
        {
            printf("KEY_DOUBLE hit\n");
            camera_switch();
        }
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while(g_dvp_finish_flag == 0)
            ;
        uint64_t v_time_start = sysctl_get_time_us();
        ai_run(FACE_KMODEL, &obj_detect_info);
        uint32_t face_found = 0;
        for(uint32_t i = 0; i < obj_detect_info.obj_number; i++)
        {
            uint32_t x1_tmp = (uint32_t)obj_detect_info.obj[i].x1;
            uint32_t x2_tmp = (uint32_t)obj_detect_info.obj[i].x2;
            uint32_t y1_tmp = (uint32_t)obj_detect_info.obj[i].y1;
            uint32_t y2_tmp = (uint32_t)obj_detect_info.obj[i].y2;
            image_absolute_src_resize(&kpu_image, &resized_image, &x1_tmp, &y1_tmp, &x2_tmp, &y2_tmp, margin, false);
            ai_run_keypoint(&kpu_image, &resized_image, &st_image, x1_tmp, y1_tmp, x2_tmp, y2_tmp, &key_point);
            draw_key_point(display_image.addr, &key_point, RED);
            ai_run_feature(&st_image, i);
        }
        uint64_t v_time_consume = sysctl_get_time_us() - v_time_start;
        printf("all feature extract for %d person need time is %ld us!\n", obj_detect_info.obj_number, v_time_consume);
        if(key_v == KEY_PRESS)
        {
            printf("press the key!\n");
            if(obj_detect_info.obj_number != 1)
            {
                printf("Can't register when no face or more than one face detectd!\n");
            } else
            {
                if(registered_face_number >= MAX_REGISTERED_FACE_NUMBER)
                {
                    printf("Can't register when regiter full!\n");
                } else
                {
                    if(FACE_REGISTER_MODE == FACE_SUBSTITUTE)
                    {
                        memcpy(&registered_face_features[FEATURE_LEN * registered_face_number], features_tmp[0], sizeof(float) * FEATURE_LEN);
                    } else
                    {
                        memcpy(&registered_face_features[FEATURE_LEN * registered_face_number++], features_tmp[0], sizeof(float) * FEATURE_LEN);
                    }
                    printf("Register Succeed!\n");
                }
            }
        }
        for(uint32_t i = 0; i < obj_detect_info.obj_number; i++)
        {
            float score_max = 0;
            uint32_t score_index = 0;
            score_index = calulate_score(features_tmp[i], registered_face_features, MAX_REGISTERED_FACE_NUMBER, &score_max);
            if(score_max >= face_thresh)
            {
                draw_edge((uint32_t *)display_image.addr, &obj_detect_info, i, GREEN);
                sprintf(display_status, "%s_%d", "recognized", score_index);
                ram_draw_string(display_image.addr, obj_detect_info.obj[i].x1, obj_detect_info.obj[i].y1 - 16, display_status, GREEN);
                printf("RECOGNIZED FACE: <%d> <%d> <%d> <%d>\n", obj_detect_info.obj[i].x1, obj_detect_info.obj[i].y1, obj_detect_info.obj[i].x2, obj_detect_info.obj[i].y2);
            } else
            {
                draw_edge((uint32_t *)display_image.addr, &obj_detect_info, i, RED);
                printf("DETECTED FACE: <%d> <%d> <%d> <%d>\n", obj_detect_info.obj[i].x1, obj_detect_info.obj[i].y1, obj_detect_info.obj[i].x2, obj_detect_info.obj[i].y2);
            }
        }
        ai_run(HEAD_KMODEL, &obj_detect_info);
        for(uint32_t i = 0; i < obj_detect_info.obj_number; i++)
        {
            draw_edge(display_image.addr, &obj_detect_info, i, WHITE);
            printf("HEAD: <%d> <%d> <%d> <%d>\n", obj_detect_info.obj[i].x1, obj_detect_info.obj[i].y1, obj_detect_info.obj[i].x2, obj_detect_info.obj[i].y2);
        }

        ai_run(BODY_KMODEL, &obj_detect_info);
        for(uint32_t i = 0; i < obj_detect_info.obj_number; i++)
        {
            draw_edge(display_image.addr, &obj_detect_info, i, YELLOW);
            printf("BODY: <%d> <%d> <%d> <%d>\n", obj_detect_info.obj[i].x1, obj_detect_info.obj[i].y1, obj_detect_info.obj[i].x2, obj_detect_info.obj[i].y2);
        }
        /* display pic*/
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);
    }
}
