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
	uint32_t addr_x = addr;				//写入的起始地址
	uint32_t end_addr = addr+wNum*4;	//写入的结束地址
	
	if((addr_x<GD32_FLASH_BASE)||(end_addr>(uint32_t)0X1FFFC00F)||(addr_x%4))return HAL_ERROR;	//非法地址
	HAL_FLASH_Unlock();             //解锁	

	while(addr_x<end_addr)		//扫清一切障碍.(对非FFFFFFFF的地方,先擦除)
	{
		if(Read_InternalFlash(addr_x) != 0xFFFFFFFFU)//有非0XFFFFFFFF的地方,要擦除这个扇区
		{   
			FlashEraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;       //擦除类型，扇区擦除 
			FlashEraseInit.Sector = Get_FlashSector(addr_x);   //要擦除的扇区
			FlashEraseInit.NbSectors = 1;                             //一次只擦除一个扇区
			FlashEraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;      //电压范围，VCC=2.7~3.6V之间!!
			status |= HAL_FLASHEx_Erase(&FlashEraseInit,&SectorError);
			if(status!=HAL_OK) 
			{
				MSG_USART(MSG_LEVEL_ERROR,"Erase sector %d error!",SectorError);
				break;//发生错误了	
			}
		}
		else addr_x+=4;
		status |= FLASH_WaitForLastOperation(FLASH_WAITETIME);                //等待上次操作完成
	}
	
	if(status == HAL_OK)
	{
		 while(addr<end_addr)//写数据
		 {
			 status |= HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,addr,*pBuffer);
			 if(status!=HAL_OK)//写入数据
			 { 
				 MSG_USART(MSG_LEVEL_ERROR,"Write flash error!");
				 break;	//写入异常
			 }
			 addr += 4;
			 pBuffer++;
		 }  
	}
	HAL_FLASH_Lock();           //上锁
	
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
		pBuffer[i] = Read_InternalFlash(rAddr);//读取4个字节.
		rAddr += 4;//偏移4个字节.	
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
	/* FLASH 解锁 ********************************/
	/* 使能访问FLASH控制寄存器 */
	HAL_FLASH_Unlock();
	/* 擦除用户区域 (用户区域指程序本身没有使用的空间，可以自定义)**/
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;/* 以“字”的大小进行操作 */ 
	EraseInitStruct.Sector        = Get_FlashSector(FLASH_USER_START_ADDR);
	EraseInitStruct.NbSectors     = Get_FlashSector(FLASH_USER_END_ADDR)-EraseInitStruct.Sector+1;	
	status = HAL_FLASHEx_Erase(&EraseInitStruct,&SectorError);
	if(status!=HAL_OK) 
		MSG_USART(MSG_LEVEL_ERROR,"Erase sector %d error!",SectorError);
	
	/* 给FLASH上锁，防止内容被篡改*/
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

