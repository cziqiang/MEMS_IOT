/*
 *****************************************************************************
 * @file:    ADXL355.c
 * @brief:   ADXL355 accelerometer IC
 * @Author:  Cheng Ziqiang
 * @version: $V1.0$
 * @date:    $2021/7/11$
 *-----------------------------------------------------------------------------
 * Copyright (c) 2021 CCIC, Inc. All rights reserved.
*/

/***************************** Include Files **********************************/

#include "adxl355.h"
#include "spi.h"
#include <stdio.h>
#include "ano_buff.h"
#include "nb_bc20.h"
/****************************** Global Data ***********************************/

int32_t  volatile i32SensorX;
int32_t  volatile i32SensorY;
int32_t  volatile i32SensorZ;
int32_t  volatile i32SensorT;
uint32_t volatile ui32SensorX;
uint32_t volatile ui32SensorY;
uint32_t volatile ui32SensorZ;
uint32_t volatile ui32SensorT;

////float volatile accel[3];
//int16_t accel_ANO[3];

volatile uint32_t ui32timer_counter = 0;

float volatile f32temp = 0.0f;
float adxl355Scale = 0.0f;

uint8_t ADXL355Rxbuff[ADXL355_REGRWMAX];
uint8_t ADXL355Txbuff[ADXL355_REGRWMAX];

uint8_t ADXL_ScanFlag = 0;

/************************* Global scope functions *****************************/
HAL_StatusTypeDef ADXL355_SPI_ReadID(void)
{
	HAL_StatusTypeDef status;
	uint8_t device_id[3];
	/* Read the Part ID register */
	status = ADXL355_Read_Register(DEVID_AD, &device_id[0],SPI_RW_ONE_REG);	
	status |= ADXL355_Read_Register(DEVID_MST, &device_id[1],SPI_RW_ONE_REG);
	status |= ADXL355_Read_Register(PARTID, &device_id[2],SPI_RW_ONE_REG); 
	/* Print the Part ID register */
	if((device_id[0]!=0xAD)||(device_id[1]!=0x1D)||(device_id[2]!=0xED))
	{
		MSG_USART(MSG_LEVEL_ERROR,"Read ADXL355 Device ID failed!");
		MSG_USART(MSG_LEVEL_INFO,"The DEVID_AD  = 0x%2X", device_id[0]);
		MSG_USART(MSG_LEVEL_INFO,"The DEVID_MST  = 0x%2X", device_id[1]);
		MSG_USART(MSG_LEVEL_INFO,"The PARTID  = 0x%2X", device_id[2]);
		return HAL_ERROR;
	}
	return status;
}

uint8_t SIMUL_SPI_TransmitReceive(uint8_t txData)
{
	uint8_t i,rxData;
	
	for(i=0;i<8;i++)
	{
		SPI_SCK = 0;
		delay_us(1);
		if(txData & 0x80){
				SPI_MOSI = 1;
		}else{
				SPI_MOSI = 0;
		}
		txData <<= 1;
		delay_us(1);

		SPI_SCK = 1;
		delay_us(1);
		rxData <<= 1;
		if(SPI_MISO){
				rxData |= 0x01;
		}
		delay_us(1);
	}
	SPI_SCK = 0;
	
	return rxData;
}

HAL_StatusTypeDef ADXL355_SPI_RW_Register(uint8_t *txBuf, uint8_t * rxBuf, uint16_t btNum)
{
	HAL_StatusTypeDef status;
	uint8_t i;
	
	ADXL355_CS = 0;
#ifndef USE_SIMULATE_SPI
	status = HAL_SPI_TransmitReceive(&hspi1,txbuffer,rxbuffer,rlen,SPITIMEOUT);
#else
	for(i=0;i<btNum;i++)
	{
		rxBuf[i] = SIMUL_SPI_TransmitReceive(txBuf[i]);
	}
	status = HAL_OK;
#endif
	ADXL355_CS = 1;
	
	return status;
}

HAL_StatusTypeDef ADXL355_Write_Register(uint8_t reg, uint8_t * wbuffer, enRegsNum wlen)
{
	HAL_StatusTypeDef status;
	uint16_t i,spiTransLen = wlen + 1;
	uint8_t txBuff[ADXL355_REGRWMAX],rxBuff[ADXL355_REGRWMAX];
	if(spiTransLen > ADXL355_REGRWMAX)
	{
		MSG_USART(MSG_LEVEL_ERROR,"SPI sent out of limit number of bytes.");
		return HAL_ERROR;
	}
	txBuff[0] = ((reg<<1)|ADXL355_WRITE);
	for(i=1;i<spiTransLen;i++)
	{
		txBuff[i] = *(wbuffer+i-1);
	}
	status = ADXL355_SPI_RW_Register(txBuff, rxBuff, spiTransLen);
	if(status == HAL_OK)
	{
		return HAL_OK;
	}
	return status;
}

HAL_StatusTypeDef ADXL355_Read_Register(uint8_t reg, uint8_t * rbuffer, enRegsNum rlen)
{
	HAL_StatusTypeDef status;

	uint16_t i,spiTransLen = rlen +1;
	uint8_t txBuff[ADXL355_REGRWMAX],rxBuff[ADXL355_REGRWMAX];
	
	if(spiTransLen > ADXL355_REGRWMAX)
	{
		MSG_USART(MSG_LEVEL_ERROR,"SPI sent out of limit number of bytes.");
		return HAL_ERROR;
	}
	txBuff[0] = ((reg<<1)|ADXL355_READ);
	for(i=1;i<spiTransLen;i++)
	{
		txBuff[i] = ADXL355_DUMMYBYTE;
	}
	status = ADXL355_SPI_RW_Register(txBuff, rxBuff, spiTransLen);
	if(status == HAL_OK)
	{
		memcpy(rbuffer,&rxBuff[1],rlen);
		return HAL_OK;
	}
		
	return status;
}
/**
   @brief Set sensor range within RANGE register
   @return none
**/
HAL_StatusTypeDef ADXL355_Set_Range(enRange range)
{
	HAL_StatusTypeDef status;
	if(range == ADXL_RANGE_2)
	{
		ADXL355Txbuff[0] = 0x81;
		status = ADXL355_Write_Register(RANGE,&ADXL355Txbuff[0],SPI_RW_ONE_REG);
		adxl355Scale = 256000.0f;
	}
	else if(range == ADXL_RANGE_4)
	{
		ADXL355Txbuff[0] = 0x82;
		status = ADXL355_Write_Register(RANGE,&ADXL355Txbuff[0],SPI_RW_ONE_REG);
		adxl355Scale = 128000.0f;
	}
	else if(range == ADXL_RANGE_4)
	{
		ADXL355Txbuff[0] = 0x83;
		status = ADXL355_Write_Register(RANGE,&ADXL355Txbuff[0],SPI_RW_ONE_REG);
		adxl355Scale = 64000.0f;
	}
	
	return status;
}

/**
   @brief Initialization the accelerometer sensor
   @return none
**/
HAL_StatusTypeDef ADXL355_Init(void)
{
	HAL_StatusTypeDef status;
	status = ADXL355_Set_Range(ADXL_RANGE_2);
	status |= ADXL355_SPI_ReadID();
	status |= ADXL355_Start_Sensor();
	
	if(status != HAL_OK)
		MSG_USART(MSG_LEVEL_ERROR,"ADXL355 Initial failed.");
	else
	{
		MSG_USART(MSG_LEVEL_INFO,"ADXL355 Initial successful.");
	}
	return status;
}

/**
   @brief Turns on accelerometer measurement mode.
   @return none

**/
HAL_StatusTypeDef ADXL355_Start_Sensor(void)
{
	HAL_StatusTypeDef status;

	status = ADXL355_Read_Register(POWER_CTL,ADXL355Rxbuff,SPI_RW_ONE_REG); /*Read POWER_CTL register, before modifying it */
	ADXL355Txbuff[0] = ADXL355Rxbuff[0] & 0xFE;
	status = ADXL355_Write_Register(POWER_CTL,ADXL355Txbuff,SPI_RW_ONE_REG);
	if(status != HAL_OK)
		MSG_USART(MSG_LEVEL_ERROR,"ADXL355 start sensor failed!");
	
	return status;
}

/**
   @brief Puts the accelerometer into standby mode.

   @return none

**/
HAL_StatusTypeDef ADXL355_Stop_Sensor(void)
{
	HAL_StatusTypeDef status;
	
	status = ADXL355_Read_Register(POWER_CTL,ADXL355Rxbuff,SPI_RW_ONE_REG); /*Read POWER_CTL register, before modifying it */
	ADXL355Txbuff[0] = ADXL355Rxbuff[0] | 0x01;	/* Clear measurement bit in POWER_CTL register */
	status = ADXL355_Write_Register(POWER_CTL,ADXL355Txbuff,SPI_RW_ONE_REG);	 /* Write the new value to POWER_CTL register */
	if(status != HAL_OK)
		MSG_USART(MSG_LEVEL_INFO,"ADXL355 stop sensor failed!");
	
	return status;
}

/**
   @brief Convert the two's complement data in X,Y,Z registers to signed integers
   @param ui32SensorData - raw data from register
   @return int32_t - signed integer data

**/
int32_t ADXL355_Acceleration_Data_Conversion (uint32_t ui32SensorData)
{
   int32_t volatile i32Conversion = 0;
   ui32SensorData = (ui32SensorData  >> 4);
   ui32SensorData = (ui32SensorData & 0x000FFFFF);

   if((ui32SensorData & 0x00080000)  == 0x00080000){
         i32Conversion = (ui32SensorData | 0xFFF00000);
   }
   else{
         i32Conversion = ui32SensorData;
   }
   return i32Conversion;
}

HAL_StatusTypeDef ADXL355_ReadData(uint8_t regType,volatile uint32_t *ui32Result,enRegsNum regLen)
{
	HAL_StatusTypeDef status;
	
	uint32_t ui32valueL = 0;
	uint32_t ui32valueM __attribute__((unused)) = 0;
	uint32_t ui32valueH = 0;
	
	status = ADXL355_Read_Register(regType,ADXL355Rxbuff,regLen); 
	
	if(regLen == SPI_RW_ONE_REG)
	{
		*ui32Result = ADXL355Rxbuff[0];
	}
	else if(regLen == SPI_RW_TWO_REG)
	{
		ui32valueH = ADXL355Rxbuff[0];
		ui32valueL = ADXL355Rxbuff[1];
		*ui32Result = ((ui32valueH << 8)|ui32valueL);
	}
	else if(regLen == SPI_RW_THREE_REG)
	{
		ui32valueH = ADXL355Rxbuff[0];
		ui32valueM = ADXL355Rxbuff[1];
		ui32valueL = ADXL355Rxbuff[2];
		*ui32Result = ((ui32valueH << 16)|(ui32valueM << 8)|ui32valueL);
	}
	
	return status;
}

/**
   @brief Reads the accelerometer data.
   @return none
**/
HAL_StatusTypeDef ADXL355_Data_Scan(void)
{
	HAL_StatusTypeDef status;
	float volatile accel[3]={0};
//	int32_t accel_ANO[3]={0};

//	if(ADXL_ScanFlag == 1)
//	{
//		ADXL_ScanFlag = 0;
//		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
		
		status = ADXL355_ReadData(XDATA3,&ui32SensorX,SPI_RW_THREE_REG); 
		status = ADXL355_ReadData(YDATA3,&ui32SensorY,SPI_RW_THREE_REG); 
		status = ADXL355_ReadData(ZDATA3,&ui32SensorZ,SPI_RW_THREE_REG); 
//		status = ADXL355_ReadData(TEMP2,&ui32SensorT,SPI_RW_TWO_REG); 

//		MSG_USART(MSG_LEVEL_INFO,"X acceleration data1 : %ld ", ui32SensorX);				 /* Print the X-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Y acceleration data1 : %ld ", ui32SensorY);        /* Print the Y-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Z acceleration data1 : %ld ", ui32SensorZ);        /* Print the Z-axis data */
		
		i32SensorX = ADXL355_Acceleration_Data_Conversion(ui32SensorX);
		i32SensorY = ADXL355_Acceleration_Data_Conversion(ui32SensorY);
		i32SensorZ = ADXL355_Acceleration_Data_Conversion(ui32SensorZ);
//		f32temp = ((((double)ui32SensorT - ADXL355_TEMP_BIAS)) / ADXL355_TEMP_SLOPE) + 25.0;
		
//		MSG_USART(MSG_LEVEL_INFO,"X acceleration data2 : %ld ", i32SensorX);				 /* Print the X-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Y acceleration data2 : %ld ", i32SensorY);        /* Print the Y-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Z acceleration data2 : %ld ", i32SensorZ);        /* Print the Z-axis data */
		
		accel[0] = (float)i32SensorX / adxl355Scale;
		accel[1] = (float)i32SensorY / adxl355Scale;
		accel[2] = (float)i32SensorZ / adxl355Scale;
		
		NB_PostFor355(accel);
		
//		HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_1);
		
//		MSG_USART(MSG_LEVEL_INFO,"X acceleration X : %.8f ", accel[0]);				/* Print the X-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Y acceleration Y : %.8f ", accel[1]);        /* Print the Y-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Z acceleration Z : %.8f ", accel[2]);        /* Print the Z-axis data */
		
		
//		for(uint8_t i=0;i<3;i++)
//			accel_ANO[i] = accel[i]*10000000;

//		ADXL355_Send_ANO(accel_ANO);
//		ADXL355_Send_ANO_int32(accel_ANO);
		
//		MSG_USART(MSG_LEVEL_INFO,"ANO acceleration X : %d ", accel_ANO[0]);				/* Print the X-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"ANO acceleration Y : %d ", accel_ANO[1]);        /* Print the Y-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"ANO acceleration Z : %d ", accel_ANO[2]);        /* Print the Z-axis data */
	
		
//		MSG_USART(MSG_LEVEL_INFO,"X acceleration data3 [G]: %.8f [g]", (float)i32SensorX / adxl355Scale);				/* Print the X-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Y acceleration data3 [G]: %.8f [g]", (float)i32SensorY / adxl355Scale);        /* Print the Y-axis data */
//		MSG_USART(MSG_LEVEL_INFO,"Z acceleration data3 [G]: %.8f [g]", (float)i32SensorZ / adxl355Scale);        /* Print the Z-axis data */
//	//	MSG_USART(MSG_LEVEL_INFO,"The temperature data is: %.2f [C]", f32temp);                                 /* Print the Temperature data */
//		MSG_USART(MSG_LEVEL_INFO,"----------------------------------------");
	
	return status;
}





