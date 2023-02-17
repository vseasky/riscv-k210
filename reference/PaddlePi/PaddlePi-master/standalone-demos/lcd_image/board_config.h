#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_
// pin definitions
/* OV2640 */
#define		OV_RST_PIN					43
#define		OV_VSYNC_PIN				42
#define		OV_PWDN_PIN					45
#define		OV_HREF_PIN					44
#define		OV_PCLK_PIN					47
#define		OV_XCLK_PIN					46
#define		OV_SCCB_SCLK_PIN			41
#define		OV_SCCB_SDA_PIN				40	
/* LCD */
#define		LCD_CS_PIN					38
#define		LCD_DC_PIN					37
#define		LCD_RW_PIN					36
#define		LCD_RST_PIN					39
#define		LCD_BLIGHT_PIN				12
/* KEY */
#define		KEY_PIN						13
/* IR-CUT */
#define     IRCUTA_PIN                  6
#define     IRCUTB_PIN                  7

// IO definitions
#define     LCD_DC_IO                   2
#define     LCD_RST_IO                  0
#define		LCD_BLIGHT_IO				17

#endif
