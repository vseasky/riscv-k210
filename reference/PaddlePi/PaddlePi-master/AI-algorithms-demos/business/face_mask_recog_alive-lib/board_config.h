#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#define BOARD_V1_2_LE 0
#define BOARD_V1_3 1
#define BOARD_KD233 2

// choose your board version
#define BOARD_VERSION BOARD_V1_3

#define OV5640 0
#define OV2640 0
#define GC0328 1

#if OV5640 + OV2640 + GC0328 != 1
#error ov sensor only choose one
#endif

#if(GC0328 == 1) || (OV2640 == 1)
#define CAMERA_REG_LENGTH (8)
#define CAMERA_XCLK_RATE (24000000)
#elif(OV5640 == 1)
#define CAMERA_REG_LENGTH (16)
#define CAMERA_XCLK_RATE (12000000)
#endif

#if(BOARD_VERSION == BOARD_V1_2_LE)
/* OV2640 */
#define DVP_RST_PIN 43
#define DVP_VSYNC_PIN 42
#define DVP_PWDN_PIN 45
#define DVP_HREF_PIN 44
#define DVP_PCLK_PIN 47
#define DVP_XCLK_PIN 46
#define DVP_SCCB_SCLK_PIN 41
#define DVP_SCCB_SDA_PIN 40
#define DVP_SCCB_SDA_PIN_2 -1

/* LCD */
#define LCD_CS_PIN 38
#define LCD_DC_PIN 37
#define LCD_RW_PIN 36
#define LCD_RST_PIN 39
#define LCD_BLIGHT_PIN 12
/* KEY */
#define KEY_PIN 13

#define LCD_XYZ DIR_YX_RLDU

#elif(BOARD_VERSION == BOARD_V1_3)

#define DVP_VSYNC_PIN 42
#define DVP_PWDN_PIN 43 /* pin40=RESET on sch, but actual it's PWDN.*/
#define DVP_HREF_PIN 44
#define DVP_PCLK_PIN 47
#define DVP_XCLK_PIN 46
#define DVP_SCCB_SCLK_PIN 41
#define DVP_SCCB_SDA_PIN 40
#define DVP_SCCB_SDA_PIN_2 45

/* LCD */
#define LCD_CS_PIN 38
#define LCD_DC_PIN 37
#define LCD_RW_PIN 36
#define LCD_RST_PIN 39
#define LCD_BLIGHT_PIN 12
/* KEY */
#define KEY_PIN 13
/* LED */
#define LED_IR_PIN 6

#define LCD_XYZ DIR_YX_RLDU

#elif(BOARD_VERSION == BOARD_KD233)

#define DVP_RST_PIN 11
#define DVP_VSYNC_PIN 12
#define DVP_PWDN_PIN 13
#define DVP_HREF_PIN 17
#define DVP_PCLK_PIN 15
#define DVP_XCLK_PIN 14
#define DVP_SCCB_SCLK_PIN 10
#define DVP_SCCB_SDA_PIN 9
#define DVP_SCCB_SDA_PIN_2 -1
/* LCD */
#define LCD_CS_PIN 6
#define LCD_DC_PIN 8
#define LCD_RW_PIN 7
#define LCD_RST_PIN -1
#define LCD_BLIGHT_PIN -1
/* KEY */
#define KEY_PIN 26

#define LCD_XYZ (DIR_YX_RLUD | 0x8)
#endif

/* IO map. */
#define LED_IR_IO 1
#define LCD_DC_IO 2
#define LCD_RST_IO 3
#define KEY_IO 4
#define LCD_BLIGHT_IO 5

#endif
