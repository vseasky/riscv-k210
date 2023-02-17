#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#include <stdio.h>

#define 	BOARD_V1_2_LE				0
#define 	BOARD_V1_3					1
// choose your board version
#define 	BOARD_VERSION				BOARD_V1_3

// pin definitions
/* CAMERA */
#if (BOARD_VERSION == BOARD_V1_2_LE)
#define		DVP_PWDN_PIN				45
#define		DVP_RST_PIN					43
#elif (BOARD_VERSION == BOARD_V1_3)
#define		DVP_SCCB_SDA_PIN_2			45
#define		DVP_PWDN_PIN				43
#endif
#define		DVP_VSYNC_PIN				42
#define		DVP_HREF_PIN				44
#define		DVP_PCLK_PIN				47
#define		DVP_XCLK_PIN				46
#define		DVP_SCCB_SCLK_PIN			41
#define		DVP_SCCB_SDA_PIN			40	
/* LCD */
#define		LCD_CS_PIN					38
#define		LCD_DC_PIN					37
#define		LCD_RW_PIN					36
#define		LCD_RST_PIN					39
#define		LCD_BLIGHT_PIN				12

#if (BOARD_VERSION == BOARD_V1_3)
/* KEY */
#define		KEY_PIN						13
/* IR LED */
#define 	LED_IR_PIN					6
#endif

// IO definitions
#define     LCD_DC_IO                   2
#define     LCD_RST_IO                  0
#define		LCD_BLIGHT_IO				17
#if (BOARD_VERSION == BOARD_V1_3)
#define 	KEY_IO						8
#define		LED_IR_IO					1

// Tick
#define		TCIK_MS						10
#define		TICK_NANOSECONDS			(TCIK_MS*1000000)
#endif

#endif
