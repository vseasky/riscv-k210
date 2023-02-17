/***********************************************************************
* Filename      : app.h
* Version       : V1.00
* Programmer(s) : kevin 
* Date			: 2015-02-02
***********************************************************************/
#ifndef  __APP_CFG_H__
#define  __APP_CFG_H__

#include "printf.h"

#define APP_DEBUG	0
#if APP_DEBUG
#define APP_PRINT printk
#else
#define APP_PRINT(fmt, ...)
#endif

#include <stdint.h>

typedef		uint8_t		INT8U	;
typedef		uint16_t	INT16U	;
typedef		uint32_t	INT32U	;


typedef struct _APP_MSG{
	INT32U type;
	INT32U param;
}APP_MSG;

typedef enum
{
	MSG_KEY = 0x01,
	MSG_LED,	
	MSG_WLAN,
	MSG_TIMER,
	MSG_SPI
}MSG_TYPE;

typedef struct _APP_SYS{
	INT32U WlanStatus;
	INT32U FdTcp;
	INT32U FdUdp;
	INT32U WlanMode;
}APP_SYS;


enum socket_protocol{
    SOCKET_PROTO_TCP,      		/* TCP Protocol */
    SOCKET_PROTO_UDP,     		/* UDP Protocol */
};

enum socket_cs_mode{
    SOCKET_CS_MODE_CLIENT,    	/* Client mode */
    SOCKET_CS_MODE_SERVER,    	/* Server mode */
};

typedef struct _WM_SOCKET_INFO{
	enum socket_protocol protocol;
	enum socket_cs_mode cs;
	INT8U hostname[32];
	INT16U remote;
	INT16U local;
}WM_SOCKET_INFO;

typedef struct _WM_SPI_RX_DESC{
    INT32U valid;
    INT8U buff[1500];
}WM_SPI_RX_DESC;


/***********************************************************************
* WLAN
***********************************************************************/
#define APP_WLAN_STA			0
#define APP_WLAN_AP				2

#define APP_ENCRY_OPEN			0
#define APP_ENCRY_WEP64			1
#define APP_ENCRY_WEP128		2
#define APP_ENCRY_WPA_TKIP		3
#define APP_ENCRY_WPA_CCMP		4
#define APP_ENCRY_WPA2_TKIP		5
#define APP_ENCRY_WPA2_CCMP		6

#define APP_KEY_HEX				0
#define APP_KEY_ASCII			1

#define APP_KEY_DHCP_ENABLE		0
#define APP_KEY_DHCP_DISABLE	1

/***********************************************************************
* WLAN ��ϢID
***********************************************************************/
#define APP_WLAN_UP				512
#define APP_WLAN_DOWN			513

/***********************************************************************
* TASK PRIORITIES
***********************************************************************/
#define APP_TASK_PRIO			3
#define LED_TASK_PRIO			4

/***********************************************************************
* TASK STACK SIZES
***********************************************************************/
#define APP_TASK_STK_SIZE				2048
#define APP_QUEUE_SIZE					32

#define LED_TASK_STK_SIZE				64
#define LED_QUEUE_SIZE					4

#define APP_TIMER_WSCAN_TIME			(100 * 3)	// 3s
#define APP_TIMER_WSCAN_ID				1001

#define APP_TIMER_WRECIVE_TIME			50			// 500ms
#define APP_TIMER_WRECIVE_ID			1002


#include <stdio.h>
#include <string.h>
#include "wmri.h"
#include "w600_interface.h"

typedef	void (*w600_rxdata_cb_t)(uint8_t *pdata, uint16_t len);

void AppSendMsg(INT32U Type, INT32U param);

void	w600_station_init(uint8_t *ssid, uint8_t *passwd, uint8_t *remote_ip, uint16_t port, uint8_t resetpin);
void	w600_station_operation(void);
uint32_t w600_station_txdata(uint8_t *data, uint16_t len);
void	w600_interrupt_operation_triger(void);
void	w600_interrupt_operation(void);
void	w600_register_rxdata_callback(w600_rxdata_cb_t callback);
uint32_t w600_socket_ready(void);
uint32_t w600_station_tx_alldata(uint8_t *data, uint32_t length);


#endif
