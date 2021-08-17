#ifndef __PCF8563_H
#define __PCF8563_H

#include "main.h" 

//////////////////////////////////////////////////////////////////////////////////	 

//PCF8563 Register definitely
#define PCF8563_REG_CONSTA1 			0x00
#define PCF8563_REG_CONSTA2 			0x01
#define PCF8563_REG_SECOND 				0x02
#define PCF8563_REG_MINUTE 				0x03
#define PCF8563_REG_HOURS 				0x04
#define PCF8563_REG_DAY 					0x05
#define PCF8563_REG_WEEK 					0x06
#define PCF8563_REG_MONTH 				0x07
#define PCF8563_REG_YEAR 					0x08
#define PCF8563_REG_MINUTE_ALARM 	0x09
#define PCF8563_REG_HOURS_ALARM 	0x0A
#define PCF8563_REG_DAY_ALARM 		0x0B
#define PCF8563_REG_WEEK_ALARM 		0x0C
#define PCF8563_REG_CLKOUT 				0x0D
#define PCF8563_REG_TIMER_CTR 		0x0E
#define PCF8563_REG_TIMER 				0x0F

//Device address
#define PCF8563_WRITE_ADDR 	0xA2
#define PCF8563_READ_ADDR 	0xA3

//CLK Out
#define PCF8563_CLKOUT_32768HZ 	0
#define PCF8563_CLKOUT_1024HZ 	1
#define PCF8563_CLKOUT_32HZ 		2
#define PCF8563_CLKOUT_1HZ 			3
////////////////////////////////////////////////////////////////////////////////// 	

typedef struct 
{
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day;
	uint8_t week;
	uint8_t month;
	uint8_t year;
	uint8_t century;
}RTC_HandlerType;

uint32_t PCF8563_Get_TimeStamp(RTC_HandlerType rtc_timer);

void PCF8563_SetTimer(uint8_t year,uint8_t month,uint8_t date,uint8_t week,uint8_t hour,uint8_t min,uint8_t sec);
void PCF8563_GetTimer(void);
void PCF8563_WriteOneReg(uint8_t regType, uint8_t data);
void PCF8563_WriteReg(uint8_t regType, uint8_t *pData, uint8_t regNum);
void PCF8563_ReadOneReg(uint8_t regType, uint8_t *pData);
void PCF8563_ReadReg(uint8_t regType, uint8_t *pData, uint8_t regNum);
static uint8_t DECtoBCD(uint8_t DEC);
static uint8_t BCDtoDEC(uint8_t BCD);

//IO��������
#define SDA_IN()  {GPIOC->MODER&=~(3<<(0*2));GPIOC->MODER|=0<<0*2;}	//PC0����ģʽ
#define SDA_OUT() {GPIOC->MODER&=~(3<<(0*2));GPIOC->MODER|=1<<0*2;} //PC0���ģʽ
//IO��������	 
#define IIC_SCL    PCout(1) //SCL
#define IIC_SDA    PCout(0) //SDA	 
#define READ_SDA   PCin(0)  //����SDA 

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��				 
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

void IIC_Write_One_Byte(uint8_t daddr,uint8_t addr,uint8_t data);
uint8_t IIC_Read_One_Byte(uint8_t daddr,uint8_t addr);	 

void PCF8563_Init(void);

#endif
