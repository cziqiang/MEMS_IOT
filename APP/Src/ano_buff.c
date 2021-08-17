/**
  ******************************************************************************
  * @file    ano_buff.c
  * @author  Cheng Ziqiang
  * @version V0.1.0
  * @date    2020/05
  * @brief   user frame implementation for ANO UPPer
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 CCIC</center></h2>
  *
  *
  ******************************************************************************
 */

 /* Includes ------------------------------------------------------------------*/
#include "ano_buff.h"
#include "usart.h"

/** @addtogroup 
  * @{
  */

/** @addtogroup 
  * @{
  */

void send_ANO_IMU(int16_t *acc,int16_t *gyr,uint8_t *shock)
{
	uint8_t DataFramBuffer[20];
  uint8_t index = 0;
  uint8_t sumcheck = 0;//sum check byte
  uint8_t addcheck = 0;//additional check byte
  int16_t accx,accy,accz;
  int16_t gyrx,gyry,gyrz;
  accx = acc[0];accy = acc[1];accz = acc[2];
  gyrx = gyr[0];gyry = gyr[1];gyrz = gyr[2];

  DataFramBuffer[index++] = ANOHEAD;
  DataFramBuffer[index++] = ANODADDR;
  DataFramBuffer[index++] = ANOIMUID;
  DataFramBuffer[index++] = ANOIMULENTH;
  DataFramBuffer[index++] = BYTE0(accx);
  DataFramBuffer[index++] = BYTE1(accx);
  DataFramBuffer[index++] = BYTE0(accy);
  DataFramBuffer[index++] = BYTE1(accy);
  DataFramBuffer[index++] = BYTE0(accz);
  DataFramBuffer[index++] = BYTE1(accz);
  DataFramBuffer[index++] = BYTE0(gyrx);
  DataFramBuffer[index++] = BYTE1(gyrx);
  DataFramBuffer[index++] = BYTE0(gyry);
  DataFramBuffer[index++] = BYTE1(gyry);
  DataFramBuffer[index++] = BYTE0(gyrz);
  DataFramBuffer[index++] = BYTE1(gyrz);
  DataFramBuffer[index++] = *shock;
  for(uint8_t i=0; i < (DataFramBuffer[3] + 4); i++) 
  {
    sumcheck += DataFramBuffer[i]; 
    addcheck += sumcheck; 
  }
  DataFramBuffer[index++] = sumcheck;
  DataFramBuffer[index++] = addcheck;
  while(HAL_UART_Transmit_DMA(USER_HUART,DataFramBuffer,index)!=HAL_OK);
}

/**
  * @}
  */

/**
   @brief Reads the accelerometer data.
   @return none
**/
void ADXL355_Send_ANO(int16_t *accel)
{
	uint8_t DataFramBuffer[20];
  uint8_t index = 0;
  uint8_t sumcheck = 0;//sum check byte
  uint8_t addcheck = 0;//additional check byte
  int16_t accx,accy,accz;	
	accx = accel[0];accy = accel[1];accz = accel[2];
	
  DataFramBuffer[index++] = ANOHEAD;
  DataFramBuffer[index++] = ANODADDR;
  DataFramBuffer[index++] = ANOIMUID;
  DataFramBuffer[index++] = ANOIMULENTH;
	DataFramBuffer[index++] = BYTE0(accx);
  DataFramBuffer[index++] = BYTE1(accx);
  DataFramBuffer[index++] = BYTE0(accy);
  DataFramBuffer[index++] = BYTE1(accy);
  DataFramBuffer[index++] = BYTE0(accz);
  DataFramBuffer[index++] = BYTE1(accz);
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	DataFramBuffer[index++] = 0;
	
	for(uint8_t i=0; i < (DataFramBuffer[3] + 4); i++) 
  {
    sumcheck += DataFramBuffer[i]; 
    addcheck += sumcheck; 
  }
  DataFramBuffer[index++] = sumcheck;
  DataFramBuffer[index++] = addcheck;
  while(HAL_UART_Transmit_DMA(USER_HUART,DataFramBuffer,index)!=HAL_OK);
}

void ADXL355_Send_ANO_int32(int32_t *accel)
{
	uint8_t DataFramBuffer[20]={0};
  uint8_t index = 0;
  uint8_t sumcheck = 0;//sum check byte
  uint8_t addcheck = 0;//additional check byte
  int32_t accx,accy,accz;	
	accx = accel[0];accy = accel[1];accz = accel[2];
	
//	while(huart2.gState!=HAL_UART_STATE_READY){}
	
  DataFramBuffer[index++] = ANOHEAD;
  DataFramBuffer[index++] = ANODADDR;
  DataFramBuffer[index++] = ANOUSERID;
  DataFramBuffer[index++] = ANOUSERLENTH;
	DataFramBuffer[index++] = BYTE0(accx);
  DataFramBuffer[index++] = BYTE1(accx);
	DataFramBuffer[index++] = BYTE2(accx);
  DataFramBuffer[index++] = BYTE3(accx);
  DataFramBuffer[index++] = BYTE0(accy);
  DataFramBuffer[index++] = BYTE1(accy);
  DataFramBuffer[index++] = BYTE2(accy);
  DataFramBuffer[index++] = BYTE3(accy);
  DataFramBuffer[index++] = BYTE0(accz);
  DataFramBuffer[index++] = BYTE1(accz);
  DataFramBuffer[index++] = BYTE2(accz);
  DataFramBuffer[index++] = BYTE3(accz);
	
	for(uint8_t i=0; i < (DataFramBuffer[3] + 4); i++) 
  {
    sumcheck += DataFramBuffer[i]; 
    addcheck += sumcheck; 
  }
  DataFramBuffer[index++] = sumcheck;
  DataFramBuffer[index++] = addcheck;
//  while(HAL_UART_Transmit_DMA(&huart2,DataFramBuffer,index)!=HAL_OK);
	HAL_UART_Transmit_DMA(USER_HUART,DataFramBuffer,index);
}

/**
  * @}
  */

/************************ (C) COPYRIGHT CCIC *****END OF FILE****/
