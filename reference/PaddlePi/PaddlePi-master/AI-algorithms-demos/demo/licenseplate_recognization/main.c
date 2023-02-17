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
#include "fully.h"

#define LOAD_KMODEL_FROM_FLASH  0

#if LOAD_KMODEL_FROM_FLASH
#define KMODEL_SIZE (500 * 1024)
uint8_t model_data[KMODEL_SIZE];
#define RECOG_SIZE (700 * 1024)
uint8_t recog_data[RECOG_SIZE];
#else

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

INCBIN(model, "detect.kmodel");
INCBIN(recog, "recog.kmodel");
#endif


volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;
static image_t cropped, resized;

kpu_model_context_t lp_detect_task;
kpu_model_context_t recog_task;
static region_layer_t lp_detect_rl;
static obj_info_t lp_detect_info;
#define ANCHOR_NUM 5
static float anchor[ANCHOR_NUM * 2] = {8.30891522166988, 2.75630994889035, 5.18609903718768, 1.7863757404970702, 6.91480529053198, 3.825771881004435, 10.218567655549439, 3.69476690620971, 6.4088204258368195, 2.38813526350986};


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

static void extend_box(obj_info_t *obj_info, uint32_t index, uint32_t margin)
{
    uint32_t x1 = obj_info->obj[index].x1;
    uint32_t y1 = obj_info->obj[index].y1;
    uint32_t x2 = obj_info->obj[index].x2;
    uint32_t y2 = obj_info->obj[index].y2;

    if (x1 <= margin)
        x1 = 0;
    else 
        x1 -= margin;
    if (x2 >= 319 - margin)
        x2 = 319;
    else
        x2 += margin;
    if (y1 <= margin)
        y1 = 0;
    else
        y1 -= margin;
    if (y2 >= 239 - margin)
        y2 = 239;
    else
        y2 += margin;

    obj_info->obj[index].x1 = x1;
    obj_info->obj[index].y1 = y1;
    obj_info->obj[index].x2 = x2;
    obj_info->obj[index].y2 = y2;
}

static void get_cropped(image_t *src_image, image_t *dst_image, obj_info_t *obj_info, uint32_t index)
{
    uint32_t x1 = obj_info->obj[index].x1;
    uint32_t y1 = obj_info->obj[index].y1;
    uint32_t x2 = obj_info->obj[index].x2;
    uint32_t y2 = obj_info->obj[index].y2;
    uint32_t h = y2 - y1 + 1;
    uint32_t w = x2 - x1 + 1;

    crop_image(src_image->addr, x1, y1, w, h, dst_image->addr);
    dst_image->height = h;
    dst_image->width = w;
}

static void get_resized(image_t *src, image_t *dst)
{
    uint32_t src_h = src->height;
    uint32_t src_w = src->width;

    uint32_t dst_h = dst->height;
    uint32_t dst_w = dst->width;

    image_resize(src->addr, src_w, src_h, dst_w, dst_h, dst->addr);
    image_resize(src->addr+src_w*src_h, src_w, src_h, dst_w, dst_h, dst->addr+dst_w*dst_h);
    image_resize(src->addr+src_w*src_h*2, src_w, src_h, dst_w, dst_h, dst->addr+dst_w*dst_h*2);
}

static float *get_recog_kpu_result(size_t *output_size)
{
    g_ai_done_flag = 0;
    kpu_run_kmodel(&recog_task, resized.addr, DMAC_CHANNEL5, ai_done, NULL);
    while(!g_ai_done_flag);
    static float *result;
    kpu_get_output(&recog_task, 0, (uint8_t **)&result, output_size);
    return result;
}

static void run_fully(const float *features, const uint32_t len, const float *weight, const float *bias, const uint32_t n_logit, float *result)
{
    memset(result, 0, sizeof(float)*n_logit);
    for(uint32_t i=0; i<n_logit; ++i){
        for(uint32_t j=0; j<len; ++j)
            result[i] += features[j] * weight[i*len + j];
        result[i] += bias[i];
    }
}

static uint32_t argmax(const float *vec, const uint32_t len)
{
    uint32_t max_index = 0;
    for(uint32_t i=1; i<len; ++i)
        if(vec[i] > vec[max_index])
            max_index = i;
    return max_index;
}

char* decode_chinese(const uint32_t max_index){
	static const char *chinese[31] = {"Wan", "Hu", "Jin", "Yu^", "Ji", "Sx", "Meng", "Liao", "Jl", "Hei", "Su", "Zhe", "Jing", "Min", "Gan", "Lu", "Yu", "E^", "Xiang", "Yue", "Gui^", "Qiong", "Cuan", "Gui", "Yun", "Zang", "Shan", "Gan^", "Qing", "Ning", "Xin"};
	
	return chinese[max_index];
}

char decode(const uint8_t max_index){
	static const char ads[34] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};	
    return ads[max_index];
}

static void *get_recog_result(const float *features, uint32_t size, char *result)
{
    static float fully_result[34]={0};
	uint32_t max_index;
    uint32_t chunk = size / 7;
    char *d1;
    char d2, d3, d4, d5, d6, d7;

    run_fully(features, chunk, weight1, bias1, 31, fully_result);
    max_index = argmax(fully_result, 31);
    d1 = decode_chinese(max_index);

    features += chunk;
    run_fully(features, chunk, weight2, bias2, 24, fully_result);
    max_index = argmax(fully_result, 24);
    d2 = decode(max_index);

    features += chunk;
    run_fully(features, chunk, weight3, bias3, 34, fully_result);
    max_index = argmax(fully_result, 34);
    d3 = decode(max_index);

    features += chunk;
    run_fully(features, chunk, weight4, bias4, 34, fully_result);
    max_index = argmax(fully_result, 34);
    d4 = decode(max_index);

    features += chunk;
    run_fully(features, chunk, weight5, bias5, 34, fully_result);
    max_index = argmax(fully_result, 34);
    d5 = decode(max_index);

    features += chunk;
    run_fully(features, chunk, weight6, bias6, 34, fully_result);
    max_index = argmax(fully_result, 34);
    d6 = decode(max_index);

    features += chunk;
    run_fully(features, chunk, weight7, bias7, 34, fully_result);
    max_index = argmax(fully_result, 34);
    d7 = decode(max_index);
	
	sprintf(result, "%s%c%c%c%c%c%c", d1, d2, d3, d4, d5, d6, d7);
}

static void run_recog(image_t *src_image, obj_info_t *obj_info, uint32_t index)
{
    extend_box(obj_info, index, 10);
    get_cropped(src_image, &cropped, obj_info, index);
    get_resized(&cropped, &resized);

    static size_t size;
    float *features = get_recog_kpu_result(&size);
    static char result[32] = {0};
    get_recog_result(features, size / 4, result);
    lcd_draw_string(320 - obj_info->obj[index].x2, obj_info->obj[index].y1, result, RED);
}

static void draw_edge(uint32_t *gram, obj_info_t *obj_info, uint32_t index, uint16_t color)
{
    uint32_t data = ((uint32_t)color << 16) | (uint32_t)color;
    uint32_t *addr1, *addr2, *addr3, *addr4, x1, y1, x2, y2;

    x1 = 320 - obj_info->obj[index].x2;
    y1 = obj_info->obj[index].y1;
    x2 = 320 - obj_info->obj[index].x1;
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
    w25qxx_read_data(0xB00000, recog_data, RECOG_SIZE, W25QXX_QUAD_FAST);
#endif
    /* LCD init */
    printf("LCD init\n");
    lcd_init();
    lcd_set_direction(LCD_XYZ);

    lcd_clear(BLACK);
    lcd_draw_string(136, 70, "DEMO", WHITE);
    lcd_draw_string(104, 150, "68 facial landmarks", WHITE);
	
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

    resized.pixel = 3;
    resized.width = 208;
    resized.height = 64;
    image_init(&resized);

    cropped.pixel = 3;
    cropped.width = 320;
    cropped.height = 240;
    image_init(&cropped);

    display_image.pixel = 2;
    display_image.width = 320;
    display_image.height = 240;
    image_init(&display_image);
    dvp_set_display_addr((uint32_t)display_image.addr);
    dvp_set_output_enable(1, 1);
	
    /* init lp detect model */
    if (kpu_load_kmodel(&lp_detect_task, model_data) != 0)
    {
        printf("\ndetect model init error\n");
        while (1);
    }
    if (kpu_load_kmodel(&recog_task, recog_data) != 0)
    {
        printf("\nrecog model init error\n");
        while (1);
    }
    lp_detect_rl.anchor_number = ANCHOR_NUM;
    lp_detect_rl.anchor = anchor;
    lp_detect_rl.threshold = 0.7;
    lp_detect_rl.nms_value = 0.3;
    region_layer_init(&lp_detect_rl, 20, 15, 30, kpu_image.width, kpu_image.height);
	
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    printf("System start\n");
    while (1)
    {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;

        /* run lp detect */
		
		for(uint32_t cc = 0; cc < kpu_image.pixel; cc++)
		{
			for(uint32_t hh = 0; hh < kpu_image.height; hh++)
			{
				flip_left_right(&kpu_image.addr[cc * kpu_image.height * kpu_image.width + hh * kpu_image.width], kpu_image.width / 2);
			}
		}
		
        g_ai_done_flag = 0;
        kpu_run_kmodel(&lp_detect_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while(!g_ai_done_flag);
        float *output;
        size_t output_size;
        kpu_get_output(&lp_detect_task, 0, (uint8_t **)&output, &output_size);
        lp_detect_rl.input = output;
        region_layer_run(&lp_detect_rl, &lp_detect_info);
		
		/*
		for(uint32_t hh = 0; hh < display_image.height; hh++)
		{
			flip_left_right(&display_image.addr[hh * display_image.width * display_image.pixel], display_image.width);	
		}
		*/

        /* run key point detect */
        for (uint32_t lp_cnt = 0; lp_cnt < lp_detect_info.obj_number; lp_cnt++)
        {
            draw_edge((uint32_t *)display_image.addr, &lp_detect_info, lp_cnt, RED);
            //run_recog(&kpu_image, &lp_detect_info, lp_cnt);
        }

        /* display result */
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);

        /* run recog and display result */
        for (uint32_t lp_cnt = 0; lp_cnt < lp_detect_info.obj_number; lp_cnt++)
            run_recog(&kpu_image, &lp_detect_info, lp_cnt);
    }
}
