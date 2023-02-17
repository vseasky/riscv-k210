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
#include "image_process.h"
#include "w25qxx.h"
#include "board_config.h"
#include "tracker.c"


#define LOAD_KMODEL_FROM_FLASH 0

#if LOAD_KMODEL_FROM_FLASH

#define PERSON_MODEL_ADDR (0xA00000)
#define PERSON_MODEL_SIZE (1300 * 1024)
uint8_t person_model_data[PERSON_MODEL_SIZE];
#define OBJECT_MODEL_ADDR (0xE00000)
#define OBJECT_MODEL_SIZE (1300 * 1024)
uint8_t object_model_data[OBJECT_MODEL_SIZE];
#else

#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

INCBIN(person_model, "person.kmodel");
INCBIN(object_model, "object.kmodel");
#endif


volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;

kpu_model_context_t person_detect_task;
kpu_model_context_t object_detect_task;

#define N_PERSON_ANCHOR 15
static float person_anchors[][4] = {
    {7.500000, 7.500000, 6.444802, 17.378922},
    {7.500000, 7.500000, 27.070000, 32.411266},
    {7.499999, 7.500000, 59.323946, 145.040344},
    {7.500000, 7.500000, 10.376238, 12.129096},
    {7.500000, 7.500000, 78.554626, 86.740006},
    {7.500000, 7.500000, 15.523774, 41.475006},
    {7.500000, 7.500000, 186.322494, 216.810502},
    {7.500000, 7.499999, 42.645004, 54.964464},
    {7.500000, 7.500000, 4.783142, 9.161744},
    {7.500000, 7.500000, 2.699996, 4.678788},
    {7.500000, 7.500000, 37.044770, 98.219254},
    {7.500000, 7.500000, 23.190628, 64.775010},
    {7.500000, 7.500000, 16.653870, 20.397190},
    {7.500000, 7.500000, 9.910644, 27.630906},
    {7.500000, 7.500000, 106.485000, 175.425660}};

static float object_anchors[9][4] = {
    {3.500000, 3.500000, 18.823532, 18.113204},
    {3.500000, 3.500000, 12.715226, 13.913040},
    {3.500000, 3.500000, 15.705536, 7.084870},
    {3.500000, 3.500000, 9.454804, 8.988766},
    {3.500000, 3.500000, 23.606552, 11.636368},
    {3.500000, 3.500000, 23.486236, 30.476188},
    {3.500000, 3.500000, 33.485946, 20.425538},
    {3.500000, 3.500000, 41.010974, 36.923080},
    {3.500000, 3.500000, 7.307332, 5.000000}};

#define N_COLOR (19)
	static uint16_t colors[] = {
		0x0000,
		0x000F,
		0x03E0,
		0x03EF,
		0x7800,
		0x780F,
		0x7BE0,
		0xC618,
		0x7BEF,
		0x001F,
		0x07E0,
		0x07FF,
		0xF81F,
		0xFFE0,
		0xFFFF,
		0xFD20,
		0xAFE5,
		0xF81F,
		0xAA55};


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

static void draw_object(tracker_t * tracker)
{
    for (uint16_t i = 0; i < tracker->max_n_object; ++i)
    {
        obj_t *obj = &(tracker->objects[i]);
        if (!(obj->is_valid))
            continue;

        float half_w = obj->w / 2;
        float half_h = obj->h / 2;
        uint32_t x1 = obj->cx - half_w;
        uint32_t y1 = obj->cy - half_h;
        uint32_t x2 = obj->cx + half_w;
        uint32_t y2 = obj->cy + half_h;

        if (x1 <= 0)
            x1 = 1;
        if (x2 >= 319)
            x2 = 318;
        if (y1 <= 0)
            y1 = 1;
        if (y2 >= 239)
            y2 = 238;

        uint16_t color = colors[obj->id % N_COLOR];
        static char display_buf[64];
        if (obj->class == HUMAN)
        {
            sprintf(display_buf, "Person %d", obj->id);
            if (obj->interaction_state == STATE_LITTERER)
            {
                color = RED;
                sprintf(display_buf + strlen(display_buf), ": %d%%", (int)(100 * obj->prob_is_litterer));
            }
        }
        else
        {
            sprintf(display_buf, "Litter %d", obj->id);
            if (obj->interaction_state == STATE_DISCARDED)
                color = RED;
            else
                color = YELLOW;
        }
        lcd_draw_string(x1 + 10 >= 319 ? 318 : x1 + 10, y2 - 20 < 0 ? 0 : y2 - 20, display_buf, color);
        lcd_draw_rectangle(x1, y1, x2, y2, 2, color);
    }
}

static void draw_edge(uint32_t *gram, float *box, uint16_t color)
{
    float *x = get_x(box);
    float *y = get_y(box);
    float *w = get_w(box);
    float *h = get_h(box);
    uint32_t x1, y1, x2, y2;
    x1 = *x - *w / 2;
    y1 = *y - *h / 2;
    x2 = *x + *w / 2;
    y2 = *y + *h / 2;

    if (x1 <= 0)
        x1 = 1;
    if (x2 >= 319)
        x2 = 318;
    if (y1 <= 0)
        y1 = 1;
    if (y2 >= 239)
        y2 = 238;
    lcd_draw_rectangle(x1, y1, x2, y2, 3, color);
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
    w25qxx_read_data(PERSON_MODEL_ADDR, person_model_data, PERSON_MODEL_SIZE, W25QXX_QUAD_FAST);
    w25qxx_read_data(OBJECT_MODEL_ADDR, object_model_data, OBJECT_MODEL_SIZE, W25QXX_QUAD_FAST);
#endif

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

    kpu_image.pixel= 3;
    kpu_image.width= 320;
    kpu_image.height= 240;
    image_init(&kpu_image);
	dvp_set_ai_addr((uint32_t)kpu_image.addr, (uint32_t)(kpu_image.addr + 320 * 240), (uint32_t)(kpu_image.addr + 320 * 240 * 2));
	dvp_set_output_enable(0, 1);
	
    display_image.pixel= 2;
    display_image.width= 320;
    display_image.height= 240;
    image_init(&display_image);
    dvp_set_display_addr((uint32_t)display_image.addr);
	dvp_set_output_enable(1, 1);

    /* init face detect model */
    if (kpu_load_kmodel(&person_detect_task, person_model_data) != 0)
    {
        printf("\nperson model init error\n");
        while (1)
            ;
    }
    if (kpu_load_kmodel(&object_detect_task, object_model_data) != 0)
    {
        printf("\nobject model init error\n");
        while (1)
            ;
    }
    /* enable global interrupt */
    sysctl_enable_irq();
    /* system start */
    printf("System start\n");

    /* resources */
    static uint16_t n_box_limit = 10;
    static float *boxes[10];
    static uint16_t n_obj = 10;
    static obj_t objects[10];
    static tracker_t tracker;
    init_tracker(&tracker, objects, n_obj);

    static float *output;
    static size_t output_size;
    static uint16_t n_result;

    while (1)
    {
        g_dvp_finish_flag = 0;
        dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
        dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
        while (g_dvp_finish_flag == 0)
            ;

        // person detector
        g_ai_done_flag = 0;
        kpu_run_kmodel(&person_detect_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag)
            ;

        kpu_get_output(&person_detect_task, 0, (uint8_t **)&output, &output_size);

        n_result = get_boxes(boxes, n_box_limit, output, 15, 20, person_anchors, N_PERSON_ANCHOR, 0.7, 0.15);
        update_tracker(&tracker, boxes, n_result, HUMAN, DIST_THRESH);

        // object detector
        g_ai_done_flag = 0;
        kpu_run_kmodel(&object_detect_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag)
            ;

        kpu_get_output(&object_detect_task, 0, (uint8_t **)&output, &output_size);

        n_result = get_boxes_8(boxes, n_box_limit, output, 30, 40, object_anchors, 9, 0.5, 0.15);
        update_tracker_8(&tracker, boxes, n_result, OBJECT, DIST_MAX);

        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);
        draw_object(&tracker);
    }
}
