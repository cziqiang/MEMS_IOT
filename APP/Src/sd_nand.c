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

uint8_t SD_Type=0;//SD��������

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

//��SD������һ������
//����: uint8_t cmd   ���� 
//      uint32_t arg  �������
//      uint8_t crc   crcУ��ֵ	   
//����ֵ:SD�����ص���Ӧ															  
uint8_t SD_SendCommand(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t r1,Retry=0;	
	
	SD_CS0=1; SD_CS1 = 1;
	SD_SPI_TransmitReceive(0xff);//����д������ʱ
	SD_SPI_TransmitReceive(0xff);     
 	SD_SPI_TransmitReceive(0xff);  	 
    //Ƭѡ���õͣ�ѡ��SD��
    SD_CS1 = 0;

    //����
    SD_SPI_TransmitReceive(cmd | 0x40);//�ֱ�д������
    SD_SPI_TransmitReceive(arg >> 24);
    SD_SPI_TransmitReceive(arg >> 16);
    SD_SPI_TransmitReceive(arg >> 8);
    SD_SPI_TransmitReceive(arg);
    SD_SPI_TransmitReceive(crc); 
    //�ȴ���Ӧ����ʱ�˳�
    while((r1=SD_SPI_TransmitReceive(0xFF))==0xFF)
    {
        Retry++;	    
        if(Retry>200)break; 
    }   
    //�ر�Ƭѡ
		SD_CS1 = 1;

    //�������϶�������8��ʱ�ӣ���SD�����ʣ�µĹ���
    SD_SPI_TransmitReceive(0xFF);
    //����״ֵ̬
    return r1;
}	

//��SD������һ������(�����ǲ�ʧ��Ƭѡ�����к������ݴ�����
//����:u8 cmd   ���� 
//     u32 arg  �������
//     u8 crc   crcУ��ֵ	 
//����ֵ:SD�����ص���Ӧ															  
uint8_t SD_SendCommand_NoDeassert(uint8_t cmd, uint32_t arg, uint8_t crc)
{
	uint8_t Retry=0;	         
	uint8_t r1;			   
	SD_SPI_TransmitReceive(0xff);//����д������ʱ
	SD_SPI_TransmitReceive(0xff); 

	SD_CS1 = 0;//Ƭѡ���õͣ�ѡ��SD��	   
	//����
	SD_SPI_TransmitReceive(cmd | 0x40); //�ֱ�д������
	SD_SPI_TransmitReceive(arg >> 24);
	SD_SPI_TransmitReceive(arg >> 16);
	SD_SPI_TransmitReceive(arg >> 8);
	SD_SPI_TransmitReceive(arg);
	SD_SPI_TransmitReceive(crc);   
	//�ȴ���Ӧ����ʱ�˳�
	while((r1=SD_SPI_TransmitReceive(0xFF))==0xFF)
	{
			Retry++;	    
			if(Retry>200)break; 
	}  	  
	//������Ӧֵ
	return r1;
}

//��SD�����õ�����ģʽ
//����ֵ:0,�ɹ�����
//       1,����ʧ��
uint8_t SD_Idle_Sta(void)
{
	uint16_t i;
	uint8_t retry;	
    //�Ȳ���>74�����壬��SD���Լ���ʼ�����
	//delay_ms(2000);
    for(i=0;i<10;i++)SD_SPI_TransmitReceive(0xFF); 
    //-----------------SD����λ��idle��ʼ-----------------
    //ѭ����������CMD0��ֱ��SD������0x01,����IDLE״̬
    //��ʱ��ֱ���˳�
    retry = 0;
    do
    {	   
        //����CMD0����SD������IDLE״̬
        i = SD_SendCommand(CMD0, 0, 0x95);
        retry++;
    }while((i!=0x01)&&(retry<200));
		MSG_USART(MSG_LEVEL_INFO,"The SD1 init ret %d\r\n",i);
    //����ѭ���󣬼��ԭ�򣺳�ʼ���ɹ���or ���Գ�ʱ��
    if(retry==200)return 1; //ʧ��
	return 0;//�ɹ�	 						  
}		

//��ʼ��SD��
//����ɹ�����,����Զ�����SPI�ٶ�Ϊ18Mhz
//����ֵ:0��NO_ERR
//       1��TIME_OUT
//      99��NO_CARD																 
uint8_t SD_Init(void)
{
	uint8_t r1;      // ���SD���ķ���ֵ
  uint16_t retry;  // �������г�ʱ����
  uint8_t buff[6];
	
	SDNAND_SPI_Init();
	SD_CS1=1;
	if(SD_Idle_Sta()) return 1;//��ʱ����1 ���õ�idle ģʽʧ��	
	    //-----------------SD����λ��idle����-----------------	 
    //��ȡ��Ƭ��SD�汾��Ϣ
 	SD_CS1=0;	
	r1 = SD_SendCommand_NoDeassert(8, 0x1aa,0x87);	
	MSG_USART(MSG_LEVEL_INFO,"SD version:%d",r1);

		//V2.0�Ŀ���CMD8�����ᴫ��4�ֽڵ����ݣ�Ҫ�����ٽ���������
	buff[0] = SD_SPI_TransmitReceive(0xFF);  //should be 0x00
	buff[1] = SD_SPI_TransmitReceive(0xFF);  //should be 0x00
	buff[2] = SD_SPI_TransmitReceive(0xFF);  //should be 0x01
	buff[3] = SD_SPI_TransmitReceive(0xFF);  //should be 0xAA	    
	SD_CS1=1;	  
	SD_SPI_TransmitReceive(0xFF);//the next 8 clocks			 
	//�жϸÿ��Ƿ�֧��2.7V-3.6V�ĵ�ѹ��Χ
 
	retry = 0;
		//������ʼ��ָ��CMD55+ACMD41
	do
	{
		r1 = SD_SendCommand(CMD55, 0, 0);
		if(r1!=0x01)return r1;	   
		r1 = SD_SendCommand(ACMD41, 0x40000000, 0);
			if(retry>200)return r1;  //��ʱ�򷵻�r1״̬  
	}while(r1!=0);		  
	//��ʼ��ָ�����ɣ���������ȡOCR��Ϣ		   
	//-----------����SD2.0���汾��ʼ-----------
	r1 = SD_SendCommand_NoDeassert(CMD58, 0, 0);
	if(r1!=0x00)
	{
		SD_CS1=1;//�ͷ�SDƬѡ�ź�
		return r1;  //�������û�з�����ȷӦ��ֱ���˳�������Ӧ��	 
	}//��OCRָ����󣬽�������4�ֽڵ�OCR��Ϣ
	buff[0] = SD_SPI_TransmitReceive(0xFF);
	buff[1] = SD_SPI_TransmitReceive(0xFF); 
	buff[2] = SD_SPI_TransmitReceive(0xFF);
	buff[3] = SD_SPI_TransmitReceive(0xFF);		 
	//OCR������ɣ�Ƭѡ�ø�
	SD_CS1=1;
	SD_SPI_TransmitReceive(0xFF);	   
	//�����յ���OCR�е�bit30λ��CCS����ȷ����ΪSD2.0����SDHC
	//���CCS=1��SDHC   CCS=0��SD2.0
	if(buff[0]&0x40)SD_Type = SD_TYPE_V2HC;    //���CCS	 
	else SD_Type = SD_TYPE_V2;	 
	MSG_USART(MSG_LEVEL_INFO,"SD Type:%d",SD_Type);
	return r1;
}
//�ȴ�SD����Ӧ
//Response:Ҫ�õ��Ļ�Ӧֵ
//����ֵ:0,�ɹ��õ��˸û�Ӧֵ
//    ����,�õ���Ӧֵʧ��
uint8_t SD_GetResponse(uint8_t Response)
{
	uint16_t Count=0xFFF;//�ȴ�����	   						  
	while ((SD_SPI_TransmitReceive(0XFF)!=Response)&&Count)Count--;//�ȴ��õ�׼ȷ�Ļ�Ӧ  	  
	if (Count==0)return MSD_RESPONSE_FAILURE;//�õ���Ӧʧ��   
	else return MSD_RESPONSE_NO_ERROR;//��ȷ��Ӧ
}
//��SD���ж���ָ�����ȵ����ݣ������ڸ���λ��
//����: u8 *data(��Ŷ������ݵ��ڴ�>len)
//      u16 len(���ݳ��ȣ�
//      u8 release(������ɺ��Ƿ��ͷ�����CS�ø� 0�����ͷ� 1���ͷţ�	 
//����ֵ:0��NO_ERR
//  	 other��������Ϣ														  
uint8_t SD_ReceiveData(uint8_t *data, uint16_t len, uint8_t release)
{
    // ����һ�δ���
    SD_CS1=0;				  	  
	if(SD_GetResponse(0xFE))//�ȴ�SD������������ʼ����0xFE
	{	  
		SD_CS1=1;
		return 1;
	}
    while(len--)//��ʼ��������
    {
        *data=SD_SPI_TransmitReceive(0xFF);
        data++;
    }
    //������2��αCRC��dummy CRC��
    SD_SPI_TransmitReceive(0xFF);
    SD_SPI_TransmitReceive(0xFF);
    if(release==RELEASE)//�����ͷ����ߣ���CS�ø�
    {
        SD_CS1=1;//�������
        SD_SPI_TransmitReceive(0xFF);
    }											  					    
    return 0;
}		

//��ȡSD����CSD��Ϣ�������������ٶ���Ϣ
//����:u8 *cid_data(���CID���ڴ棬����16Byte��	    
//����ֵ:0��NO_ERR
//       1��TIME_OUT
//       other��������Ϣ														   
uint8_t SD_GetCSD(uint8_t *csd_data)
{
    uint8_t r1;	 
    r1=SD_SendCommand(CMD9,0,0xFF);//��CMD9�����CSD
    if(r1)return r1;  //û������ȷӦ�����˳�������  
    SD_ReceiveData(csd_data, 16, RELEASE);//����16���ֽڵ����� 
    return 0;
} 

//��ȡSD�����������ֽڣ�   
//����ֵ:0�� ȡ�������� 
//       ����:SD��������(�ֽ�)														  
uint32_t SD_GetCapacity(void)
{
    uint8_t csd[16];
    uint32_t Capacity;
    uint8_t r1;
    uint16_t i;
	uint16_t temp;  					    
	//ȡCSD��Ϣ������ڼ��������0
    if(SD_GetCSD(csd)!=0) return 0;	    
    //���ΪSDHC�����������淽ʽ����
    if((csd[0]&0xC0)==0x40)
    {									  
	    Capacity=((uint32_t)csd[8])<<8;
		Capacity+=(uint32_t)csd[9]+1;	 
        Capacity = (Capacity)*1024;//�õ�������
		Capacity*=512;//�õ��ֽ���			   
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
    	Capacity *= (uint32_t)temp;//�ֽ�Ϊ��λ 	  
    }
    return (uint32_t)Capacity;
}	

//��ȡSD����CID��Ϣ��������������Ϣ
//����: u8 *cid_data(���CID���ڴ棬����16Byte��	  
//����ֵ:0��NO_ERR
//		 1��TIME_OUT
//       other��������Ϣ														   
uint8_t SD_GetCID(uint8_t *cid_data)
{
    uint8_t r1;	   
    //��CMD10�����CID
    r1 = SD_SendCommand(CMD10,0,0xFF);
    if(r1 != 0x00)return r1;  //û������ȷӦ�����˳�������  
    SD_ReceiveData(cid_data,16,RELEASE);//����16���ֽڵ�����	 
    return 0;
}	

uint8_t SDTest(void)
{
	uint32_t sd_size;
	uint8_t retry = 3;	
	uint8_t CIDdata[16],status;
	while(SD_Init()!=0 && retry-->0) //��ⲻ��SD��
	{
		MSG_USART(MSG_LEVEL_INFO,"SD Card Failed!\r\n");
		HAL_Delay(500);
		MSG_USART(MSG_LEVEL_INFO,"Please Check!	");
		HAL_Delay(500);
	}
	if(retry == 255)return 1;
	//���SD���ɹ� 	
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
