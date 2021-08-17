/*
 *****************************************************************************
 * @file:    ad_nand.c
 * @brief:   CSNP4GCR01-AMW SD-NAND IC
 * @Author:  Cheng Ziqiang
 * @version: $V1.0$
 * @date:    $2021/7/11$
 *-----------------------------------------------------------------------------
 * Copyright (c) 2021 CCIC, Inc. All rights reserved.
*/

#include "sd_nand.h"

uint8_t SD_Type=0;//SD卡的类型

void SDNAND_SPI_Init(void)
{
		GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOB_CLK_ENABLE();
	
	//PB7--CS0_M		PB6--CS1_M		PB5--SCLK_M		PB4--MISO(SDO)	PB3--MOSI(SDI)
	/*Configure GPIO pin : PAPin */
  GPIO_InitStruct.Pin = GPIO_PIN_7|GPIO_PIN_6|GPIO_PIN_5|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	
  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, (GPIO_PIN_7|GPIO_PIN_6), GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_5,GPIO_PIN_RESET);

	/*Configure GPIO pin : PB4 MISO */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);	

}

uint8_t SD_SPI_TransmitReceive(uint8_t txData)
{
	uint8_t i,rxData;
	for(i=0;i<8;i++)
	{
		SD_SCK = 0;
		delay_us(1);
		if(txData & 0x80){
				SD_MOSI = 1;
		}else{
				SD_MOSI = 0;
		}
		txData <<= 1;
		delay_us(1);

		SD_SCK = 1;
		delay_us(1);
		rxData <<= 1;
		if(SD_MISO){
				rxData |= 0x01;
		}
		delay_us(1);
	}
	SD_SCK = 0;
	return rxData;
}

void SD_SPI_RW_Register(uint8_t *txBuf, uint8_t * rxBuf, uint16_t btNum)
{
	for(uint8_t i=0;i<btNum;i++)
	{
		rxBuf[i] = SD_SPI_TransmitReceive(txBuf[i]);
	}
}

//向SD卡发送一个命令
//输入: uint8_t cmd   命令 
//      uint32_t arg  命令参数
//      uint8_t crc   crc校验值	   
//返回值:SD卡返回的响应															  
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t r1,Retry=0;	
	
	SD_CS0=1; SD_CS1 = 1;
	SD_SPI_TransmitReceive(0xff);//高速写命令延时
	SD_SPI_TransmitReceive(0xff);     
 	SD_SPI_TransmitReceive(0xff);  	 
    //片选端置低，选中SD卡
    SD_CS1 = 0;

    //发送
    SD_SPI_TransmitReceive(cmd | 0x40);//分别写入命令
    SD_SPI_TransmitReceive(arg >> 24);
    SD_SPI_TransmitReceive(arg >> 16);
    SD_SPI_TransmitReceive(arg >> 8);
    SD_SPI_TransmitReceive(arg);
    SD_SPI_TransmitReceive(crc); 
    //等待响应，或超时退出
    while((r1=SD_SPI_TransmitReceive(0xFF))==0xFF)
    {
        Retry++;	    
        if(Retry>200)break; 
    }   
    //关闭片选
		SD_CS1 = 1;

    //在总线上额外增加8个时钟，让SD卡完成剩下的工作
    SD_SPI_TransmitReceive(0xFF);
    //返回状态值
    return r1;
}	

//向SD卡发送一个命令(结束是不失能片选，还有后续数据传来）
//输入:u8 cmd   命令 
//     u32 arg  命令参数
//     u8 crc   crc校验值	 
//返回值:SD卡返回的响应															  
uint8_t SD_SendCommand_NoDeassert(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t Retry=0;	         
	uint8_t r1;			   
	SD_SPI_TransmitReceive(0xff);//高速写命令延时
	SD_SPI_TransmitReceive(0xff); 

	SD_CS1 = 0;//片选端置低，选中SD卡	   
	//发送
	SD_SPI_TransmitReceive(cmd | 0x40); //分别写入命令
	SD_SPI_TransmitReceive(arg >> 24);
	SD_SPI_TransmitReceive(arg >> 16);
	SD_SPI_TransmitReceive(arg >> 8);
	SD_SPI_TransmitReceive(arg);
	SD_SPI_TransmitReceive(crc);   
	//等待响应，或超时退出
	while((r1=SD_SPI_TransmitReceive(0xFF))==0xFF)
	{
			Retry++;	    
			if(Retry>200)break; 
	}  	  
	//返回响应值
	return r1;
}

//把SD卡设置到挂起模式
//返回值:0,成功设置
//       1,设置失败
uint8_t SD_Idle_Sta(void)
{
	uint16_t i;
	uint8_t retry;	
    //先产生>74个脉冲，让SD卡自己初始化完成
	//delay_ms(2000);
    for(i=0;i<10;i++)SD_SPI_TransmitReceive(0xFF); 
    //-----------------SD卡复位到idle开始-----------------
    //循环连续发送CMD0，直到SD卡返回0x01,进入IDLE状态
    //超时则直接退出
    retry = 0;
    do
    {	   
        //发送CMD0，让SD卡进入IDLE状态
        i = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    }while((i!=0x01)&&(retry<200));
		MSG_USART(MSG_LEVEL_INFO,"The SD1 init ret %d\r\n",i);
    //跳出循环后，检查原因：初始化成功？or 重试超时？
    if(retry==200)return 1; //失败
	return 0;//成功	 						  
}		

//初始化SD卡
//如果成功返回,则会自动设置SPI速度为18Mhz
//返回值:0：NO_ERR
//       1：TIME_OUT
//      99：NO_CARD																 
uint8_t SD_Init(void)
{
	uint8_t r1;      // 存放SD卡的返回值
  uint16_t retry;  // 用来进行超时计数
  uint8_t buff[6];
	
	SDNAND_SPI_Init();
	SD_CS1=1;
	if(SD_Idle_Sta()) return 1;//超时返回1 设置到idle 模式失败	
	    //-----------------SD卡复位到idle结束-----------------	 
    //获取卡片的SD版本信息
 	SD_CS1=0;	
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	
	MSG_USART(MSG_LEVEL_INFO,"SD version:%d",r1);

		//V2.0的卡，CMD8命令后会传回4字节的数据，要跳过再结束本命令
	buff[0] = SD_SPI_TransmitReceive(0xFF);  //should be 0x00
	buff[1] = SD_SPI_TransmitReceive(0xFF);  //should be 0x00
	buff[2] = SD_SPI_TransmitReceive(0xFF);  //should be 0x01
	buff[3] = SD_SPI_TransmitReceive(0xFF);  //should be 0xAA	    
	SD_CS1=1;	  
	SD_SPI_TransmitReceive(0xFF);//the next 8 clocks			 
	//判断该卡是否支持2.7V-3.6V的电压范围
 
	retry = 0;
		//发卡初始化指令CMD55+ACMD41
	do
	{
		r1 = SD_SendCommand(CMD55, 0, 0);
		if(r1!=0x01)return r1;	   
		r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
			if(retry>200)return r1;  //超时则返回r1状态  
	}while(r1!=0);		  
	//初始化指令发送完成，接下来获取OCR信息		   
	//-----------鉴别SD2.0卡版本开始-----------
	r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
	if(r1!=0x00)
	{
		SD_CS1=1;//释放SD片选信号
		return r1;  //如果命令没有返回正确应答，直接退出，返回应答	 
	}//读OCR指令发出后，紧接着是4字节的OCR信息
	buff[0] = SD_SPI_TransmitReceive(0xFF);
	buff[1] = SD_SPI_TransmitReceive(0xFF); 
	buff[2] = SD_SPI_TransmitReceive(0xFF);
	buff[3] = SD_SPI_TransmitReceive(0xFF);		 
	//OCR接收完成，片选置高
	SD_CS1=1;
	SD_SPI_TransmitReceive(0xFF);	   
	//检查接收到的OCR中的bit30位（CCS），确定其为SD2.0还是SDHC
	//如果CCS=1：SDHC   CCS=0：SD2.0
	if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //检查CCS	 
	else SD_Type = SD_TYPE_V2;	 
	MSG_USART(MSG_LEVEL_INFO,"SD Type:%d",SD_Type);
	return r1;
}
//等待SD卡回应
//Response:要得到的回应值
//返回值:0,成功得到了该回应值
//    其他,得到回应值失败
uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFF;//等待次数	   						  
	while ((SD_SPI_TransmitReceive(0XFF)!=Response)&&Count)Count--;//等待得到准确的回应  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;//得到回应失败   
	else return MSD_RESPONSE_NO_ERROR;//正确回应
}
//从SD卡中读回指定长度的数据，放置在给定位置
//输入: u8 *data(存放读回数据的内存>len)
//      u16 len(数据长度）
//      u8 release(传输完成后是否释放总线CS置高 0：不释放 1：释放）	 
//返回值:0：NO_ERR
//  	 other：错误信息														  
uint8_t SD_ReceiveData(uint8_t *data, uint16_t len, uint8_t release)
{
    // 启动一次传输
    SD_CS1=0;				  	  
	if(SD_GetResponse(0xFE))//等待SD卡发回数据起始令牌0xFE
	{	  
		SD_CS1=1;
		return 1;
	}
    while(len--)//开始接收数据
    {
        *data=SD_SPI_TransmitReceive(0xFF);
        data++;
    }
    //下面是2个伪CRC（dummy CRC）
    SD_SPI_TransmitReceive(0xFF);
    SD_SPI_TransmitReceive(0xFF);
    if(release==RELEASE)//按需释放总线，将CS置高
    {
        SD_CS1=1;//传输结束
        SD_SPI_TransmitReceive(0xFF);
    }											  					    
    return 0;
}		

//获取SD卡的CSD信息，包括容量和速度信息
//输入:u8 *cid_data(存放CID的内存，至少16Byte）	    
//返回值:0：NO_ERR
//       1：TIME_OUT
//       other：错误信息														   
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;	 
    r1=SD_SendCommand(CMD9,0,0xFF);//发CMD9命令，读CSD
    if(r1)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(csd_data, 16, RELEASE);//接收16个字节的数据 
    return 0;
} 

//获取SD卡的容量（字节）   
//返回值:0： 取容量出错 
//       其他:SD卡的容量(字节)														  
uint32_t SD_GetCapacity(void)
{
    uint8_t csd[16];
    uint32_t Capacity;
    uint8_t r1;
    uint16_t i;
	uint16_t temp;  					    
	//取CSD信息，如果期间出错，返回0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //如果为SDHC卡，按照下面方式计算
    if((csd[0]&0xC0)==0x40)
    {									  
	    Capacity=((uint32_t)csd[8])<<8;
		Capacity+=(uint32_t)csd[9]+1;	 
        Capacity = (Capacity)*1024;//得到扇区数
		Capacity*=512;//得到字节数			   
    }
    else
    {		    
    	i = csd[6]&0x03;
    	i<<=8;
    	i += csd[7];
    	i<<=2;
    	i += ((csd[8]&0xc0)>>6);
        //C_SIZE_MULT
    	r1 = csd[9]&0x03;
    	r1<<=1;
    	r1 += ((csd[10]&0x80)>>7);	 
    	r1+=2;//BLOCKNR
    	temp = 1;
    	while(r1)
    	{
    		temp*=2;
    		r1--;
    	}
    	Capacity = ((uint32_t)(i+1))*((uint32_t)temp);	 
        // READ_BL_LEN
    	i = csd[5]&0x0f;
        //BLOCK_LEN
    	temp = 1;
    	while(i)
    	{
    		temp*=2;
    		i--;
    	}
        //The final result
    	Capacity *= (uint32_t)temp;//字节为单位 	  
    }
    return (uint32_t)Capacity;
}	

//获取SD卡的CID信息，包括制造商信息
//输入: u8 *cid_data(存放CID的内存，至少16Byte）	  
//返回值:0：NO_ERR
//		 1：TIME_OUT
//       other：错误信息														   
uint8_t SD_GetCID(uint8_t *cid_data)
{
    uint8_t r1;	   
    //发CMD10命令，读CID
    r1 = SD_SendCommand(CMD10,0,0xFF);
    if(r1 != 0x00)return r1;  //没返回正确应答，则退出，报错  
    SD_ReceiveData(cid_data,16,RELEASE);//接收16个字节的数据	 
    return 0;
}	

uint8_t SDTest(void)
{
	uint32_t sd_size;
	uint8_t retry = 3;	
	uint8_t CIDdata[16],status;
	while(SD_Init()!=0 && retry-->0) //检测不到SD卡
	{
		MSG_USART(MSG_LEVEL_INFO,"SD Card Failed!\r\n");
		HAL_Delay(500);
		MSG_USART(MSG_LEVEL_INFO,"Please Check!	");
		HAL_Delay(500);
	}
	if(retry == 255)return 1;
	//检测SD卡成功 	
	sd_size=SD_GetCapacity();											
	MSG_USART(MSG_LEVEL_INFO,"SD Card Checked OK \r\n");
	MSG_USART(MSG_LEVEL_INFO,"SD Card Size:  %d Byte\r\n",sd_size);	
	status = SD_GetCID(CIDdata);
	MSG_USART(MSG_LEVEL_INFO,"The CID infomation [DEC]: status:%d",status);
	for(retry = 0;retry <16;retry++)
		MSG_USART(MSG_LEVEL_INFO,"CIDregister[%d]: %d",retry,CIDdata[retry]);
	MSG_USART(MSG_LEVEL_INFO,"The CID infomation [HEX]: status:%d",status);
	for(retry = 0;retry <16;retry++)
		MSG_USART(MSG_LEVEL_INFO,"CIDregister[%d]: %X",retry,CIDdata[retry]);
	return 0;
}
