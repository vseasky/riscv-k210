#ifndef __W600_INTERFACE_H
#define __W600_INTERFACE_H


#define SPI_RX_DESC_NUM			30

#define SPI_TIMEOUT				0xf00		
#define SPI_REG_INT_STTS		0x06
#define SPI_REG_RX_DAT_LEN		0x02
#define SPI_REG_TX_BUFF_AVAIL	0x03
#define SPI_CMD_RX_DATA			0x10
#define SPI_CMD_TX_CMD			0x91
#define SPI_CMD_TX_DATA			0x90


void SPIFreeRxBuff(WM_SPI_RX_DESC* desc);
INT8U SPITxCmd(INT8U *TXBuf, INT16U CmdLen);
INT8U SPITxData(INT8U *TXBuf, INT16U DataLen);
void SPIRxData(void);


#endif	   
