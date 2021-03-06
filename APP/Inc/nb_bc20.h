#ifndef __NB_BC20_H
#define __NB_BC20_H

#include "main.h"

#define CMD_TIMEOUT	10000			//max 1s

#define CMD_AT			"AT"
#define CMD_CGATT		"AT+CGATT?"			//+CGATT: 1

//Ali cloud three elements
#define ProductKey		"a1ksNkXCHQc"
#define DeviceName		"MEMS_IOT_02"
#define DeviceSecret	"cf33ccc31e8fa0485357d61d006cc752"

#define PubTopic         "/sys/a1ksNkXCHQc/MEMS_IOT_02/thing/event/property/post"
#define SubTopic        "/sys/a1ksNkXCHQc/MEMS_IOT_02/thing/service/property/set"

/* Shenzhen University Iot platform */
#define ProductModel		"BC26MEMS"
#define ProductSN				"BC26MEMS001"
#define ProductID				"526558"

#define ClientID				"BC26MEMS001@526558@sha1"
#define Username				"BC26MEMS001@526558@2021"
#define Password				"358141cd40a06ddc98047eac519c8819cf11f71b"


typedef enum 
{
  NB_OK       = 0x00U,
  NB_ERROR    = 0x01U,
  NB_BUSY     = 0x02U,
  NB_TIMEOUT  = 0x03U
} BC20_Status;

BC20_Status NB_BC20_Init(void);
BC20_Status NB_PostMSG(const char * cmp, const char * str, ...);
BC20_Status NB_CompareAck(const char *str,uint16_t sec);

void NB_MQTT_Init(void);

BC20_Status NB_PostFor355(volatile float *acc,uint64_t timestamp);
#endif
