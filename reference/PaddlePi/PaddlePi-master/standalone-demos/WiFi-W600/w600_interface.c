
#include "app.h"
#include "spi.h"
#include "Includes.h"

#define SPI_DEBUG		0

#if 	SPI_DEBUG
#define SPI_PRINT 		printk
#else
#define SPI_PRINT(fmt, ...)
#endif


static 		WM_SPI_RX_DESC gsSPIRxDesc[SPI_RX_DESC_NUM];


/* malloc spi receive data buff.*/
static WM_SPI_RX_DESC* SPIGetRxBuff(INT32U size)
{
	INT32U i;

	for(i = 0; i < SPI_RX_DESC_NUM; i++)
	{
		if(0 == gsSPIRxDesc[i].valid)
		{
			gsSPIRxDesc[i].valid = 1;
			return &gsSPIRxDesc[i];
		}
	}
	SPI_PRINT("kevin debug SPIGetRxBuff gsSPIRxDesc err\r\n");
	return NULL;
}


/* free spi receive data buff.*/
void SPIFreeRxBuff(WM_SPI_RX_DESC* desc)
{
	desc->valid = 0;
	return;
}

/* spi receive data.*/
void SPIRxData(void)
{
	INT16U temp = 0;
	WM_SPI_RX_DESC *rxdesc;
	static INT8U tempdata[1500];
	
	INT8U cmd = SPI_REG_INT_STTS;
	spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
									SPI_DMA_RX_CHL, 
									SPI_DEVICE,
									SPI_CS_SELECT,
									&cmd,
									sizeof(cmd),
									(uint8_t *)&temp,
									sizeof(temp)); 

	if((temp != 0xffff) && (temp & 0x01))	//���ݻ������Ѿ�׼����
	{
		cmd = SPI_REG_RX_DAT_LEN;
		spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
										SPI_DMA_RX_CHL, 
										SPI_DEVICE,
										SPI_CS_SELECT,
										&cmd,
										sizeof(cmd),
										(uint8_t *)&temp,
										sizeof(temp)); 

		if(temp > 0)
		{
			if(temp % 4)
			{
				temp = ((temp + 3) / 4) << 2;
			}
			if(temp > 1500) temp = 1500;
			rxdesc = SPIGetRxBuff(temp);
			if(rxdesc)
			{
				cmd = SPI_CMD_RX_DATA;
				spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
												SPI_DMA_RX_CHL, 
												SPI_DEVICE,
												SPI_CS_SELECT,
												&cmd,
												sizeof(cmd),
												&(rxdesc->buff[0]),
												temp);
				/*Report receved data! */				
				AppSendMsg(MSG_SPI, (INT32U)rxdesc);
			}
			else
			{
				cmd = SPI_CMD_RX_DATA;
				spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
												SPI_DMA_RX_CHL, 
												SPI_DEVICE,
												SPI_CS_SELECT,
												&cmd,
												sizeof(cmd),
												&tempdata[0],
												temp);
				//- lost data !.				
			}
		}
	}
}


/* spi send command.*/
INT8U SPITxCmd(INT8U *TXBuf, INT16U CmdLen)
{
	INT16U volatile temp = 0;
	INT32U retry = 0;
	INT8U cmd = 0;
	
	if(NULL == TXBuf)
	{
		SPI_PRINT("###kevin debug SPITxCmd buff == NULL\r\n");
		return 0;
	}

	do
	{
		retry++;
		
		cmd = SPI_REG_TX_BUFF_AVAIL;
		spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
										SPI_DMA_RX_CHL, 
										SPI_DEVICE,
										SPI_CS_SELECT,
										&cmd,
										sizeof(cmd),
										(uint8_t *)&temp,
										sizeof(temp)); 		
		
		if(retry > SPI_TIMEOUT)
		{
			SPI_PRINT("###kevin debug SPI_CMD_TIMEOUT\r\n");
			return 0;
		}	
	}while((temp != 0xffff) && (0 == (temp & 0x02)));
	

	if(CmdLen > 0)
	{
		if(CmdLen % 4)
		{
			CmdLen = ((CmdLen + 3) / 4) << 2;
		}
		SPI_PRINT("kevin debug TX_BUFF_AVAIL = %d, cmdlen=%d\r\n", temp, CmdLen);

		cmd = SPI_CMD_TX_CMD;	
		spi_send_data_standard( SPI_DEVICE,
								SPI_CS_SELECT,
								&cmd,
								sizeof(cmd),
								TXBuf,
								CmdLen);
		
	}
	return 1;
}


/* spi send data.*/
INT8U SPITxData(INT8U *TXBuf, INT16U DataLen)
{
	INT16U volatile	temp = 0;
	INT16U retry=0;
	INT8U cmd = 0;
	
	if(NULL == TXBuf)
	{
		SPI_PRINT("### SPITxData buff == NULL\r\n");
		return 0;
	}

	do
	{
		retry ++;
		
		cmd = SPI_REG_TX_BUFF_AVAIL;	
		spi_receive_data_standard_dma(	SPI_DMA_TX_CHL, 
										SPI_DMA_RX_CHL, 
										SPI_DEVICE,
										SPI_CS_SELECT,
										&cmd,
										sizeof(cmd),
										(uint8_t *)&temp,
										sizeof(temp)); 		

		if(retry > SPI_TIMEOUT)
		{
			SPI_PRINT("kevin debug TX_BUFF_AVAIL SPI_TIMEOUT (SPITxData)\r\n");
			return 0;
		}
	}while((temp != 0xffff) && (0 == (temp & 0x01)));

	if(DataLen > 0)
	{
		if(DataLen % 4)
		{
			DataLen = ((DataLen + 3) / 4) << 2;
		}

		cmd = SPI_CMD_TX_DATA;
		spi_send_data_standard( SPI_DEVICE,
								SPI_CS_SELECT,
								&cmd,
								sizeof(cmd),
								TXBuf,
								DataLen);
	}	
	return 1;
}


