/*
 *****************************************************************************
 * @file:    flash.c
 * @brief:   Internal flash read or write function
 * @Author:  Cheng Ziqiang
 * @version: $V1.0$
 * @date:    $2021/8/6$
 *-----------------------------------------------------------------------------
 * Copyright (c) 2021 CCIC, Inc. All rights reserved.
*/

/***************************** Include Files **********************************/

#include "flash.h"

/**
   @brief Read the MCU internal flash(32bit).
   @return data from the specified address

**/
uint32_t Read_InternalFlash(uint32_t addr)
{
	return *(__IO uint32_t*)addr; 
}

/**
   @brief Gets the sector in which the address space resides.
   @return Sector address

**/
uint8_t Get_FlashSector(uint32_t addr)
{
	if(addr<ADDR_FLASH_SECTOR_1)return FLASH_SECTOR_0;
	else if(addr<ADDR_FLASH_SECTOR_2)return FLASH_SECTOR_1;
	else if(addr<ADDR_FLASH_SECTOR_3)return FLASH_SECTOR_2;
	else if(addr<ADDR_FLASH_SECTOR_4)return FLASH_SECTOR_3;
	else if(addr<ADDR_FLASH_SECTOR_5)return FLASH_SECTOR_4;
	else if(addr<ADDR_FLASH_SECTOR_6)return FLASH_SECTOR_5;
	else if(addr<ADDR_FLASH_SECTOR_7)return FLASH_SECTOR_6;
	return FLASH_SECTOR_7;
}

/**
   @brief Writes a specified amount of data from a specified address.
   @return HAL_StatusTypeDef

**/
HAL_StatusTypeDef WriteData_InternalFlash(uint32_t addr,uint32_t *pBuffer,uint32_t wNum)
{
	HAL_StatusTypeDef status = HAL_OK;
	FLASH_EraseInitTypeDef FlashEraseInit;
	
	uint32_t SectorError=0;
	uint32_t addr_x = addr;				//д�����ʼ��ַ
	uint32_t end_addr = addr+wNum*4;	//д��Ľ�����ַ
	
	if((addr_x<GD32_FLASH_BASE)||(end_addr>(uint32_t)0X1FFFC00F)||(addr_x%4))return HAL_ERROR;	//�Ƿ���ַ
	HAL_FLASH_Unlock();             //����	

	while(addr_x<end_addr)		//ɨ��һ���ϰ�.(�Է�FFFFFFFF�ĵط�,�Ȳ���)
	{
		if(Read_InternalFlash(addr_x) != 0xFFFFFFFFU)//�з�0XFFFFFFFF�ĵط�,Ҫ�����������
		{   
			FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;       //�������ͣ��������� 
			FlashEraseInit.Sector = Get_FlashSector(addr_x);   //Ҫ����������
			FlashEraseInit.NbSectors = 1;                             //һ��ֻ����һ������
			FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      //��ѹ��Χ��VCC=2.7~3.6V֮��!!
			status |= HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError);
			if(status!=HAL_OK) 
			{
				MSG_USART(MSG_LEVEL_ERROR,"Erase sector %d error!",SectorError);
				break;//����������	
			}
		}
		else addr_x+=4;
		status |= FLASH_WaitForLastOperation(FLASH_WAITETIME);                //�ȴ��ϴβ������
	}
	
	if(status == HAL_OK)
	{
		 while(addr<end_addr)//д����
		 {
			 status |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr,*pBuffer);
			 if(status!=HAL_OK)//д������
			 { 
				 MSG_USART(MSG_LEVEL_ERROR,"Write flash error!");
				 break;	//д���쳣
			 }
			 addr += 4;
			 pBuffer++;
		 }  
	}
	HAL_FLASH_Lock();           //����
	
	return status;
}

/**
   @brief Reads a specified amount of data from a specified address.
   @return none

**/
void ReadData_InternalFlash(uint32_t rAddr,uint32_t *pBuffer,uint32_t rNum)
{
	uint32_t i;
	for(i=0;i<rNum;i++)
	{
		pBuffer[i] = Read_InternalFlash(rAddr);//��ȡ4���ֽ�.
		rAddr += 4;//ƫ��4���ֽ�.	
	}
}

void RW_InternalFlash_Test(void)
{
	uint32_t data32 = 0x00000000;
	uint32_t DATA32 = 0x00000000;
	uint32_t memoryError = 0;
	uint32_t addr = FLASH_USER_START_ADDR;
	while(addr < FLASH_USER_END_ADDR)
	{
		WriteData_InternalFlash(addr,&data32,1);
		data32++;
		addr += 4;
	}
	addr = FLASH_USER_START_ADDR;
	while(addr < FLASH_USER_END_ADDR)
	{
		data32 = *(__IO uint32_t*)addr;
		if(data32 != DATA32)	memoryError++;
		DATA32++;
		addr += 4;
	}
	if(memoryError)
		MSG_USART(MSG_LEVEL_ERROR,"Write and read internal flash failed! memoryError:%d",memoryError);
	else
		MSG_USART(MSG_LEVEL_INFO,"Write and read internal flash succesed.");
}

void EraseFlash_UserArea(void)
{
	HAL_StatusTypeDef status;
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError=0;
	/* FLASH ���� ********************************/
	/* ʹ�ܷ���FLASH���ƼĴ��� */
	HAL_FLASH_Unlock();
	/* �����û����� (�û�����ָ������û��ʹ�õĿռ䣬�����Զ���)**/
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;/* �ԡ��֡��Ĵ�С���в��� */ 
	EraseInitStruct.Sector        = Get_FlashSector(FLASH_USER_START_ADDR);
	EraseInitStruct.NbSectors     = Get_FlashSector(FLASH_USER_END_ADDR)-EraseInitStruct.Sector+1;	
	status = HAL_FLASHEx_Erase(&EraseInitStruct,&SectorError);
	if(status!=HAL_OK) 
		MSG_USART(MSG_LEVEL_ERROR,"Erase sector %d error!",SectorError);
	
	/* ��FLASH��������ֹ���ݱ��۸�*/
	HAL_FLASH_Lock(); 
	
	uint32_t memoryError = 0,DATA32 = 0;
	uint32_t addr = FLASH_USER_START_ADDR;
	while(addr < FLASH_USER_END_ADDR)
	{
		DATA32 = *(__IO uint32_t*)addr;
		if(DATA32 != 0xFFFFFFFFU)
			memoryError++;
		addr+=4;
	}
	if(memoryError)
		MSG_USART(MSG_LEVEL_ERROR,"Erase the user area in internal flash is failed! memoryError:%d",memoryError);
	else
		MSG_USART(MSG_LEVEL_INFO,"Erase the user area in internal flash succesed.");
}

