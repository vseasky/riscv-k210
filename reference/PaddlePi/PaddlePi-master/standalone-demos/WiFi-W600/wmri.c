/***********************************************************************
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : kevin 
* Date			: 2015-02-02
***********************************************************************/

/***********************************************************************
*	INCLUDE FILES
***********************************************************************/
#include "app.h"

/***********************************************************************
*	APPLICATION GLOBALS
***********************************************************************/
static INT8U gsSeqNum = 0;


/***********************************************************************
*	LOCAL FUNCTION PROTOTYPES
***********************************************************************/
static int StringToIpaddr(const char *buf, INT8U *addr);
static INT8U RICheckSum(INT8U *data, INT16U len);
static INT32U RICreateCmdHeader(WM_SPI_HDR *header, INT8U cmd, INT16U len);

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : houxf 
***********************************************************************/
static int StringToIpaddr(const char *buf, INT8U *addr)
{
	int count = 0, rc = 0;
	int in[4];
	char c;

	rc = sscanf(buf, "%u.%u.%u.%u%c", &in[0], &in[1], &in[2], &in[3], &c);
	if (rc != 4 && (rc != 5 || c != '\n'))
	{
		return -1;
	}
	for (count = 0; count < 4; count++) 
	{
		if (in[count] > 255)
		{
			return -1;
		}
		addr[count] = in[count];
	}
	return 0;
}
/***********************************************************************
* Description : 计算校验和
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT8U RICheckSum(INT8U *data, INT16U len)
{
	INT16U i;
	INT16U sum = 0;
	
	for(i = 0; i < len; i++)
	{
		sum += *data++;
	}
	return sum;
}
/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT32U RICreateCmdHeader(WM_SPI_HDR *header, INT8U cmd, INT16U len)
{	
	if(NULL == header)
	{
		return 1;
	}
	RI_PRINT("kevin debug CMD cmd = 0x%x, len = %d\r\n", cmd, len);
	header->hdr.sync		= RI_CMD_SYNC;
	header->hdr.type		= RI_CMD;
	header->hdr.length		= Swap16(len + sizeof(WM_RI_CMD_HDR));
	header->hdr.seq_num		= gsSeqNum++;
	header->hdr.flag		= 0;
	header->hdr.dest_addr	= 0;
	header->hdr.chk			= RICheckSum(&header->hdr.type, 6);
	header->cmd.msg_type	= RI_CMD_MT_REQ;
	header->cmd.code		= cmd;
	header->cmd.err			= 0;
	if(len)
	{
		header->cmd.ext		= 1;
	}
	else
	{
		header->cmd.ext		= 0;
	}
	return 0;
}

/***********************************************************************
* Description : 保存参数
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
/*INT32U RISaveParam(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_PMTF, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}*/
/***********************************************************************
* Description : 复位模块
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
/*INT32U RIResetDevice(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_RESET, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}*/
/***********************************************************************
* Description : 设置一键配置
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISetOneShotCfg(INT8U flag)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = flag; 
	len++;
	RICreateCmdHeader(header, RI_CMD_ONESHOT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 设置网络类型
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISetWlanType(INT32U type)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = type; 
	len++;
	RICreateCmdHeader(header, RI_CMD_WPRT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 设置SSID
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISetSSID(INT8U *ssid)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	len = strlen((char *)ssid);
	buff[offset++] = len; 
	memcpy((void *)&buff[offset++], ssid, len);
	len++;
	RICreateCmdHeader(header, RI_CMD_SSID, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 设置密码
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISetKey(INT8U type, INT8U index, INT8U *key)
{
	INT8U buff[96];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = type; 
	len++;
	buff[offset++] = index; 
	len++;
	buff[offset++] = strlen((char *)key); 
	len++;
	memcpy((void *)&buff[offset++], key, strlen((char *)key));
	len += strlen((char *)key);
	RICreateCmdHeader(header, RI_CMD_KEY, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 设置加密方式
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
/*INT32U RISetEncrypt(INT32U type)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = type; 
	len++;
	RICreateCmdHeader(header, RI_CMD_ENCRYPT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/
/***********************************************************************
* Description : 设置IP相关信息
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
/*INT32U RISetNip(INT8U dhcp, INT8U *ip, INT8U *netmast, INT8U *gateway, INT8U *dns)
{
	INT8U buff[96];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = dhcp; 
	len++;
//	if(dhcp)
//	{
		StringToIpaddr((char *)ip, &buff[offset]);
		offset += 4; len += 4;
		StringToIpaddr((char *)netmast, &buff[offset]);
		offset += 4; len += 4;
		StringToIpaddr((char *)gateway, &buff[offset]);
		offset += 4; len += 4;
		StringToIpaddr((char *)dns, &buff[offset]);
		offset += 4; len += 4;
//	}
	RICreateCmdHeader(header, RI_CMD_NIP, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/
/***********************************************************************
* Description : STA模式加入网络；AP模式创建网络
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RIWlanJoin(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_WJOIN, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}
/***********************************************************************
* Description : 获取网络连接状态
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RIGetWlanStatus(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_LINK_STATUS, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}
/***********************************************************************
* Description : 创建socket连接
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISocketCreate(WM_SOCKET_INFO *sk)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = sk->protocol; 
	len++;
	buff[offset++] = sk->cs; 
	len++;
	buff[offset++] = 4; 
	len++;
	StringToIpaddr((char *)sk->hostname, &buff[offset]);
	offset += 4; len += 4;
	//*(INT16U *)&buff[offset] = Swap16(sk->remote);
	buff[offset++] = (INT8U)(sk->remote >> 8);
	buff[offset++] = (INT8U)(sk->remote);
	
	/*offset += 2;*/ len += 2;
	//*(INT16U *)&buff[offset] = Swap16(sk->local);
	buff[offset++] = (INT8U)(sk->local >> 8);
	buff[offset++] = (INT8U)(sk->local);	
	/*offset += 2;*/ len += 2;

	RICreateCmdHeader(header, RI_CMD_SKCT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 关闭socket连接
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISocketClose(INT32U fd)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = fd; 
	len++;
	RICreateCmdHeader(header, RI_CMD_SKCLOSE, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}
/***********************************************************************
* Description : 获取当前收到的数据长度
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
/*INT32U RIGetSocketReceiveLen(INT32U fd)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = fd; 
	len++;
	RICreateCmdHeader(header, RI_CMD_SKSTT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/
/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
static INT32U RICreateDataHeader(WM_RI_HDR *header, INT32U fd, INT16U len, INT16U type)
{
	if(NULL == header)
	{
		return 1;
	}
//	RI_PRINT("kevin debug CMD cmd = 0x%x, len = %d\r\n", cmd, len);
	header->sync		= RI_CMD_SYNC;
	header->type		= RI_DATA;
	header->length		= Swap16(len);
	header->seq_num		= gsSeqNum++;
	header->flag		= 0;
	header->dest_addr	= fd & 0x3F;
	if(type)
	{
		header->dest_addr |= 0x40; 
	}
	header->chk = RICheckSum(&header->type, 6);
	return 0;
}



INT8U tcp_sendbuff[1500];
/***********************************************************************
* Description : socket发送数据
* Arguments   : 
* Returns     : 
* Author      : houxf
***********************************************************************/
INT32U RISocketTCPSend(INT32U fd, INT8U *data, INT16U len)
{
	WM_RI_HDR *header;
	INT32U retry = 3;
	
	header = (WM_RI_HDR *)tcp_sendbuff;
	memcpy((void *)&tcp_sendbuff[sizeof(WM_RI_HDR)], data, len);
	RICreateDataHeader(header, fd, len, 0);
	do {
	
		if(1 == SPITxData(tcp_sendbuff, sizeof(WM_RI_HDR) + len)) break;
		retry--;
		RI_PRINT("retry=%d\n", retry);
		/*msleep(5);*/
	}while(retry);
	
	if(retry == 0) { 
		RI_PRINT("%s FAIL!\n", __func__);
		return 0;
	}
	return 1;
}


INT32U RI_txtest_set_channel(INT8U channel, INT8U bandwith)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = channel; 
	len++;
	buff[offset++] = bandwith; 
	len++;	
	RICreateCmdHeader(header, RI_CMD_TEST_CHANNEL, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
}

INT32U RI_txtest_startup(INT32U packetlen, INT32U gain, INT32U rate)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	/*
	32bit  32bit		32bit 	 32bit	 32bit		32bit  32bit  32bit 	 
	res    count		pdulen  txgain  datarate    res	   res	  res
	*/
	buff[offset++] = 0; buff[offset++] = 0; buff[offset++] = 0; buff[offset++] = 0; 
	buff[offset++] = 0; buff[offset++] = 0; buff[offset++] = 0; buff[offset++] = 0; 
	buff[offset++] = (INT8U)(packetlen>>24); buff[offset++] = (INT8U)(packetlen>>16); buff[offset++] = (INT8U)(packetlen>>8); buff[offset++] = (INT8U)(packetlen>>0); 
	buff[offset++] = (INT8U)(gain>>24); 	 buff[offset++] = (INT8U)(gain>>16); 	  buff[offset++] = (INT8U)(gain>>8); 	  buff[offset++] = (INT8U)(gain>>0);
	buff[offset++] = (INT8U)(rate>>24); 	 buff[offset++] = (INT8U)(rate>>16); 	  buff[offset++] = (INT8U)(rate>>8); 	  buff[offset++] = (INT8U)(rate>>0);
	len = 32;

	RICreateCmdHeader(header, RI_CMD_TEST_STARTUP, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
}

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_ENTS(INT8U ps_type, INT8U wake_type, INT16U delay_time,INT16U wake_time)
{
	INT8U buff[32]; INT16U* buff2;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = ps_type; 
	len++;

	buff[offset++] = wake_type; 
	len++;
	
    buff2 = (INT16U*)(buff+offset);
	
	buff2[0] = delay_time;
	len += 2;
	
	buff2[1] = wake_time;
	len += 2;
	
	RICreateCmdHeader(header, RI_CMD_PS, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_RSTF(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_RESET_FLASH, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_IOC(INT8U gpio, INT8U direc, INT8U status)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = gpio; 
	len++;
	
	buff[offset++] = direc; 
	len++;

	buff[offset++] = status; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_GPIO, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_QMAC(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	RICreateCmdHeader(header, RI_CMD_MAC, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_QVER(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	
	RICreateCmdHeader(header, RI_CMD_VER, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;

}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WLEAVE(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	
	RICreateCmdHeader(header, RI_CMD_WLEAVE, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WSCAN(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	
	RICreateCmdHeader(header, RI_CMD_WSCAN, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WPSST(void)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	
	RICreateCmdHeader(header, RI_CMD_WPSST, 0);
	SPITxCmd(buff, sizeof(WM_SPI_HDR));
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_SKSDF(INT8U socket)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);

	buff[offset++] = socket; 
	len++;

	RICreateCmdHeader(header, RI_CMD_SKSDF, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_BSSID(INT8U enable,INT8U* bssid)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	memcpy((void*)&buff[offset],bssid,strlen(bssid));
	offset += strlen(bssid); len += strlen(bssid); 
	
	RICreateCmdHeader(header, RI_CMD_BSSID, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;		
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_BRDSSID(INT8U enable)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	RICreateCmdHeader(header, RI_CMD_BRD_SSID, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_CHL(INT8U enable,INT8U channel)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	buff[offset++] = channel;
	len++;
	
	RICreateCmdHeader(header, RI_CMD_CHNL, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR)+len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WREG(INT16U region)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset] = region; 
	len += 2;
	
	RICreateCmdHeader(header, RI_CMD_WREG, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WBGR(INT8U bg_mode,INT8U max_rate)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = bg_mode; 
	len++;
	buff[offset++] = max_rate; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_WBGR, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WATC(INT8U enable)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_WATC, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WPSM(INT8U enable)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_WPSM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_WARM(INT8U enable)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_WARM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}

INT32U RI_WWPS(INT8U enable,INT8U pin)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	
	buff[offset++] = pin; 
	len++;	
	
	RICreateCmdHeader(header, RI_CMD_WPS, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_ATM(INT8U mode)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = mode; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_ATM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}

INT32U RI_ATRM(WM_SOCKET_INFO *sk)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	buff[offset++] = sk->protocol; 
	len++;
	buff[offset++] = sk->cs; 
	len++;
	buff[offset++] = 4; 
	len++;
	StringToIpaddr((char *)sk->hostname, &buff[offset]);
	offset += 4; len += 4;
	*(INT16U *)&buff[offset] = Swap16(sk->remote);
	offset += 2; len += 2;
	*(INT16U *)&buff[offset] = Swap16(sk->local);
	offset += 2; len += 2;
	RICreateCmdHeader(header, RI_CMD_ATRM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}

// RI_CMD_AOLM  
INT32U RI_PORTM(INT8U mode)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = mode; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_PORTM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_UART(INT32U baudrate,INT8U databit,INT8U stopbit,INT8U parity,INT8U flowcontrol)
{
	INT8U buff[32];INT32U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	ptr = (INT32U*)(buff + offset);
	ptr[0] = baudrate; 
	offset += 3;
	len += 3;
	
	buff[offset++] = databit; 
	len++;

	buff[offset++] = stopbit; 
	len++;	

	buff[offset++] = parity; 
	len++;	
	
	buff[offset++] = flowcontrol; 
	len++;	
	
	RICreateCmdHeader(header, RI_CMD_PORTM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;

}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_ATLT(INT16U length)
{
	INT8U buff[32];
	INT16U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);

  	ptr = (INT16U*)&buff[offset];
	ptr[0] = length; 
	len += 2;
	
	RICreateCmdHeader(header, RI_CMD_ATLT, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_DNS(INT8U* dns)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);

	len = strlen((char *)dns);
	buff[offset++] = len; 
	memcpy((void *)&buff[offset++], dns, len);
	
	RICreateCmdHeader(header, RI_CMD_DNS, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_DDNS(INT8U enable,INT8U* user,INT8U* pass)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0,len1 = 0,len2 = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);

    buff[offset++] = enable; 
	len++;

	len1 = strlen((char *)user);
	buff[offset++] = len1; 
	memcpy((void *)&buff[offset], user, len1);
	offset += len1;


	len2 = strlen((char *)pass);
	buff[offset++] = len2; 
	memcpy((void *)&buff[offset], pass, len2);
	offset += len2;

	len += len1;
	len += len2;
	
    RICreateCmdHeader(header, RI_CMD_DDNS, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_UPNP(INT8U enable)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = enable; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_UPNP, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_DNAME(INT8U* DNAME)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	len = strlen((char *)DNAME);
	buff[offset++] = len; 
	memcpy((void *)&buff[offset++], DNAME, len);
	len++;
	
	RICreateCmdHeader(header, RI_CMD_DNAME, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;

}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_PASS(INT8U* pass)
{
	INT8U buff[64];
	WM_SPI_HDR *header;
	INT16U len = 0, offset;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);

	len = strlen((char *)pass);
	memcpy((void *)&buff[offset], pass, len);

	RICreateCmdHeader(header, RI_CMD_PASS, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/


/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_REGR(INT32U addr,INT8U num)
{
	INT8U buff[128];
	INT32U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT32U*)&buff[offset];
	ptr[0] = addr;
	len += 32; offset += 32;
	buff[offset++] = num;
	len++;
	
	RICreateCmdHeader(header, RI_CMD_REGR, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description :  val 的值暂时为一个    ，num==1
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_REGW(INT32U addr,INT8U num,INT32U val)
{
	INT8U buff[128];
	INT32U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT32U*)&buff[offset];
	ptr[0] = addr;
	len += 32; offset += 32;
	buff[offset++] = num;
	len++;
	
	ptr = (INT32U*)&buff[offset];
	ptr[0] = val;
	len += 32; offset += 32;
	
	RICreateCmdHeader(header, RI_CMD_REGW, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_RFR(INT16U addr,INT8U num)
{
	INT8U buff[128];
	INT16U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT16U*)&buff[offset];
	ptr[0] = addr;
	len += 16; offset += 16;
	buff[offset++] = num;
	len++;
	
	RICreateCmdHeader(header, RI_CMD_RFR, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_RFW(INT16U addr,INT8U num,INT16U val)
{
	INT8U buff[128];
	INT16U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT16U*)&buff[offset];
	ptr[0] = addr;
	len += 16; offset += 16;
	buff[offset++] = num;
	len++;

	ptr = (INT16U*)&buff[offset];
	ptr[0] = val;
	len += 16; offset += 16;
	
	RICreateCmdHeader(header, RI_CMD_RFW, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_FLSR(INT16U addr,INT8U num)
{
	INT8U buff[128];
	INT32U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT32U*)&buff[offset];
	ptr[0] = addr;
	len += 32; offset += 32;
	buff[offset++] = num;
	len++;
	
	RICreateCmdHeader(header, RI_CMD_FLSR, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description :  val 的值暂时为一个    ，num==1
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_FLSW(INT32U addr,INT8U num,INT32U val)
{
	INT8U buff[128];
	INT32U* ptr;
	WM_SPI_HDR *header;
	INT16U len = 0, offset;
	
	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);	

	ptr = (INT32U*)&buff[offset];
	ptr[0] = addr;
	len += 32; offset += 32;
	buff[offset++] = num;
	len++;
	
	ptr = (INT32U*)&buff[offset];
	ptr[0] = val;
	len += 32; offset += 32;
	
	RICreateCmdHeader(header, RI_CMD_FLSW, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;	
}*/

/***********************************************************************
* Description : 
* Arguments   : 
* Returns     : 
* Author      : hans hu
***********************************************************************/
/*INT32U RI_UPDM(INT8U mode)
{
	INT8U buff[32];
	WM_SPI_HDR *header;
	INT16U len = 0, offset = 0;

	memset((char *)buff, 0, sizeof(buff));
	header = (WM_SPI_HDR *)buff;
	offset = sizeof(WM_SPI_HDR);
	
	buff[offset++] = mode; 
	len++;
	
	RICreateCmdHeader(header, RI_CMD_UPDM, len);
	SPITxCmd(buff, sizeof(WM_SPI_HDR) + len);
	return 0;
}*/

