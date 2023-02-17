/***********************************************************************
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : kevin 
* Date			: 2015-02-02
***********************************************************************/

#include	"app.h"
#include 	<stdlib.h>
#include	"gpiohs.h"
#include	"gpio.h"
#include 	"sleep.h"
#include	"Includes.h"
#include 	"atomic.h"

typedef	struct {
	INT8U   *ssid;
	INT8U   *passwd;
	INT8U	*remote_ip;
	INT16U	remote_port;
	INT8U	restpin;
}__attribute__((aligned(4))) wlan_op_t;

wlan_op_t wlan_op;

static APP_SYS gsSTSysCtrol;
static void AppWlanProc(INT32U param);
static void AppSPIProc(INT32U param);
static INT32U AppRICheckSPIHeader(WM_RI_HDR *ri);
static INT32U AppRICmdProc(WM_RI_CMD_HDR *cmd);
static INT32U AppCreatSocketProc(uint16_t remote_port);
static uint32_t AppSocketSendProc(INT32U fd, INT8U *data, INT16U len);
static void AppCloseSocketProc(INT32U fd);
static w600_rxdata_cb_t rxdata_operation = (w600_rxdata_cb_t)0;

#define	FIFO_LENGTH		64
typedef	struct	fifo {
	INT32U	read;
	INT32U 	write;
	APP_MSG fifo[FIFO_LENGTH];
}__attribute__((align(4)))	fifo_t;
fifo_t	msgfifo;

uint32_t	w600_interrupt_triger = 0;

/*Init message memory. */
static	void	msg_fifo_init(void)
{
	memset(&msgfifo, 0 , sizeof(msgfifo));
}

/*write message */
static	void	msg_fifo_write(INT32U type, INT32U param)
{
	msgfifo.fifo[msgfifo.write].type = type;
	msgfifo.fifo[msgfifo.write].param = param;
	msgfifo.write++;
	if(msgfifo.write == FIFO_LENGTH) msgfifo.write = 0;	
}

/*read message */
static	APP_MSG * msg_fifo_read(void)
{
	APP_MSG *msg = (APP_MSG *) 0;
	if(msgfifo.read == msgfifo.write)  return (APP_MSG *) 0;
	
	msg = (APP_MSG *)&(msgfifo.fifo[msgfifo.read++]);
	if(msgfifo.read == FIFO_LENGTH) msgfifo.read = 0;

	return	msg;
}

/*write message API */
void AppSendMsg(INT32U Type, INT32U param)
{
	msg_fifo_write(Type, param);	
}

/* W600 Init: station mode*/
void	w600_station_init(uint8_t *ssid, uint8_t *passwd, uint8_t *remote_ip, uint16_t port, uint8_t resetpin)
{	
	gsSTSysCtrol.WlanStatus = 0;
	gsSTSysCtrol.WlanMode = 0;			/*- 0: station. 1: ap. -*/		
	w600_interrupt_triger = 0;
	msg_fifo_init();

	ASSERT(ssid);
	ASSERT(passwd);
	ASSERT(remote_ip);

	wlan_op.ssid = ssid;
	wlan_op.passwd = passwd;
	wlan_op.remote_ip = remote_ip;
	wlan_op.remote_port = port;
	wlan_op.restpin = resetpin;						
	/*reset w600.*/
	gpio_set_pin(wlan_op.restpin, GPIO_PV_LOW);
	for(uint32_t volatile i=0; i<0x1000; i++);
	gpio_set_pin(wlan_op.restpin, GPIO_PV_HIGH);	
	APP_PRINT("reset w600\n");
}


/*W600 main operation*/
void	w600_station_operation(void)
{
	w600_interrupt_operation();

	APP_MSG *msg = msg_fifo_read();
	if(msg == (APP_MSG *)0) return;

	APP_PRINT("%s\n", __func__);
	switch(msg->type)
	{
	case MSG_WLAN:
		AppWlanProc(msg->param);
		break;
		
	case MSG_SPI:
		AppSPIProc(msg->param);
		break;
		
	default:
		break;
	}
}


/*W600 send data*/
uint32_t	w600_station_txdata(uint8_t *data, uint16_t len)
{
	uint32_t result = 1;

	if(gsSTSysCtrol.WlanStatus == 2) {
		result = AppSocketSendProc(gsSTSysCtrol.FdTcp, data, len);
	}
	else {
		APP_PRINT("txdata fail, WlanStatus=%d\n", gsSTSysCtrol.WlanStatus);
		result = 0;
	}
	return result;
}

/*W600 interrupt triger */
void	w600_interrupt_operation_triger(void)
{
	w600_interrupt_triger++;
}

/*W600 interrupt operation */
void	w600_interrupt_operation(void)
{
	while (w600_interrupt_triger != 0) {
		
		sysctl_disable_irq();
		w600_interrupt_triger--;
		sysctl_enable_irq();
		
		APP_PRINT("%s\n", __func__);
		SPIRxData();
	}
}

/*W600 register rx-data callback*/
void	w600_register_rxdata_callback(w600_rxdata_cb_t callback)
{
	rxdata_operation = callback;
}

/*w600 socket status: 1>ready. */
uint32_t	w600_socket_ready(void)
{
	return (gsSTSysCtrol.WlanStatus == 2)? 1: 0;
}

/***********************************************************************
* Description : ������Ϣ��������
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static void AppWlanProc(INT32U param)
{
	switch(param)
	{
		case APP_WLAN_UP:
			gsSTSysCtrol.WlanStatus = 1;
			/*RIGetWlanStatus();*/
			msleep(100);
			AppCreatSocketProc(1000);
			break;
		case APP_WLAN_DOWN:
			gsSTSysCtrol.WlanStatus = 0;
			AppCloseSocketProc(gsSTSysCtrol.FdTcp);
			printk("zzzzzzzzzzzzzzzzzZZZZZZZZZZZ: RI_EVENT_LINKDOWN \n");
            RIWlanJoin();			
			break;
	}
}

/***********************************************************************
* Description : SPI��Ӧ����
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static void AppSPIProc(INT32U param)
{
	WM_SPI_RX_DESC* rxdesc = (WM_SPI_RX_DESC*)param;
	WM_SPI_HDR *ri = (WM_SPI_HDR *)rxdesc->buff;
	INT8U *pdata;
	INT16U length;
	
	if(AppRICheckSPIHeader(&ri->hdr))
	{
		SPIFreeRxBuff(rxdesc);
		return;
	}
	switch(ri->hdr.type)
	{
		case RI_CMD:
			AppRICmdProc(&ri->cmd);
			break;
		case RI_DATA:
			pdata = (INT8U*)ri + sizeof(WM_RI_HDR);
		
			length = Swap16(ri->hdr.length);
			APP_PRINT("Data length=%d\n", length);
			if(length >= 1500 - sizeof(WM_RI_HDR)) {
				APP_PRINT("\n\n !!!XXXXYYYYZZZZ!!! \n\n");
				length = 1500 - sizeof(WM_RI_HDR) - 1;
			}	
			*(pdata + length) = '\0';
			/*APP_PRINT("kevin debug RI_DATA = %s\r\n", pdata);*/

			if(rxdata_operation != (w600_rxdata_cb_t)0) {
				rxdata_operation(pdata, length);
			}
			break;
			
		default:
			break;
	}
	SPIFreeRxBuff(rxdesc);
	return;

}

/***********************************************************************
* Description : ���RIָ��ͬ���� 
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT32U AppRICheckSPIHeader(WM_RI_HDR *ri)
{
	if((NULL == ri) || (0xAA != ri->sync))
	{
		APP_PRINT("kevin debug ri->sync = %x\r\n", ri->sync);
		return 1;
	}
	return 0;
}

/***********************************************************************
* Description : RIָ����Ӧ��������
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT32U AppRICmdProc(WM_RI_CMD_HDR *cmd)
{
	INT8U *pdata;
	INT8U status;
	INT32U ip;

	INT8U xip[4];
	INT8U result[2];
	INT8U join_result;
	
	if(NULL == cmd)
	{
		return 1;
	}
	APP_PRINT("kevin debug RICmdProc code = %x, err = %x, ext = %x\r\n", cmd->code, cmd->err, cmd->ext);
	switch(cmd->code)
	{
		case RI_CMD_CUSTDATA:
			// length(2 Byte) + ssid(str) + pwd(str) + server(str) + id(str) + name(str)
			pdata = (INT8U*)cmd + sizeof(WM_RI_CMD_HDR);
			uint16_t data_len = pdata[1] * 256 + pdata[0];
			char *index;

			break;
			
		case RI_EVENT_INIT_END:
			printk("!!! RI_EVENT_INIT_END : W600 READY !!!\n");

			RISetWlanType(APP_WLAN_STA);
			break;
		case RI_CMD_WPRT: 
			/* RI_CMD_WPRT reponse*/
			RISetSSID(wlan_op.ssid);
			break;
		case RI_CMD_SSID:
			/* RI_CMD_SSID reponse*/
			RISetKey(APP_KEY_ASCII, 0, wlan_op.passwd);
			break;

		case RI_EVENT_JOIN_RES:
			join_result = *(uint8_t *)((uint8_t*)cmd + sizeof(WM_RI_CMD_HDR));
			if(join_result == 1) {
				break;
			}	
			APP_PRINT("retry join AP\n");
			/*Follow through*/
		case RI_CMD_KEY:
			RIWlanJoin();
			break;	
		
		case RI_EVENT_LINKUP:
			AppSendMsg(MSG_WLAN, APP_WLAN_UP);
			/*AppWlanProc(APP_WLAN_UP);*/			
			break;
		case RI_EVENT_LINKDOWN:
			AppSendMsg(MSG_WLAN, APP_WLAN_DOWN);
			/*AppWlanProc(APP_WLAN_DOWN);*/
			break;

		case RI_CMD_SKCT:
			pdata = (INT8U*)cmd + sizeof(WM_RI_CMD_HDR);
			gsSTSysCtrol.FdTcp = *pdata;
			APP_PRINT("### kevin debug AppCreatSocketProc = %d\r\n", gsSTSysCtrol.FdTcp);
			printk("!!! RI_CMD_SKCT : Socket cmd reponse !!!\n");			
			break;
			
		case RI_CMD_LINK_STATUS:
			pdata = (INT8U*)cmd + sizeof(WM_RI_CMD_HDR);
			status = *pdata++;
			xip[0] = *pdata++; xip[1] = *pdata++; xip[2] = *pdata++; xip[3] = *pdata++;
			
			APP_PRINT("### kevin debug RI_CMD_LINK_STATUS = %d, %d,%d.%d.%d\r\n", 
				status, (xip[0]),  (xip[1]), (xip[2]), (xip[3]));
			break;

		case	RI_EVENT_TCP_CONN:						
			pdata = (INT8U*)cmd + sizeof(WM_RI_CMD_HDR);
			result[0] = *pdata++;
			result[1] = *pdata;
			
			printk("!!! RI_EVENT_TCP_CONN : Socket Result: 0x%x 0x%x !!!\n", result[0], result[1]);
			
			if(result[1] == 0) {
				
				AppCreatSocketProc(1000);
			}
			else {
				gsSTSysCtrol.WlanStatus = 2;
			}
			break;
		case	RI_EVENT_TCP_DIS:

			gsSTSysCtrol.WlanStatus = 1;
			
			for(INT8U i=0; i<20; i++) {
				APP_PRINT("00000000000000000000000000000000000\n");
			}
			
			AppCreatSocketProc(1000);			
			break;
	}
	return 0;
}


/***********************************************************************
* Description : ����socket����
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT32U AppCreatSocketProc(uint16_t remote_port)
{
	WM_SOCKET_INFO sk;
	INT32U ret = 0;
	static	INT16U local_port = 0;

	memset(&sk, 0, sizeof(WM_SOCKET_INFO));
	sk.protocol = SOCKET_PROTO_TCP;
	sk.cs = SOCKET_CS_MODE_CLIENT;
	strcpy((char *)sk.hostname, wlan_op.remote_ip);
	sk.remote = wlan_op.remote_port;
	sk.local = 2000 + local_port;
	local_port++;
	if(local_port == 1000) local_port = 0;
	
	ret = RISocketCreate(&sk);

	return ret;
}

/***********************************************************************
* Description : ͨ��socket��������
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static uint32_t AppSocketSendProc(INT32U fd, INT8U *data, INT16U len)
{
	return RISocketTCPSend(fd, data, len);
}

/***********************************************************************
* Description : �ر�socket
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static void AppCloseSocketProc(INT32U fd)
{
	RISocketClose(fd);
}


