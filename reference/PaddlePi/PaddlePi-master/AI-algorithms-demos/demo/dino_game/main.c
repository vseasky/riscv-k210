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

#include "images/dino.h"
#include "images/bird.h"
#include "images/obstacle.h"
#include "images/cloud.h"

#define LOAD_KMODEL_FROM_FLASH 0

#if LOAD_KMODEL_FROM_FLASH

#define PERSON_MODEL_ADDR (0xA00000)
#define PERSON_MODEL_SIZE (1400 * 1024)
uint8_t person_model_data[PERSON_MODEL_SIZE];
#define SEGMENT_MODEL_ADDR (0xC00000)
#define SEGMENT_MODEL_SIZE (1600 * 1024)
uint8_t segment_model_data[SEGMENT_MODEL_SIZE];

#else
#define INCBIN_STYLE INCBIN_STYLE_SNAKE
#define INCBIN_PREFIX
#include "incbin.h"

INCBIN(person_model, "person.kmodel");
INCBIN(segment_model, "segment.kmodel");
#endif


volatile uint32_t g_ai_done_flag;
volatile uint8_t g_dvp_finish_flag;
static image_t kpu_image, display_image;

kpu_model_context_t person_detect_task;
kpu_model_context_t person_segment_task;

#define SEGMENT_THRESH (2.7)
#define N_PERSON_ANCHOR (15)

static float person_anchors[][4] =
{
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

uint16_t colors[] = {
	BLACK,
	NAVY,
	DARKGREEN,
	DARKCYAN,
	MAROON,
	PURPLE,
	OLIVE,
	LIGHTGREY,
	DARKGREY,
	BLUE,
	GREEN,
	CYAN,
	RED,
	MAGENTA,
	YELLOW,
	WHITE,
	ORANGE,
	GREENYELLOW,
	PINK,
	USER_COLOR};



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

static void draw_object(tracker_t *tracker)
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

        uint16_t color = colors[obj->id % 20];
        lcd_draw_rectangle(x1, y1, x2, y2, 2, color);
        static char buf[10];
        sprintf(buf, "%d", obj->id);
        lcd_draw_string(x1 + 5, y2 - 10, buf, color);
    }
}

static uint32_t transform_index(uint16_t h, uint16_t w)
{
    static const uint16_t block_size = 16;
    static uint16_t left, block_h, block_w, in_block_h, in_block_w;
    static uint32_t transformed_depth, segment_index, left_mask, right_mask, mask;

    left = 2 * w;

    block_h = h / block_size;
    block_w = left / block_size;

    in_block_h = h % block_size;
    in_block_w = left % block_size;

    transformed_depth = in_block_h * block_size + in_block_w;
    segment_index = transformed_depth * 300 + block_h * 20 + block_w;
    return segment_index;
}

static uint32_t query_point_mask(float *segment_output, uint16_t h, uint16_t w)
{
    // h in [0, 239], w in [0, 319]
    static const uint16_t block_size = 16;
    static uint16_t left, block_h, block_w, in_block_h, in_block_w;
    static uint32_t transformed_depth, segment_index, left_mask, right_mask, mask;

    left = 2 * w;

    block_h = h / block_size;
    block_w = left / block_size;

    in_block_h = h % block_size;
    in_block_w = left % block_size;

    transformed_depth = in_block_h * block_size + in_block_w;
    segment_index = transformed_depth * 300 + block_h * 20 + block_w;

    left_mask = segment_output[segment_index] > 2.5 ? 0xffff0000 : 0x00000000;

    right_mask = segment_output[segment_index + 300] > 2.5 ? 0x0000ffff : 0x00000000;
    mask = left_mask | right_mask;

    return mask;
}

static void draw_segment(uint32_t *canvas, float *segment_output)
{
    // size of canvas: 240 * (320/2) = 38400
    // size of segment_output: 256 * 15 * 20 = 76800
    // block size is 16 * 16
    static const uint16_t block_size = 16;
    for (uint16_t h = 0; h < 240; ++h)
    {
        for (uint16_t w = 0; w < 160; ++w)
        {
            uint16_t left = 2 * w;
            //uint16_t right = left + 1;

            uint16_t block_h = h / block_size;
            uint16_t block_w = left / block_size;

            uint16_t in_block_h = h % block_size;
            uint16_t in_block_w = left % block_size;

            uint32_t transformed_depth = in_block_h * block_size + in_block_w;
            uint32_t segment_index = transformed_depth * 300 + block_h * 20 + block_w;

            uint32_t left_mask = segment_output[segment_index] > 2.5 ? 0xf0000000 : 0xffff0000;

            uint32_t right_mask = segment_output[segment_index + 300] > 2.5 ? 0x0000f000 : 0x0000ffff;
            uint32_t mask = left_mask | right_mask;

            canvas[h * 160 + w] &= mask;
        }
    }
}

static void draw_object_segment(uint32_t *canvas, tracker_t *tracker, float *segment_output)
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

        uint16_t color = colors[obj->id % 20];
        for (uint32_t h = y1; h < y2; ++h)
        {
            for (uint32_t w = x1; w < x2; w += 2)
            {
                uint32_t w_div_2 = w / 2;
                uint32_t index = transform_index(h, w_div_2);
                uint32_t left_mask = segment_output[index] > 2.5 ? color : 0;
                uint32_t right_mask = segment_output[index + 300] > 2.5 ? color : 0;
                uint32_t mask = (left_mask << 16) | right_mask;
                canvas[h * 160 + w_div_2] |= mask;
            }
        }
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

/* dino game*/
static int8_t is_dino_flag_touched(float *segment_output)
{
    // draw a myth box at x1, y1, x2, y2 -> cx cy
    lcd_draw_picture(10, 10, 50, 50, dino_pic);
    uint16_t cx = 35;
    uint16_t cy = 35;
    uint32_t index = transform_index(cy, cx / 2);
    return segment_output[index] > 2.5;
}

typedef struct dino_game_ctx
{

    int8_t player_life;
    uint16_t score;
    uint8_t velocity;

    uint8_t is_obstacle_valid;
    int16_t obstacle_x;
    uint8_t obstacle_y;

    uint8_t is_bird_valid;
    int16_t bird_x;
    uint8_t bird_y;

    uint8_t is_cloud_valid;
    int16_t cloud_x;
    uint8_t cloud_y;

    tracker_t *tracker;
    float *segment_output;

} dino_game_t;

#define GROUND_Y (230)
static void init_dino_game(dino_game_t *ctx, tracker_t *tracker)
{
    ctx->player_life = 100;

    ctx->velocity = 5;
    ctx->score = 0;

    ctx->is_obstacle_valid = 0;
    ctx->obstacle_x = 319;
    ctx->obstacle_y = GROUND_Y-45;

    ctx->is_bird_valid = 0;
    ctx->bird_x = 319;
    ctx->bird_y = 50;

    ctx->is_cloud_valid = 0;
    ctx->cloud_x = 319;
    ctx->cloud_y = 15;

    ctx->tracker = tracker;
}

static void _detection_handler(dino_game_t *ctx)
{
    g_ai_done_flag = 0;
    kpu_run_kmodel(&person_detect_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
    while (!g_ai_done_flag)
        ;

    static float *detect_output;
    static size_t detect_output_size;
    kpu_get_output(&person_detect_task, 0, (uint8_t **)&detect_output, &detect_output_size);

    static uint16_t n_box_limit = 10;
    static float *boxes[10];
    static uint16_t n_result;
    n_result = get_boxes(boxes, n_box_limit, detect_output, 15, 20, person_anchors, 9, 0.7, 0.15);
    update_tracker(ctx->tracker, boxes, n_result, HUMAN, DIST_THRESH);
}

static void _segment_handler(dino_game_t *ctx)
{
    g_ai_done_flag = 0;
    kpu_run_kmodel(&person_segment_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
    while (!g_ai_done_flag)
        ;

    static float *segment_output;
    static size_t segment_output_size;
    kpu_get_output(&person_segment_task, 0, (uint8_t **)&segment_output, &segment_output_size);
    ctx->segment_output = segment_output;
}

static uint8_t _collide(dino_game_t *ctx, int16_t x, int16_t y)
{
    uint32_t index = transform_index(y, x / 2);
    return ctx->segment_output[index] > SEGMENT_THRESH ? 1 : 0;
}

static void _collision_handler(dino_game_t *ctx)
{
    // check dead
    if (ctx->is_bird_valid)
    {
        int16_t x, y;
        x = ctx->bird_x + 10;
        y = ctx->bird_y + 5;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->bird_x + 30;
        y = ctx->bird_y + 5;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->bird_x + 10;
        y = ctx->bird_y + 15;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->bird_x + 30;
        y = ctx->bird_y + 15;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;
    }
    else if (ctx->is_obstacle_valid)
    {
        int16_t x, y;
        x = ctx->obstacle_x + 6;
        y = ctx->obstacle_y + 12;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->obstacle_x + 6;
        y = ctx->obstacle_y + 36;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->obstacle_x + 18;
        y = ctx->obstacle_y + 12;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;

        x = ctx->obstacle_x + 18;
        y = ctx->obstacle_y + 36;
        if(_collide(ctx, x, y))
            ctx->player_life = 0;
    }
    else;

}

static void state_handler(dino_game_t *ctx)
{
    _detection_handler(ctx);
    _segment_handler(ctx);

    ctx->score += ctx->velocity;
    // cloud
    if (!ctx->is_cloud_valid)
    {
        ctx->cloud_x = 319 - 50;
        ctx->cloud_y = rand() % 20 + 5;
        ctx->is_cloud_valid = 1;
    }
    else
    {
        ctx->cloud_x -= ctx->velocity / 2;
        if (ctx->cloud_x <= 0)
            ctx->is_cloud_valid = 0;
    }

    // bird or obstacle
    if (!ctx->is_bird_valid && !ctx->is_obstacle_valid)
    {
        ctx->velocity += 2;
        if (rand() % 2 == 0)
        {
            ctx->bird_x = 319 - 50;
            ctx->bird_y = rand() % 40 + 55;
            ctx->is_bird_valid = 1;
        }
        else
        {
            ctx->obstacle_x = 319 - 25;
            ctx->obstacle_y = rand() % 5 + 181;
            ctx->is_obstacle_valid = 1;
        }
    }
    else if (ctx->is_bird_valid)
    {
        ctx->bird_x -= ctx->velocity;
        if (ctx->bird_x <= 0)
        {
            ctx->is_bird_valid = 0;
        }
    }
    else if (ctx->is_obstacle_valid)
    {
        ctx->obstacle_x -= ctx->velocity;
        if (ctx->obstacle_x <= 0)
        {
            ctx->is_obstacle_valid = 0;
        }
    }
    else
        ;
    _collision_handler(ctx);
}

static void render(dino_game_t *ctx)
{
    // dvp
    g_dvp_finish_flag = 0;
    dvp_clear_interrupt(DVP_STS_FRAME_START | DVP_STS_FRAME_FINISH);
    dvp_config_interrupt(DVP_CFG_START_INT_ENABLE | DVP_CFG_FINISH_INT_ENABLE, 1);
    while (g_dvp_finish_flag == 0)
        ;

    uint32_t *canvas = display_image.addr;
    for (uint16_t h = 0; h < 240; ++h)
    {
        for (uint16_t w = 0; w < 160; ++w)
        {
            uint32_t index = transform_index(h, w);
            uint32_t left_mask = ctx->segment_output[index] > SEGMENT_THRESH ? 0x00000000 : 0xffff0000;
            uint32_t right_mask = ctx->segment_output[index + 300] > SEGMENT_THRESH ? 0x00000000 : 0x0000ffff;
            uint32_t mask = left_mask | right_mask;
            canvas[h * 160 + w] |= mask;
        }
    }
    // add ground line on canvas
    for (uint16_t w = 0; w < 160; ++w)
        canvas[GROUND_Y * 160 + w] &= 0;

    // draw bird
    if (ctx->is_bird_valid)
    {
        uint16_t y1 = ctx->bird_y;
        uint16_t y2 = y1 + 20;
        uint16_t x1 = ctx->bird_x / 2;
        uint16_t x2 = x1 + 20;
        for (uint16_t h = y1; h < y2; ++h)
        {
            for (uint16_t w = x1; w < x2; ++w)
            {
                canvas[h * 160 + w] &= bird_pic[(h - y1) * 20 + (w - x1)];
            }
        }
    }
    // draw obstacle
    if (ctx->is_obstacle_valid)
    {
        uint16_t y1 = ctx->obstacle_y;
        uint16_t y2 = y1 + 50;
        uint16_t x1 = ctx->obstacle_x / 2;
        uint16_t x2 = x1 + 12;
        for (uint16_t h = y1; h < y2; ++h)
        {
            for (uint16_t w = x1; w < x2; ++w)
            {
                canvas[h * 160 + w] &= obstacle_pic[(h - y1) * 12 + (w - x1)];
            }
        }
    }
    // draw cloud if valid
    if (ctx->is_cloud_valid)
    {
        uint16_t y1 = ctx->cloud_y;
        uint16_t y2 = y1 + 24;
        uint16_t x1 = ctx->cloud_x / 2;
        uint16_t x2 = x1 + 25;
        for (uint16_t h = y1; h < y2; ++h)
        {
            for (uint16_t w = x1; w < x2; ++w)
            {
                canvas[h * 160 + w] &= cloud_pic[(h - y1) * 25 + (w - x1)];
            }
        }
    }
    lcd_draw_picture(0, 0, 320, 240, canvas);
    static char buf[7];
    sprintf(buf, "%06d", ctx->score);
    lcd_draw_string(260, 10, buf, BLACK);
}

static void start_dino_game(tracker_t *tracker)
{
    dino_game_t ctx;
    init_dino_game(&ctx, tracker);
    while (ctx.player_life > 0)
    {
        state_handler(&ctx);
        render(&ctx);
    }
    lcd_draw_string(100, 10, "Game Over !!", RED);
    sleep(2);
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
    w25qxx_read_data(SEGMENT_MODEL_ADDR, segment_model_data, SEGMENT_MODEL_SIZE, W25QXX_QUAD_FAST);
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
    if (kpu_load_kmodel(&person_detect_task, person_model_data) != 0)
    {
        printf("\nperson model init error\n");
        while (1)
            ;
    }
    if (kpu_load_kmodel(&person_segment_task, segment_model_data) != 0)
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
    static uint16_t n_result;

    static float *detect_output;
    static size_t detect_output_size;

    static float *segment_output;
    static size_t segment_output_size;

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

        kpu_get_output(&person_detect_task, 0, (uint8_t **)&detect_output, &detect_output_size);

        n_result = get_boxes(boxes, n_box_limit, detect_output, 15, 20, person_anchors, N_PERSON_ANCHOR, 0.7, 0.15);
        update_tracker(&tracker, boxes, n_result, HUMAN, DIST_THRESH);

        // person segment
        g_ai_done_flag = 0;
        kpu_run_kmodel(&person_segment_task, kpu_image.addr, DMAC_CHANNEL5, ai_done, NULL);
        while (!g_ai_done_flag)
            ;

        kpu_get_output(&person_segment_task, 0, (uint8_t **)&segment_output, &segment_output_size);

        draw_object_segment((uint32_t *)display_image.addr, &tracker, segment_output);
        lcd_draw_picture(0, 0, 320, 240, (uint32_t *)display_image.addr);
        draw_object(&tracker);

        if (is_dino_flag_touched(segment_output))
            start_dino_game(&tracker);
    }
}
