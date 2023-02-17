/***********************************************************************
* Filename      : wmri.h
* Version       : V1.00
* Programmer(s) : kevin 
* Date			: 2015-02-10
***********************************************************************/
#ifndef  __WM_RI_H__
#define  __WM_RI_H__


#define RI_DEBUG	0
#if RI_DEBUG
#define RI_PRINT printk
#else
#define RI_PRINT(fmt, ...)
#endif

/***************************************************************
 * High speed/HSPI DATA/CMD/EVENT/RSP Format definition
 ***************************************************************/
#define RI_CMD_NOP				0
#define RI_CMD_RESET			1
#define RI_CMD_PS				2
#define RI_CMD_RESET_FLASH		3
#define RI_CMD_PMTF				4
#define RI_CMD_GPIO				5
#define RI_CMD_MAC				6
#define RI_CMD_VER				7
#define RI_CMD_WJOIN			0x20
#define RI_CMD_WLEAVE			0x21
#define RI_CMD_WSCAN			0x22
#define RI_CMD_LINK_STATUS		0x23
#define RI_CMD_WPSST			0x24
#define RI_CMD_SKCT				0x28
#define RI_CMD_SKSTT			0x29
#define RI_CMD_SKCLOSE			0x2A
#define RI_CMD_SKSDF			0x2B
#define RI_CMD_ONESHOT				0x2C
#define RI_CMD_HTTPC				0x2D
#define RI_CMD_WPRT					0x40
#define RI_CMD_SSID					0x41
#define RI_CMD_KEY					0x42
#define RI_CMD_ENCRYPT				0x43
#define RI_CMD_BSSID				0x44
#define RI_CMD_BRD_SSID				0x45
#define RI_CMD_CHNL					0x46
#define RI_CMD_WREG					0x47
#define RI_CMD_WBGR					0x48
#define RI_CMD_WATC					0x49
#define RI_CMD_WPSM					0x4A
#define RI_CMD_WARM					0x4B
#define RI_CMD_WPS					0x4C
#define RI_CMD_CUSTDATA				0x59
#define RI_CMD_NIP					0x60
#define RI_CMD_ATM					0x61
#define RI_CMD_ATRM					0x62
#define RI_CMD_AOLM					0x63
#define RI_CMD_PORTM				0x64
#define RI_CMD_UART					0x65
#define RI_CMD_ATLT					0x66
#define RI_CMD_DNS					0x67
#define RI_CMD_DDNS					0x68
#define RI_CMD_UPNP					0x69
#define RI_CMD_DNAME				0x6A
#define RI_CMD_PASS				    0x6B
#define RI_CMD_DBG					0xF0
#define RI_CMD_REGR					0xF1
#define RI_CMD_REGW					0xF2
#define RI_CMD_RFR					0xF3
#define RI_CMD_RFW					0xF4
#define RI_CMD_FLSR					0xF5
#define RI_CMD_FLSW					0xF6
#define RI_CMD_UPDM					0xF7
#define RI_CMD_UPDD 				0xF8

#define RI_EVENT_INIT_END			0xE0
#define RI_EVENT_CRC_ERR			0xE1
#define RI_EVENT_SCAN_RES			0xE2
#define RI_EVENT_JOIN_RES			0xE3
#define RI_EVENT_STA_JOIN			0xE4
#define RI_EVENT_STA_LEAVE			0xE5
#define RI_EVENT_LINKUP				0xE6
#define RI_EVENT_LINKDOWN			0xE7
#define RI_EVENT_TCP_CONN			0xE8
#define RI_EVENT_TCP_JOIN			0xE9
#define RI_EVENT_TCP_DIS			0xEA
#define RI_EVENT_TX_ERR				0xEB 

#define RI_CMD_SYNC					0xAA
#define RI_DATA						0x00
#define RI_CMD						0x01
#define RI_CMD_MT_EVENT				0x00
#define RI_CMD_MT_REQ				0x01
#define RI_CMD_MT_RSP				0x02

#define	RI_CMD_TEST_CHANNEL			0xB2
#define	RI_CMD_TEST_STARTUP			0xB3
#define	RI_CMD_TEST_STOP			0xB4

typedef struct _WM_RI_HDR 
{
	INT8U sync;
	INT8U type;
	INT16U length;
	INT8U seq_num;
	INT8U flag;
	INT8U dest_addr;
	INT8U chk; 
}__attribute__((packed)) WM_RI_HDR;

typedef struct _WM_RICMD_HDR 
{
	INT8U msg_type;
	INT8U code;
	INT8U err;
	INT8U ext;
}__attribute__((packed)) WM_RI_CMD_HDR;

typedef struct  _WM_RICMD_EXT_HDR 
{
	INT32U remote_ip;
	INT16U remote_port;
	INT16U local_port;
}__attribute__((packed)) WM_RICMD_EXT_HDR;

typedef struct _WM_SPI_HDR 
{
	WM_RI_HDR hdr;
	WM_RI_CMD_HDR cmd;
}__attribute__((packed)) WM_SPI_HDR;


//#define Swap16(v)	(((v & 0xff) << 8) | (v >> 8))
#define Swap16(v)	((((v) & 0xff) << 8) | ((v) >> 8))


INT32U RISaveParam(void);
INT32U RIResetDevice(void);
INT32U RISetOneShotCfg(INT8U flag);
INT32U RISetWlanType(INT32U type);
INT32U RISetSSID(INT8U *ssid);
INT32U RISetKey(INT8U type, INT8U index, INT8U *key);
INT32U RISetEncrypt(INT32U type);
INT32U RISetNip(INT8U dhcp, INT8U *ip, INT8U *netmast, INT8U *gateway, INT8U *dns);
INT32U RIWlanJoin(void);
INT32U RIGetWlanStatus(void);
INT32U RISocketCreate(WM_SOCKET_INFO *sk);
INT32U RISocketClose(INT32U fd);
INT32U RIGetSocketReceiveLen(INT32U fd);
INT32U RISocketTCPSend(INT32U fd, INT8U *data, INT16U len);
INT32U RI_txtest_set_channel(INT8U channel, INT8U bandwith);
INT32U RI_txtest_startup(INT32U packetlen, INT32U gain, INT32U rate);

#endif
