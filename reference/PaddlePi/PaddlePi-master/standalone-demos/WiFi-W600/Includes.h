#ifndef		__INCLUDES_H
#define		__INCLUDES_H

/* spi > WiFi */
#define		SPI_DEVICE					SPI_DEVICE_1
#define		SPI_CS_SELECT				SPI_CHIP_SELECT_0
#define		SPI_DMA_TX_CHL				DMAC_CHANNEL3
#define		SPI_DMA_RX_CHL				DMAC_CHANNEL4


/* W600 */
#define 	W600_INTERRUPT_PIN   		19
#define 	W600_RESET_PIN       		23
#define		W600_SPI_CS_PIN				20
#define		W600_SPI_CLK_PIN			18
#define		W600_SPI_MOSI_PIN			21
#define		W600_SPI_MISO_PIN			22

/* IO */
#define 	W600_INTERRUPT_IO   		4
#define 	W600_RESET_IO        		3

#include    "entry.h"
#include 	"fpioa.h"
#include 	"sysctl.h"
#include 	"gpiohs.h"
#include 	"gpio.h"
#include 	"spi.h"
#include 	"uarths.h"
#include    "app.h"

#define		ASSERT(ex)				while(!(ex));


#endif
