/**
  ******************************************************************************
  * @file    ano_buff.h
  * @author  Cheng Ziqiang
  * @version V0.1.0
  * @date    2021/7
  * @brief   sensor algorithm header file
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2020 CCIC</center></h2>
  *
  *
  ******************************************************************************
 */
#ifndef __ANO_BUFF_H
#define __ANO_BUFF_H
#include "main.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
extern UART_HandleTypeDef huart2;
#define BYTE0(dwTemp)		(*((char *)(&dwTemp)+0))
#define BYTE1(dwTemp)		(*((char *)(&dwTemp)+1))
#define BYTE2(dwTemp)		(*((char *)(&dwTemp)+2))
#define BYTE3(dwTemp)		(*((char *)(&dwTemp)+3))

//frame define
#define ANOHEAD   				0xAA
#define ANODADDR  				0xFF

//IMU frame
#define ANOIMUID					0x01
#define ANOIMULENTH  			(13)

#define ANOUSERID					0xF1
#define ANOUSERLENTH  		(12)

//user define frame
#define ANOIDF1   0xF1
#define ANOIDF2   0xF2
#define ANOIDF3   0xF3
#define ANOIDF4   0xF4
#define ANOIDF5   0xF5
#define ANOIDF6   0xF6
#define ANOIDF7   0xF7
#define ANOIDF8   0xF8
#define ANOIDF9   0xF9
#define ANOIDFA   0xFA

#define ANOF1LENTH  (6)//get two bytes in one axe

/* Private macro -------------------------------------------------------------*/


/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
#ifdef __GNUC__
  /* With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
     set to 'Yes') calls __io_putchar() */
  #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif /* __GNUC__ */
  
/* Private functions ---------------------------------------------------------*/


//void send_ANO_acc(int16_t *acc);
void send_ANO_IMU(int16_t *acc,int16_t *gyr,uint8_t *shock);
void ADXL355_Send_ANO(int16_t *accel);
	void ADXL355_Send_ANO_int32(int32_t *accel);
/**
  * @}
  */

/**
  * @}
  */
#endif
/************************ (C) COPYRIGHT CCIC *****END OF FILE****/
