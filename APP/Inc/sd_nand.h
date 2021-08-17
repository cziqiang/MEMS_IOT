/*
 *****************************************************************************
 * @file:    ad_nand.h
 * @brief:   CSNP4GCR01-AMW SD-NAND IC
 * @Author:  Cheng Ziqiang
 * @version: $V1.0$
 * @date:    $2021/7/11$
 *-----------------------------------------------------------------------------
 * Copyright (c) 2021 CCIC, Inc. All rights reserved.
*/

#ifndef __SD_NAND_H
#define __SD_NAND_H

#include "main.h"

#define SD_CS0			PBout(7)
#define	SD_CS1 			PBout(6)  		//ADXL355的片选信号
#define SD_SCK    	PBout(5)
#define SD_MOSI			PBout(3)
#define SD_MISO			PBin(4)

//SD传输数据结束后是否释放总线宏定义  
#define NO_RELEASE      0
#define RELEASE         1		 
// SD卡类型定义  
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4	   
// SD卡指令表  	   
#define CMD0    0       //卡复位
#define CMD1    1
#define CMD9    9       //命令9 ，读CSD数据
#define CMD10   10      //命令10，读CID数据
#define CMD12   12      //命令12，停止数据传输
#define CMD16   16      //命令16，设置SectorSize 应返回0x00
#define CMD17   17      //命令17，读sector
#define CMD18   18      //命令18，读Multi sector
#define ACMD23  23      //命令23，设置多sector写入前预先擦除N个block
#define CMD24   24      //命令24，写sector
#define CMD25   25      //命令25，写Multi sector
#define ACMD41  41      //命令41，应返回0x00
#define CMD55   55      //命令55，应返回0x01
#define CMD58   58      //命令58，读OCR信息
#define CMD59   59      //命令59，使能/禁止CRC，应返回0x00
//数据写入回应字意义
#define MSD_DATA_OK                0x05
#define MSD_DATA_CRC_ERROR         0x0B
#define MSD_DATA_WRITE_ERROR       0x0D
#define MSD_DATA_OTHER_ERROR       0xFF
//SD卡回应标记字
#define MSD_RESPONSE_NO_ERROR      0x00
#define MSD_IN_IDLE_STATE          0x01
#define MSD_ERASE_RESET            0x02
#define MSD_ILLEGAL_COMMAND        0x04
#define MSD_COM_CRC_ERROR          0x08
#define MSD_ERASE_SEQUENCE_ERROR   0x10
#define MSD_ADDRESS_ERROR          0x20
#define MSD_PARAMETER_ERROR        0x40
#define MSD_RESPONSE_FAILURE       0xFF

// SD卡类型定义  
#define SD_TYPE_MMC     0
#define SD_TYPE_V1      1
#define SD_TYPE_V2      2
#define SD_TYPE_V2HC    4	

uint8_t SD_Init(void);
uint8_t SDTest(void);

#endif
