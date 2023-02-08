#ifndef _BOARD_CONFIG_
#define _BOARD_CONFIG_

#define  OV5640             0
#define  OV2640             1

#define  BOARD_KD233        0
#define  BOARD_LICHEEDAN    1


#define LCD_INVERSION_ON 1

#define DVP_VER_FLIP 1 // 垂直翻转
#define DVP_HOR_MIRR 0 // 水平镜像

#define LCD_VER_FLIP 0 // 垂直翻转
#define LCD_HOR_MIRR 0 // 水平镜像

#if OV5640 + OV2640 != 1
#error ov sensor only choose one
#endif

#if BOARD_KD233 + BOARD_LICHEEDAN != 1
#error board only choose one
#endif

#endif
