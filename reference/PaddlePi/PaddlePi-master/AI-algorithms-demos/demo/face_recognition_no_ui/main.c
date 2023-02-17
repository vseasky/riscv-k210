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
#include "key_point.h"
#include "flash.h"


#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"
INCBIN(detect_model, "detect.kmodel");
INCBIN(key_point_model, "key_point.kmodel");
INCBIN(feature_model, "feature.kmodel");


volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;

static kpu_task_t detect_task;
kpu_model_layer_metadata_t *detect_meta;

static region_layer_t detect_rl;
static obj_info_t detect_info;
#define ANCHOR_NUM 5
static float anchor[ANCHOR_NUM * 2] = {1.889,2.5245,  2.9465,3.94056, 3.99987,5.3658, 5.155437,6.92275, 6.718375,9.01025};

static kpu_task_t key_point_task;
kpu_model_layer_metadata_t *key_point_meta;

static image_t crop_image, resize_image;
static key_point_t key_point;

static image_t similarity_image;
kpu_model_layer_metadata_t *feature_meta;

static kpu_task_t feature1_task;
static kpu_task_t feature2_task;
extern unsigned char gImage_image[];

static char display_string[16];
static uint32_t person_count = 0;
#define PERSON_MAX 20
#define FEATURE_DIMENSION 196
static float features_save[PERSON_MAX][FEATURE_DIMENSION];


static void ai_done(void)
{
    g_ai_done_flag = 1;
}

void l2normalize(float *x, float *dx, int len)
{
    int f;
    float sum = 0;
    for(f = 0; f < len; ++f){
        sum += x[f]*x[f];
    }
    sum = sqrtf(sum);
    for(f = 0; f < len; ++f){
		dx[f] = x[f] / sum;
    }
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

volatile uint32_t g_gpio_flag = 0;
volatile uint32_t g_key_press = 0;
volatile uint64_t g_gpio_time = 0;
int irq_gpiohs(void* ctx)
{
    printf("Key Working\n");
    g_gpio_flag = 1;
    g_gpio_time = sysctl_get_time_us();
    return 0;
}

void handle_key(void)
{
    if(g_gpio_flag)
    {
        uint64_t v_time_now = sysctl_get_time_us();
        if(v_time_now - g_gpio_time > 10*1000)
        {
            if(gpiohs_get_pin(KEY_IO) == 1)/*press */
            {
                g_key_press = 1;
                g_gpio_flag = 0;
            }
        }
        if(v_time_now - g_gpio_time > 1 * 1000 * 1000) /* 长按1s */
        {
            if(gpiohs_get_pin(KEY_IO) == 0)
            {
                /* Del All */
                printf("Del All feature!\n");
                flash_delete_face_all();
                g_gpio_flag = 0;
            }
        }
    }
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

	fpioa_set_function(KEY_PIN, FUNC_GPIOHS0 + KEY_IO);
	gpiohs_set_drive_mode(KEY_IO, GPIO_DM_INPUT_PULL_UP);
	gpiohs_set_pin_edge(KEY_IO, GPIO_PE_FALLING);
	gpiohs_irq_register(KEY_IO, 1, irq_gpiohs, NULL);
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

static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = obj_info->obj[index].x1;
    y1 = obj_info->obj[index].y1;
    x2 = obj_info->obj[index].x2;
    y2 = obj_info->obj[index].y2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;

    addr1 = gram + (320 * y1 + x1) / 2;
    addr2 = gram + (320 * y1 + x2 - 8) / 2;
    addr3 = gram + (320 * (y2 - 1) + x1) / 2;
    addr4 = gram + (320 * (y2 - 1) + x2 - 8) / 2;
    for (uint32_t i = 0; i < 4; i++)
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
    for (uint32_t i = 0; i < 8; i++)
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

static void draw_key_point(uint32_t *gram, key_point_t *key_point)
{
    uint32_t data = ((uint32_t)RED << 16) | (uint32_t)RED;
    uint32_t *addr;

    for (uint32_t i = 0; i < 5; i++)
    {
        addr = gram + (320 * key_point->point[i].y + key_point->point[i].x) / 2;
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
    uarths_init();
    io_set_power();
    plic_init();
    io_mux_init();
	
    /* flash init */
    flash_init();

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

    kpu_image.pixel = 3;
    kpu_image.width = 320;
    kpu_image.height = 240;
    image_init(&kpu_image);
	dvp_set_ai_addr((uint32_t)kpu_image.addr, (uint32_t)(kpu_image.addr + 320 * 240), (uint32_t)(kpu_image.addr + 320 * 240 * 2));	
	dvp_set_output_enable(0, 1);
	
    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_display_addr((uint32_t)display_image.addr);
	dvp_set_output_enable(1, 1);


    /* init face detect model */
    kpu_model_load_from_buffer(&detect_task, detect_model_data, &detect_meta);
    detect_task.src = kpu_image.addr;
    detect_task.dma_ch = 5;
    detect_task.callback = ai_done;
    kpu_single_task_init(&detect_task);
    detect_rl.anchor_number = ANCHOR_NUM;
    detect_rl.anchor = anchor;
    detect_rl.threshold = 0.7;
    detect_rl.nms_value = 0.3;
    region_layer_init(&detect_rl, &detect_task);
    /* init key point detect model */
    resize_image.pixel = 3;
    resize_image.width = 128;
    resize_image.height = 128;
    image_init(&resize_image);
    kpu_model_load_from_buffer(&key_point_task, key_point_model_data, &key_point_meta);
    key_point_task.src = resize_image.addr;
    key_point_task.dma_ch = 5;
    key_point_task.callback = ai_done;
    kpu_single_task_init(&key_point_task);
    /* init feature model */
    kpu_model_load_from_buffer(&feature1_task, feature_model_data, &feature_meta);
    feature1_task.layers_length = 27;
    feature1_task.dma_ch = 5;
    feature1_task.callback = ai_done;
    kpu_single_task_init(&feature1_task);
    kpu_model_load_from_buffer(&feature2_task, feature_model_data, &feature_meta);
    feature2_task.layers = &feature2_task.layers[27];
    feature2_task.layers_length = 1;
    feature2_task.src = NULL;
    feature2_task.dma_ch = 5;
    feature2_task.callback = ai_done;
    kpu_single_task_init(&feature2_task);
	
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    printf("System start\n");
    while (1)
    {
        handle_key();
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;
        /* run face detect */
        g_ai_done_flag = 0;
        kpu_start(&detect_task);
        while(!g_ai_done_flag);
        region_layer_run(&detect_rl, &detect_info);
        /* run key point detect */
        for (uint32_t face_cnt = 0; face_cnt < detect_info.obj_number; face_cnt++)
        {
            crop_image.pixel = 3;
            crop_image.width = detect_info.obj[face_cnt].x2 - detect_info.obj[face_cnt].x1 + 1;
            crop_image.height = detect_info.obj[face_cnt].y2 - detect_info.obj[face_cnt].y1 + 1;
            key_point.width = crop_image.width;
            key_point.height = crop_image.height;
            if (crop_image.width < 80 || crop_image.height < 100)
            {
                draw_edge(display_image.addr, &detect_info, face_cnt, RED);
                continue;
            }
            image_init(&crop_image);
            image_crop(&kpu_image, &crop_image, detect_info.obj[face_cnt].x1, detect_info.obj[face_cnt].y1);
            image_resize(&crop_image, &resize_image);
            image_deinit(&crop_image);
            g_ai_done_flag = 0;
            kpu_start(&key_point_task);
            while(!g_ai_done_flag);
            key_point_last_handle(&key_point_task, &key_point);
            double matrix_src[5][2], matrix_dst[10];
            for (uint32_t point_cnt = 0; point_cnt < 5; point_cnt++)
            {
                key_point.point[point_cnt].x += detect_info.obj[face_cnt].x1;
                key_point.point[point_cnt].y += detect_info.obj[face_cnt].y1;
                matrix_src[point_cnt][0] = key_point.point[point_cnt].x;
                matrix_src[point_cnt][1] = key_point.point[point_cnt].y;
            }
            draw_key_point(display_image.addr, &key_point);
            image_umeyama(&matrix_src, &matrix_dst);
            image_similarity(&kpu_image, &resize_image, matrix_dst);

            feature1_task.src = resize_image.addr;
            g_ai_done_flag = 0;
            kpu_start(&feature1_task);
            while (!g_ai_done_flag);
            quantize_param_t q1 = {.scale = 0.0125532000672583, .bias = 0 }, q2 = {.scale = 0.00478086798798804, .bias = 0};
            kpu_global_average_pool(feature1_task.dst, &q1, 16, 1024, AI_IO_BASE_ADDR + feature2_task.layers[0].image_addr.data.image_src_addr * 64, &q2);
            g_ai_done_flag = 0;
            kpu_start(&feature2_task);
            while (!g_ai_done_flag);
            quantize_param_t q = {.scale = feature2_task.output_scale, .bias = feature2_task.output_bias};
            float features[FEATURE_DIMENSION], features_tmp[FEATURE_DIMENSION];
            kpu_matmul_end(feature2_task.dst, FEATURE_DIMENSION, features, &q);
            l2normalize(features, features_tmp, FEATURE_DIMENSION);

            if(g_key_press)
            {
                if(flash_save_face_info(NULL, features_tmp) < 0)
                {
                    printf("Feature Full\n");
                    break;
                }
                g_key_press = 0;
            }

            float score, score_max = 0;
            uint32_t score_index = 0;
            score_index = calulate_score(features_tmp, &score_max);

            sprintf(display_string, "%.2f", score_max);
            if (score_max > FACE_RECGONITION_SCORE)
            {
                draw_edge(display_image.addr, &detect_info, face_cnt, GREEN);
                sprintf(display_string, "%d  %.2f", score_index, score_max);
                ram_draw_string(display_image.addr, detect_info.obj[face_cnt].x1, detect_info.obj[face_cnt].y1 - 16, display_string, GREEN);
            }
            else
            {
                draw_edge(display_image.addr, &detect_info, face_cnt, RED);
                ram_draw_string(display_image.addr, detect_info.obj[face_cnt].x1, detect_info.obj[face_cnt].y1 - 16, display_string, RED);
            }
        }

        /* display result */
        lcd_draw_picture(0, 0, 320, 240, display_image.addr);
    }
}
