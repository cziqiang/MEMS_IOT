/*****************************************************************************
* @file:   APP/Src/nb_bc20.c
* @author: Cheng Ziqiang
* @Email:  ziqiang.cheng@qq.com
* @date:   M07/Y21
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/

#include "nb_bc20.h"
#include "usart.h"

//cmdFlag = 0---PC send;cmdFlag = 1---MCU send;
volatile uint8_t cmdFlag;
volatile uint16_t nbAckCount;
volatile char cmpStr[256];
extern volatile uint8_t SECONE;

BC20_Status NB_BC20_Init(void)
{
	BC20_Status status;
	MSG_USART(MSG_LEVEL_INFO,"Wait for BC20 to initialize...");
	NB_USART("AT");
	HAL_Delay(3000);
	for(uint8_t i=0;i<0x0F;i++)
	{
		status = NB_PostMSG("+CGATT: 1",CMD_CGATT);
		if(status==NB_OK)
		{
			MSG_USART(MSG_LEVEL_INFO,"BC20 has initialized.");
			return status;
		}
		HAL_Delay(1000);
	}
	MSG_USART(MSG_LEVEL_INFO,"BC20 initial failed.");
	return status;
}

/**
  * @brief NB message print by uart and compare it to the expected results
  * @retval None
  */
BC20_Status NB_PostMSG(const char * cmp, const char * str, ...)
{
	BC20_Status status;

	cmdFlag = 1;	//MCU send msg
	recFlag = 0;
	
	va_list ap;
	va_start(ap, str);
	uart_nb_printer(str, ap);
	va_end(ap);
	
	status = NB_CompareAck(cmp,10);
	
	cmdFlag = 0;	//PC send cmd
	
	return status;
}

BC20_Status NB_CompareAck(const char *str,uint16_t sec)
{
	BC20_Status status;
	uint16_t count = 0;
	cmdFlag = 1;	//MCU send msg
	strcpy((char *)cmpStr,str);
	MSG_USART(MSG_LEVEL_DEBUG,"The cmpStr is \"%s\"",cmpStr);
	SECONE=0;
	count = 0;
	while((!recFlag)&&(count<sec))
	{
		if(SECONE==1)
		{	SECONE = 0; count++;}
	}
	if(recFlag)
	{
		recFlag=0;
		status = NB_OK;
		MSG_USART(MSG_LEVEL_DEBUG,"Ack matching successful.");
		MSG_USART(MSG_LEVEL_DEBUG,"BC20 return \r\n%s",NB_Buffer);
		cmdFlag = 0;
		return status;
	}
	else
	{
		status = NB_TIMEOUT;
		MSG_USART(MSG_LEVEL_ERROR,"BC20 Ack Timeout.");
	}
	
	return status;
}

void NB_MQTT_Init(void)
{
	BC20_Status status;
	NB_USART("AT+QMTCLOSE=0");
	HAL_Delay(1000);
	NB_USART("AT+QMTDISC=0");
	HAL_Delay(1000);
	
	NB_PostMSG("OK","AT+QMTCFG=\"aliauth\",0,\"%s\",\"%s\",\"%s\"",ProductKey,DeviceName,DeviceSecret);
	NB_PostMSG("+QMTOPEN: 0,0","AT+QMTOPEN=0,\"%s.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883",ProductKey);

	status = NB_PostMSG("+QMTCONN: 0,0,0","AT+QMTCONN=0,\"%s\"",DeviceName);
	if(status == NB_OK)
	{
		MSG_USART(MSG_LEVEL_INFO,"MQTT client has opened.");
		MSG_USART(MSG_LEVEL_INFO,"BC20 MQTT initial successful.");
	}
	else
		MSG_USART(MSG_LEVEL_INFO,"BC20 MQTT intial failed.");

	NB_PostMSG("+QMTPUB: 0,0,0","AT+QMTPUB=0,0,0,0,\
	\"/sys/%s/%s/thing/event/property/post\",\
	\"{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\
	\"Acc_X\":{\"value\":0.1234567},\
	\"Acc_Y\":{\"value\":0.5832441},\
	\"Acc_Z\":{\"value\":0.2132421}},\
	\"method\":\"thing.event.property.post\"}\"",ProductKey,DeviceName);
	NB_PostMSG("+QMTPUB: 0,0,0","AT+QMTPUB=0,0,0,0,\"/sys/a1ksNkXCHQc/MEMS_IOT_02/thing/event/property/post\",\"{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\"Acc_X\":{\"value\":0.1234567},\"Acc_Y\":{\"value\":0.58324241}},\"method\":\"thing.event.property.post\"}\"");
//	NB_PostMSG("+QMTPUB: 0,0,0","AT+QMTPUB=0,0,0,0,\"/sys/a1ksNkXCHQc/MEMS_IOT_02/thing/event/property/post\",\"{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\"Acc_X\":{\"value\":0.1234567},\"Acc_Y\":{\"value\":0.58324241}},\"method\":\"thing.event.property.post\"}\"");

//	NB_USART("AT+QMTPUB=0,0,0,0,\"/sys/a1ksNkXCHQc/MEMS_IOT_02/thing/event/property/post\",\"{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\"Acc_X\":{\"value\":0.1234567},\"Acc_Y\":{\"value\":0.58324241}},\"method\":\"thing.event.property.post\"}\"");
}

BC20_Status NB_PostFor355(volatile float *acc)
{
	BC20_Status status;
	status = NB_PostMSG("+QMTPUB: 0,0,0","AT+QMTPUB=0,0,0,0,\
	\"/sys/%s/%s/thing/event/property/post\",\
	\"{\"id\":\"26\",\"version\":\"1.0\",\"params\":{\
	\"Acc_X\":{\"value\":%f},\
	\"Acc_Y\":{\"value\":%f},\
	\"Acc_Z\":{\"value\":%f}},\
	\"method\":\"thing.event.property.post\"}\"",ProductKey,DeviceName,acc[0],acc[1],acc[2]);
	return status;
}
