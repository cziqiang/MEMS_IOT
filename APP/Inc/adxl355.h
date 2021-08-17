#ifndef __ADXL355_H_
#define __ADXL355_H_

#include "main.h"

/********************************* Definitions ********************************/

/* ADXL355 registers addresses */
#define DEVID_AD                 0x00
#define DEVID_MST                0x01
#define PARTID                   0x02
#define REVID                    0x03
#define STATUS                   0x04
#define FIFO_ENTRIES             0x05
#define TEMP2                    0x06
#define TEMP1                    0x07
#define XDATA3                   0x08
#define XDATA2                   0x09
#define XDATA1                   0x0A
#define YDATA3                   0x0B
#define YDATA2                   0x0C
#define YDATA1                   0x0D
#define ZDATA3                   0x0E
#define ZDATA2                   0x0F
#define ZDATA1                   0x10
#define FIFO_DATA                0x11
#define OFFSET_X_H               0x1E
#define OFFSET_X_L               0x1F
#define OFFSET_Y_H               0x20
#define OFFSET_Y_L               0x21
#define OFFSET_Z_H               0x22
#define OFFSET_Z_L               0x23
#define ACT_EN                   0x24
#define ACT_THRESH_H             0x25
#define ACT_THRESH_L             0x26
#define ACT_COUNT                0x27
#define FILTER                   0x28
#define FIFO_SAMPLES             0x29
#define INT_MAP                  0x2A
#define SYNC                     0x2B
#define RANGE                    0x2C
#define POWER_CTL                0x2D
#define SELF_TEST                0x2E
#define ADXL_RESET               0x2F

#define ADXL355_REGRWMAX (4)//max read out
#define ADXL355_DUMMYBYTE  0xFF

/**************************** Configuration parameters **********************/

/* Temperature parameters */
#define ADXL355_TEMP_BIAS       (float)1852.0      /* Accelerometer temperature bias(in ADC codes) at 25 Deg C */
#define ADXL355_TEMP_SLOPE      (float)-9.05       /* Accelerometer temperature change from datasheet (LSB/degC) */

/* Accelerometer parameters */
//#define ADXL_RANGE     2     /* ADXL362 sensitivity: 2, 4, 8 [g] */

typedef enum {
	ADXL_RANGE_2 = 2,
	ADXL_RANGE_4 = 4,
	ADXL_RANGE_8 = 8
} enRange;

/*******************************************************************************
**************************** Internal types ************************************
********************************************************************************/
extern float volatile f32temp;
extern float adxl355Scale;
extern uint8_t ADXL_ScanFlag;

/*******************************************************************************
**************************** Internal types ************************************
********************************************************************************/

/* Write data mode */

typedef enum {
   SPI_RW_ONE_REG = 1,            /* Read one ACC register */
   SPI_RW_TWO_REG,                /* Read two ACC registers */
   SPI_RW_THREE_REG,              /* Read three ACC registers */

} enRegsNum;


/*******************************************************************************
**************************** Internal definitions ******************************
********************************************************************************/

/* Accelerometer write command */
#define ADXL355_WRITE         0x0

/* Accelerometer read command */
#define ADXL355_READ          0x1

/*******************************************************************************
**************************** Functions declarations *****************************
********************************************************************************/
HAL_StatusTypeDef ADXL355_Init(void);
HAL_StatusTypeDef ADXL355_Start_Sensor(void);
HAL_StatusTypeDef ADXL355_Data_Scan(void);

HAL_StatusTypeDef ADXL355_Set_Range(enRange range);
HAL_StatusTypeDef ADXL355_Write_Register(uint8_t reg, uint8_t * wbuffer, enRegsNum wlen);
HAL_StatusTypeDef ADXL355_Read_Register(uint8_t reg, uint8_t * rbuffer, enRegsNum rlen);
void ADXL355_Send_ANO(int16_t *accel);


#endif
