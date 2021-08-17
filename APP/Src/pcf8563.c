/*
 *****************************************************************************
 * @file:    PCF8563.c
 * @brief:   PCF8563 RTC IC
 * @Author:  Cheng Ziqiang
 * @version: $V1.0$
 * @date:    $2021/7/18$
 *-----------------------------------------------------------------------------
 * Copyright (c) 2021 CCIC, Inc. All rights reserved.
*/

#include "pcf8563.h"

void PCF8563_Init(void)
{
	uint8_t data;
	IIC_Init();
	PCF8563_WriteOneReg(PCF8563_REG_CONSTA1,0x08);
	PCF8563_ReadOneReg(PCF8563_REG_SECOND,&data);
//	if(data & 0x80)
//	{
		MSG_USART(MSG_LEVEL_ERROR,"The RTC may be powered down and need to reset the time.");
		PCF8563_SetTimer(21,7,18,6,17,46,55);	
//	}
//	else
		MSG_USART(MSG_LEVEL_INFO,"PCF8563 Initial successful.");
}

void PCF8563_SetTimer(uint8_t year,uint8_t month,uint8_t date,uint8_t week,uint8_t hour,uint8_t min,uint8_t sec)
{
	uint8_t data[7];
	data[0] = DECtoBCD(sec);	
	data[1] = DECtoBCD(min);	
	data[2] = DECtoBCD(hour);	
	data[3] = DECtoBCD(date);	
	data[4] = DECtoBCD(week);
	data[5] = DECtoBCD(month);	
	data[6] = DECtoBCD(year);
	
	PCF8563_WriteReg(PCF8563_REG_SECOND, data, 7);	
	PCF8563_WriteOneReg(PCF8563_REG_CONSTA1, 0x08);
}

void PCF8563_GetTimer(void)
{
	RTC_HandlerType timer;
	memset(&timer,0,sizeof(RTC_HandlerType));
	uint8_t data[7];
	uint8_t retry;
	for(retry = 0;retry < 3;retry ++)
	{
		PCF8563_ReadReg(PCF8563_REG_SECOND, data, 7);
		if(data[0] & 0x80)	
		{
			PCF8563_SetTimer(21,7,18,6,17,46,55);	
		}
		else
		{
//		printf("%02X,%02X,%02X,%02X,%02X,%02X,%02X\r\n",data[0],data[1],data[2],data[3],data[4],data[5],data[6]);
			if(timer.second!=BCDtoDEC(data[0]&0x7F))
			{
				timer.second = BCDtoDEC(data[0]&0x7F);;
				timer.minute = BCDtoDEC(data[1]&0x7F);
				timer.hour = BCDtoDEC(data[2]&0x3F);
				timer.day = BCDtoDEC(data[3]&0x3F);
				timer.week = BCDtoDEC(data[4]&0x07);
				timer.month = BCDtoDEC(data[5]&0x1F);
				timer.year = BCDtoDEC(data[6]);
				MSG_USART(MSG_LEVEL_INFO,"%04d-%02d-%02d  %02d:%02d:%02d  %2d",timer.year+2000,timer.month,timer.day,
							timer.hour,timer.minute,timer.second,timer.week);
				PCF8563_Get_TimeStamp(timer);
			}
		}
	}
}

uint32_t PCF8563_Get_TimeStamp(RTC_HandlerType rtc_timer)
{
	//Unix times stamp
	static const uint16_t AddMonDays[12] = {0,31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
	uint16_t Year = rtc_timer.year+2000;
	uint32_t result = (Year - 1970) * 365 + AddMonDays[rtc_timer.month-1] + rtc_timer.day ;
	result += (rtc_timer.month>2 && (Year % 4 == 0) && (Year % 100 != 0 || Year % 400 == 0));
	Year -= 1970;
	result += (Year/4 - Year/100 + Year/400);
//	result = result * 24 * 3600 + (sTime.Hours-8) * 3600 + sTime.Minutes * 60 + sTime.Seconds;
	result = result * (675<<7) + (rtc_timer.hour-8) * (225<<4) + rtc_timer.minute * 60 + rtc_timer.second;
	
	MSG_USART(MSG_LEVEL_NULL,"Time stamp: %u\r\n",result);
	
	return result;
}

//Write one register
void PCF8563_WriteOneReg(uint8_t regType, uint8_t data) 
{
	IIC_Start();
	IIC_Send_Byte(PCF8563_WRITE_ADDR);
	IIC_Send_Byte(regType);
	IIC_Send_Byte(data);
	IIC_Stop();
}

//Write multiple registers
void PCF8563_WriteReg(uint8_t regType, uint8_t *pData, uint8_t regNum) 
{
	uint8_t i;
	IIC_Start();
	IIC_Send_Byte(PCF8563_WRITE_ADDR);
	IIC_Send_Byte(regType);
	
	for(i = 0;i < regNum;i ++)
	{
		IIC_Send_Byte(pData[i]);
	}
	IIC_Stop();
}

//Read one register
void PCF8563_ReadOneReg(uint8_t regType, uint8_t *pData) 
{
	IIC_Start();
	IIC_Send_Byte(PCF8563_WRITE_ADDR);
	IIC_Send_Byte(regType);
	IIC_Start();
	IIC_Send_Byte(PCF8563_READ_ADDR);
	*pData = IIC_Read_Byte(0);					//no ACK
	IIC_Stop();
}

//Read multiple register
void PCF8563_ReadReg(uint8_t regType, uint8_t *pData, uint8_t regNum) 
{
	uint8_t i;
	IIC_Start();
	IIC_Send_Byte(PCF8563_WRITE_ADDR);
	IIC_Send_Byte(regType);
	IIC_Start();
	IIC_Send_Byte(PCF8563_READ_ADDR);
	
	for(i = 0;i < regNum;i ++)
	{
		if(i == (regNum-1))	
		{
			pData[i] = IIC_Read_Byte(0);					//no ACK
		}
		else
		{
			pData[i] = IIC_Read_Byte(1);					// ACK
		}
	}
	IIC_Stop();
}

static uint8_t DECtoBCD(uint8_t DEC) 
{
	return ((uint8_t)(DEC/10)<<4)+(DEC%10);
}
 
static uint8_t BCDtoDEC(uint8_t BCD)
{
	return (uint8_t)(BCD>>4)*10+(BCD&0x0f);
}

//初始化IIC
void IIC_Init(void)
{			
  GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOC_CLK_ENABLE();

  //GPIO PC0,PC1初始化设置
	GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	IIC_SCL=1;
	IIC_SDA=1;
}
//产生IIC起始信号
void IIC_Start(void)
{
	SDA_OUT();     //sda线输出
	IIC_SDA=1;	  	  
	IIC_SCL=1;
	delay_us(10);
 	IIC_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(10);
	IIC_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	  
//产生IIC停止信号
void IIC_Stop(void)
{
	SDA_OUT();//sda线输出
	IIC_SCL=0;
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(10);
	IIC_SCL=1; 
	IIC_SDA=1;//发送I2C总线结束信号
	delay_us(10);							   	
}
//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;
	SDA_IN();      //SDA设置为输入  
	IIC_SDA=1;delay_us(10);	   
	IIC_SCL=1;delay_us(10);	 
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//时钟输出0 	   
	return 0;  
} 
//产生ACK应答
void IIC_Ack(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=0;
	delay_us(10);
	IIC_SCL=1;
	delay_us(10);
	IIC_SCL=0;
}
//不产生ACK应答		    
void IIC_NAck(void)
{
	IIC_SCL=0;
	SDA_OUT();
	IIC_SDA=1;
	delay_us(10);
	IIC_SCL=1;
	delay_us(10);
	IIC_SCL=0;
}					 				     
//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC_Send_Byte(uint8_t txd)
{                        
	uint8_t t;   
	SDA_OUT(); 	    
	IIC_SCL=0;//拉低时钟开始数据传输
	for(t=0;t<8;t++)
	{              
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1; 	  
		delay_us(10);   //对TEA5767这三个延时都是必须的
		IIC_SCL=1;
		delay_us(10); 
		IIC_SCL=0;	
		delay_us(10);
	}	
	IIC_Wait_Ack();
} 	    
//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA_IN();//SDA设置为输入
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0; 
        delay_us(10);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;   
		delay_us(10); 
    }					 
    if (!ack)
        IIC_NAck();//发送nACK
    else
        IIC_Ack(); //发送ACK   
    return receive;
}


