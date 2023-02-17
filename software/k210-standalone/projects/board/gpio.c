#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include "dvp.h"
#include "fpioa.h"
#include "board_config.h"
#include "nt35310.h"
#include "sysctl.h"

#if BOARD_LICHEEDAN
void io_mux_init(void)
{
    /*初始化DVP输入输出映射和功能设置*/
    fpioa_set_function(40, FUNC_SCCB_SDA);   // DVP_SDA
    fpioa_set_function(41, FUNC_SCCB_SCLK);  // DVP_SCL
    fpioa_set_function(42, FUNC_CMOS_RST);   // DVP_RST
    fpioa_set_function(43, FUNC_CMOS_VSYNC); // DVP_VSYNC
    fpioa_set_function(44, FUNC_CMOS_PWDN);  // DVP_PWDN
    fpioa_set_function(45, FUNC_CMOS_HREF);  // DVP_HREF
    fpioa_set_function(46, FUNC_CMOS_XCLK);  // DVP_XCLK
    fpioa_set_function(47, FUNC_CMOS_PCLK);  // DVP_PCLK

    /*初始化串行接口输入输出映射和功能设置*/
    fpioa_set_function(36, FUNC_SPI0_SS3);              // LCD_CS
    fpioa_set_function(37, FUNC_GPIOHS0 + RST_GPIONUM); // LCD_RST
    fpioa_set_function(38, FUNC_GPIOHS0 + DCX_GPIONUM); // LCD_DC
    fpioa_set_function(39, FUNC_SPI0_SCLK);             // LCD_WR

    sysctl_set_spi0_dvp_data(1);
}

void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK6, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK7, SYSCTL_POWER_V18);
}
#else

void io_mux_init(void)
{
    /* Init DVP IO map and function settings */
    fpioa_set_function(11, FUNC_CMOS_RST);
    fpioa_set_function(13, FUNC_CMOS_PWDN);
    fpioa_set_function(14, FUNC_CMOS_XCLK);
    fpioa_set_function(12, FUNC_CMOS_VSYNC);
    fpioa_set_function(17, FUNC_CMOS_HREF);
    fpioa_set_function(15, FUNC_CMOS_PCLK);
    fpioa_set_function(10, FUNC_SCCB_SCLK);
    fpioa_set_function(9, FUNC_SCCB_SDA);

    /* Init SPI IO map and function settings */
    fpioa_set_function(8, FUNC_GPIOHS0 + DCX_GPIONUM);
    fpioa_set_function(6, FUNC_SPI0_SS3);
    fpioa_set_function(7, FUNC_SPI0_SCLK);

    sysctl_set_spi0_dvp_data(1);
}

void io_set_power(void)
{
    /* Set dvp and spi pin to 1.8V */
    sysctl_set_power_mode(SYSCTL_POWER_BANK1, SYSCTL_POWER_V18);
    sysctl_set_power_mode(SYSCTL_POWER_BANK2, SYSCTL_POWER_V18);
}
#endif