/* Copyright 2018 Canaan Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "board_config.h"

#if (BOARD_VERSION == BOARD_V1_3)

#include "dvp.h"
#include "fpioa.h"
#include "i2c.h"
#include "stdio.h"
#include <unistd.h>

#define	ONE_I2C_CONTROL		1

#define GC0328_ADDR                     0x42
static const uint8_t gc0328_config[][2] =
{
    {0xFE, 0x80},   // [7] soft reset; [1:0] page select 00:REGF 01:REGF1
    {0xFE, 0x80},
    {0xFC, 0x16},   // [4] digital clock enable; [2] da25_en; [1] da18_en
    {0xFC, 0x16},
    {0xFC, 0x16},
    {0xFC, 0x16},

    {0xFE, 0x00},   // [7] soft reset; [1:0] page select 00:REGF 01:REGF1
    {0x4F, 0x00},   // [0] AEC enable
    {0x42, 0x00},   // [7] auto saturation; [6] auto EE; [5] auto DN; [4] auto DD; [3] auto LSC; [2] ABS enable; [1] AWB enable; [0] auto Y offset
    {0x03, 0x00},   // Exposure time high: [3:0] exposure [11:8], use line processing time as the unit. controlled by AEC if AEC is in function
    {0x04, 0xC0},   // Exposure time low: Exposure[7:0], controlled by AEC if AEC is in function
    {0x77, 0x62},   // AWB_R_gain: 2.6bits, AWB red gain, controlled by AWB
    {0x78, 0x40},   // AWB_G_gain: 2.6bits, AWB green gain, controlled by AWB
    {0x79, 0x4D},   // AWB_B_gain: 2.6bits, AWB blue gain, controlled by AWB
        
    {0x05, 0x02},   // HB high: [3:0] HBLANK high bit[11:8]
    {0x06, 0x2C},   // HB low: HBLANK low bit[7:0]
    {0x07, 0x00},   // VB high: [3:0] VBLANK high bit[11:8]
    {0x08, 0xB8},   // VB low: VBLANK low bit[7:0]
    
    {0xFE , 0x01},   // page1: [7] soft reset; [1:0] page select 00:REGF 01:REGF1
    {0x29 , 0x00},   // AEC_anti_flicker_step[11:8]: Anti-flicker step[11:8]
    {0x2a , 0x60},   // AEC_anti_flicker_step[7:0]: Anti-flicker step[7:0]
        
    {0x2b , 0x02},   // AEC_exp_level_0
    {0x2c , 0xA0},            
    {0x2D , 0x03},   // AEC_exp_level_1
    {0x2E , 0x00},   
    {0x2F , 0x03},   // AEC_exp_level_2
    {0x30 , 0xC0},   
    {0x31 , 0x05},   // AEC_exp_level_3 
    {0x32 , 0x40},   
    {0xFE , 0x00},   // page 0
    
    {0xFE , 0x01},   // page 1
    {0x51 , 0x80},   // AWB_PRE_THD_min: Dominate luma THD
    {0x52 , 0x12},   
    {0x53 , 0x80},   // AWB_PRE_THD_min_MIX: mix luma number THD
    {0x54 , 0x60},   
    {0x55 , 0x01},   
    {0x56 , 0x06},   
    {0x5B , 0x02},   // mix base gain and adaptive gain limit
    {0x61 , 0xDC}, 
    {0x62 , 0xDC},  
    {0x7C , 0x71},   // adust_speed adjust_margin: [6:4] AWB gain adjust speed, the bigger the quicker; [3:0] if averages of R/G/B's difference is smaller than margin, it means AWB is OK, and AWB will stop
    {0x7D , 0x00},  
    {0x76 , 0x00},  
    {0x79 , 0x20},  
    {0x7B , 0x00},  // show_and_mode: [5] skin_mode; [1] dark_mode
    {0x70 , 0xFF},  
    {0x71 , 0x00},  
    {0x72 , 0x10},  
    {0x73 , 0x40},  
    {0x74 , 0x40},  
        
    {0x50 , 0x00},  
    {0xFE , 0x01},  // page 1
    {0x4F , 0x00},   
    {0x4C , 0x01},  
    {0x4F , 0x00},  
    {0x4F , 0x00},  
    {0x4F , 0x00},  
    {0x4D , 0x36},  
    {0x4E , 0x02}, 
    {0x4E , 0x02},  
    {0x4D , 0x44}, 
    {0x4E , 0x02},
    {0x4E , 0x02},
    {0x4E , 0x02},
    {0x4E , 0x02},
    {0x4D , 0x53},  
    {0x4E , 0x08},  
    {0x4E , 0x08},  
    {0x4E , 0x02}, 
    {0x4D , 0x63},
    {0x4E , 0x08},
    {0x4E , 0x08},
    {0xFE , 0x00},  // page 0
    {0x4D , 0x73},  // auto_middle_gamma_en: [0] auto middle gamma enable
    {0x4E , 0x20},  
    {0x4D , 0x83},
    {0x4E , 0x20},
    {0x4F , 0x01},  // AEC enable: [0] AEC enable

    {0x50 , 0x88},  // Crop_win_mode: [0] crop window mode enable
    {0xFE , 0x00},  // page 0
        
    {0x27 , 0x00},  
    {0x2A , 0x40},
    {0x2B , 0x40},
    {0x2C , 0x40},
    {0x2D , 0x40},

    {0xFE , 0x00},  // page 0
    {0x0D, 0x01}, // window height high: [0] Window height high[8]   -- height 488
    {0x0E, 0xE8}, // Window height low: Window height low[7:0]
    {0x0F, 0x02}, // window width high: [1:0] Window width high[9:8]   -- width 648
    {0x10, 0x88}, // window width low: Window width low[7:0]
    {0x09 , 0x00},  // Row start high: [0] row start high bit[8]
    {0x0A , 0x00},  // Row start low:row start low bit[7:0]
    {0x0B , 0x00},  // Col start high: [1:0] col start high bit[9:8]
    {0x0C , 0x00},  // Col start low: col start low bit[7:0]
    {0x16 , 0x00},  // Analog gain: [7] Analog gain enable
	{0x17 , 0x16},
    {0x18 , 0x0E},  // CISCTL_mode2: [7:6] output mode-VGA mode; [5] column binning; [4] double reset mode; [3:2] sdark mode -- sdark 4 rows in each frame; [1] new exposure/normal bad frame; [0] badframe enable
    {0x19 , 0x06},  // CISCTL_mode3: [6] for double restg; [5] restg on/off; [4] capture AD data edge; [3:0] AD pipe number

    {0x1B , 0x48},  // Rsh width: [7:4] restg_width, X2; [3:0] sh_width, X2
    {0x1F , 0xC8},  
    {0x20 , 0x01},
    {0x21 , 0x78},
    {0x22 , 0xB0},
    {0x23 , 0x04},
    {0x24 , 0x11},
    {0x26 , 0x00},

    {0x50 , 0x01},  // Crop_win_mode: [0] crop window mode enable

    {0x59 , 0x22},  // subsample: [7:4] subsample row ratio; [3:0] subsample col ratio
    {0x51 , 0x00},  // Crop_win_y1
    {0x52 , 0x00},
    {0x53 , 0x00},  // Crop_win_x1
    {0x54 , 0x00},
    {0x55 , 0x00},  // Crop_win_height
    {0x56 , 0xF0},  
    {0x57 , 0x01},  // Crop_win_width
    {0x58 , 0x40},

    {0x70 , 0x85},  // Global gain

    {0x40 , 0x7F},  // Block_enable_1: [7] middle gamma enable; [6] gamma enable; [5] CC enable; [4] Edge enhancement enable, [3] Interpolation enable; [2] Noise removal enable; [1] Defect removal enable; [0] Lens-shading correction enable
    {0x41 , 0x26},  // Block_enable_2: [6] low light Y enable; [5] skin enable; [4] skin Y enable; [3] new skin mode; [2] autogray enable; [1] Y gamma enable; [0] block skin
    {0x42 , 0xFF},  // [7] auto saturation; [6] auto EE; [5] auto DN; [4] auto DD; [3] auto LSC; [2] ABS enable; [1] AWB enable; [0] auto Y offset
    {0x45 , 0x00},  // Auto middle gamma mode: [1] auto gamma mode outdoor; [0] auto gamma mode lowlight
    {0x44 , 0x06},  // Output_format: RGB565
    {0x46 , 0x02},  // SYNC_mode: [1] HSYNC polarity; [0] VSYNC polarity

    {0x4B , 0x01},  // Debug mode 1: [0] update gain mode
    {0x50 , 0x01},  // Crop_win_mode: [0] crop window mode enable

    {0x7E , 0x0A},   
    {0x7F , 0x03},
    {0x80 , 0x27},
    {0x81 , 0x15},
    {0x82 , 0x90},
    {0x83 , 0x02},
    {0x84 , 0x23},
    {0x90 , 0x2C},
    {0x92 , 0x02},
    {0x94 , 0x02},
    {0x95 , 0x35},

    {0xD1 , 0x32},  // Cb saturation
    {0xD2 , 0x32},  // Cr saturation
    {0xDD , 0x18},  
    {0xDE , 0x32},
    {0xE4 , 0x88},
    {0xE5 , 0x40},
    {0xD7 , 0x0E},

    {0xFE , 0x00},  // page 0
    {0xBF , 0x10},
    {0xC0 , 0x1C},
    {0xC1 , 0x33},
    {0xC2 , 0x48},
    {0xC3 , 0x5A},
    {0xC4 , 0x6B},
    {0xC5 , 0x7B},
    {0xC6 , 0x95},
    {0xC7 , 0xAB},
    {0xC8 , 0xBF},
    {0xC9 , 0xCD},
    {0xCA , 0xD9},
    {0xCB , 0xE3},
    {0xCC , 0xEB},
    {0xCD , 0xF7},
    {0xCE , 0xFD},
    {0xCF , 0xFF},

    {0xFE , 0x00},  // page 0
    {0x63 , 0x00},
    {0x64 , 0x05},
    {0x65 , 0x0C},
    {0x66 , 0x1A},
    {0x67 , 0x29},
    {0x68 , 0x39},
    {0x69 , 0x4B},
    {0x6A , 0x5E},
    {0x6B , 0x82},
    {0x6C , 0xA4},
    {0x6D , 0xC5},
    {0x6E , 0xE5},
    {0x6F , 0xFF},

    {0xFE , 0x01},  // page 1
    {0x18 , 0x02},
    {0xFE , 0x00},  // page 0
    {0x98 , 0x00},
    {0x9B , 0x20},
    {0x9C , 0x80},
    {0xA4 , 0x10},
    {0xA8 , 0xB0},
    {0xAA , 0x40},
    {0xA2 , 0x23},
    {0xAD , 0x01},

    {0xFE , 0x01},  // page 1
    {0x9C , 0x02},
    {0x08 , 0xA0},
    {0x09 , 0xE8},

    {0x10 , 0x00},
    {0x11 , 0x11},
    {0x12 , 0x10},
    {0x13 , 0x80},
    {0x15 , 0xFC},
    {0x18 , 0x03},
    {0x21 , 0xC0},
    {0x22 , 0x60},
    {0x23 , 0x30},
    {0x25 , 0x00},
    {0x24 , 0x14},

    {0xFE , 0x01},  // page 1
    {0xC0 , 0x10},
    {0xC1 , 0x0C},
    {0xC2 , 0x0A},
    {0xC6 , 0x0E},
    {0xC7 , 0x0B},
    {0xC8 , 0x0A},
    {0xBA , 0x26},
    {0xBB , 0x1C},
    {0xBC , 0x1D},
    {0xB4 , 0x23},
    {0xB5 , 0x1C},
    {0xB6 , 0x1A},
    {0xC3 , 0x00},
    {0xC4 , 0x00},
    {0xC5 , 0x00},
    {0xC9 , 0x00},
    {0xCA , 0x00},
    {0xCB , 0x00},
    {0xBD , 0x00},
    {0xBE , 0x00},
    {0xBF , 0x00},
    {0xB7 , 0x07},
    {0xB8 , 0x05},
    {0xB9 , 0x05},
    {0xA8 , 0x07},
    {0xA9 , 0x06},
    {0xAA , 0x00},
    {0xAB , 0x04},
    {0xAC , 0x00},
    {0xAD , 0x02},
    {0xAE , 0x0D},
    {0xAF , 0x05},
    {0xB0 , 0x00},
    {0xB1 , 0x07},
    {0xB2 , 0x03},
    {0xB3 , 0x00},
    {0xA4 , 0x00},
    {0xA5 , 0x00},
    {0xA6 , 0x00},
    {0xA7 , 0x00},
    {0xA1 , 0x3C},
    {0xA2 , 0x50},
    {0xFE , 0x00},  // page 0

    {0xB1 , 0x04},  
    {0xB2 , 0xFD},
    {0xB3 , 0xFC},
    {0xB4 , 0xF0},
    {0xB5 , 0x05},
    {0xB6 , 0xF0},

    // msleep(200);
    {0xFE , 0x00},  // page 0
    {0x27 , 0xF7},
    {0x28 , 0x7F},
    {0x29 , 0x20},
    {0x33 , 0x20},
    {0x34 , 0x20},
    {0x35 , 0x20},
    {0x36 , 0x20},
    {0x32 , 0x08},

    {0x47 , 0x00},
    {0x48 , 0x00},

    {0xFE , 0x01},  // page 1
    {0x79 , 0x00},  
    {0x7D , 0x00},
    {0x50 , 0x88},  // AWB_PRE_mode: [7] PRE_enable; [3] AWB_PRE_adjust_speed enable
    {0x5B , 0x0C},  
    {0x76 , 0x8F},
    {0x80 , 0x70},
    {0x81 , 0x70},
    {0x82 , 0xB0},
    {0x70 , 0xFF},
    {0x71 , 0x00},
    {0x72 , 0x28},
    {0x73 , 0x0B},
    {0x74 , 0x0B},

    {0xFE , 0x00},  // page 0
    {0x70 , 0x45},  // global gain
    {0x4F , 0x01},  // AEC enable
//    {0xF1 , 0x07},
    {0xF1 , 0x00},  // Pad_setting1: [2] pclk output enable; [1] HSYNC output enable; [0] VSYNC output enable

//    {0xF2 , 0x01},
    {0xF2 , 0x00},  // Pad_setting2: [0] data output enable
    
    {0 , 0}
};

void i2c_master_init(int num)
{
#if	ONE_I2C_CONTROL
	if (num)
	{
		fpioa_set_function(DVP_SCCB_SDA_PIN_2, FUNC_RESV0);
		fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_I2C0_SDA);
		fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_I2C0_SCLK);
	}
	else
	{
		fpioa_set_function(DVP_SCCB_SDA_PIN, FUNC_RESV0);
		fpioa_set_function(DVP_SCCB_SDA_PIN_2, FUNC_I2C0_SDA);
		fpioa_set_function(DVP_SCCB_SCLK_PIN, FUNC_I2C0_SCLK);
	}
	i2c_init(0, GC0328_ADDR >> 1, 7, 100000);
	
#else
    if (num)
    {
        fpioa_set_function(13, FUNC_I2C1_SDA);
        fpioa_set_function(10, FUNC_I2C1_SCLK);
    }
    else
    {
        fpioa_set_function(9, FUNC_I2C0_SDA);
        fpioa_set_function(10, FUNC_I2C0_SCLK);
    }
    i2c_init(num, GC0328_ADDR >> 1, 7, 100000);
#endif
}

static void gc0328_wr_reg(uint8_t num, uint8_t reg,uint8_t data)
{
    uint8_t buf[2];

    buf[0] = reg & 0xff;
    buf[1] = data;
    i2c_send_data(num, buf, 2);
}

static uint8_t gc0328_rd_reg(uint8_t num, uint8_t reg)
{
    uint8_t reg_buf[1];
    uint8_t data_buf;

    reg_buf[0] = reg & 0xff;
    i2c_recv_data(num, reg_buf, 1, &data_buf, 1);
    return data_buf;
}

void open_gc0328_0()
{
#if	ONE_I2C_CONTROL
    usleep(1 * 1000);
    i2c_master_init(1);
    usleep(1 * 1000);
    gc0328_wr_reg(0, 0xFE, 0x00);
    gc0328_wr_reg(0, 0xF1, 0x00);
    gc0328_wr_reg(0, 0xF2, 0x00);
    
    usleep(1 * 1000);
    i2c_master_init(0);
    usleep(1 * 1000);
    gc0328_wr_reg(0, 0xFE, 0x00);
    gc0328_wr_reg(0, 0xF1, 0x07);
    gc0328_wr_reg(0, 0xF2, 0x01);
#else
    usleep(1 * 1000);
    i2c_master_init(1);
    usleep(1 * 1000);
    gc0328_wr_reg(1, 0xFE, 0x00);
    gc0328_wr_reg(1, 0xF1, 0x00);
    gc0328_wr_reg(1, 0xF2, 0x00);
    
    usleep(1 * 1000);
    i2c_master_init(0);
    usleep(1 * 1000);
    gc0328_wr_reg(0, 0xFE, 0x00);
    gc0328_wr_reg(0, 0xF1, 0x07);
    gc0328_wr_reg(0, 0xF2, 0x01);
#endif
}

void open_gc0328_1()
{
#if	ONE_I2C_CONTROL
	//usleep(1 * 1000);
	i2c_master_init(0);
	//usleep(1 * 1000);
	gc0328_wr_reg(0, 0xFE, 0x00);
	gc0328_wr_reg(0, 0xF1, 0x00);
	gc0328_wr_reg(0, 0xF2, 0x00);
	usleep(1 * 1000);
	i2c_master_init(1);
	//usleep(1 * 1000);
	gc0328_wr_reg(0, 0xFE, 0x00);
	gc0328_wr_reg(0, 0xF1, 0x07);
	gc0328_wr_reg(0, 0xF2, 0x01);

#else
    usleep(1 * 1000);
    i2c_master_init(0);
    usleep(1 * 1000);
    gc0328_wr_reg(0, 0xFE, 0x00);
    gc0328_wr_reg(0, 0xF1, 0x00);
    gc0328_wr_reg(0, 0xF2, 0x00);
    usleep(1 * 1000);
    i2c_master_init(1);
    usleep(1 * 1000);
    gc0328_wr_reg(1, 0xFE, 0x00);
    gc0328_wr_reg(1, 0xF1, 0x07);
    gc0328_wr_reg(1, 0xF2, 0x01);
#endif	
}

int gc0328_read_id(uint8_t num, uint8_t *id)
{
    *id = gc0328_rd_reg(num, 0xf0);
    return 0;
}

int gc0328_init(void)
{
    uint8_t data;
    uint32_t i;
    uint8_t num = 0;
    i2c_master_init(num);
    gc0328_read_id(num, &data);
    printf("gc0328 0 ID : 0x%x\n", data);

    for (i = 0; gc0328_config[i][0]; i++)
    {
        gc0328_wr_reg(num, gc0328_config[i][0], gc0328_config[i][1]);
    }
    usleep(1 * 1000);
#if	ONE_I2C_CONTROL	
    num = 0;
	i2c_master_init(1);
	gc0328_read_id(num, &data);
#else
    num = 1;
    i2c_master_init(num);
    gc0328_read_id(num, &data);
#endif	
    printf("gc0328 1 ID : 0x%x\n", data);
    for (i = 0; gc0328_config[i][0]; i++)
    {
        gc0328_wr_reg(num, gc0328_config[i][0], gc0328_config[i][1]);
    }
    return 0;
}

uint8_t gc0328_read_luma(void)
{
	uint8_t luma;
	
	gc0328_wr_reg(0, 0xfe, 0x1);
	luma = gc0328_rd_reg(0, 0x14);

	return luma;
}

uint16_t gc0328_exposure_time(void)
{
	uint8_t exp11_8;
	uint8_t exp7_0;
	uint16_t exp = 0;

	gc0328_wr_reg(0, 0xfe, 0x0);
	
	exp11_8 = gc0328_rd_reg(0, 0x03);
	exp7_0  = gc0328_rd_reg(0, 0x04);

	exp = (exp11_8<<8) | exp7_0;
	return exp;
}

#endif // (BOARD_VERSION == BOARD_V1_3)