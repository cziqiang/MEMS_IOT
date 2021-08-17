/*****************************************************************************
* @file:   APP/Inc/msg_queue.c
* @author: Cheng Ziqiang
* @Email:  ziqiang.cheng@qq.com
* @date:   M07/Y21
* @brief:  msg_queue headfile 
******************************************************************************
* Revision History:
* Rev. Date         Who                 Changes
* 1   M07/Y21    Cheng Ziqiang       New Created.
*******************************************************************************/
/* Includes ------------------------------------------------------------------*/

#include "msg_queue.h"

static int msg_level;
static msg_printer_t msg_printer;
//static nb_printer_t nb_printer = uart_nb_printer;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

/** @addtogroup MSG_DEBUG_UART
  * @{
  */
/**
  * @brief  This function realized the debug printer 
  * 
  * @retval None
**/
void msg_setup(int level, msg_printer_t printer)
{
  msg_level   = level;
  if (level < MSG_LEVEL_OFF)
    msg_level = MSG_LEVEL_OFF;
  else if (level > MSG_LEVEL_MAX)
    msg_level = MSG_LEVEL_MAX;
  msg_printer = printer;
}

void msg_formate(int level, const char * str, ...)
{
  if(level && level <= msg_level && msg_printer) {
    va_list ap;
    va_start(ap, str);
    msg_printer(level, str, ap);
    va_end(ap);
  }
}

int msg_get_level(void)
{
  return msg_level;
}

void msg_level_setup(int level)
{
  msg_level   = level;
}


/**
  * @brief debug message print by uart
  * @retval None
  */
void uart_debug_printer(int level, const char * str, va_list ap)
{
  static char out_str[256]; /* static to limit stack usage */
  uint16_t idx = 0;
  const char * s[MSG_LEVEL_MAX] = {
    "",      // MSG_LEVEL_OFF
	  "",		   //	MSG_LEVEL_NULL
		"[E]: ", // MSG_LEVEL_ERROR
		"[W]: ", // MSG_LEVEL_WARNING
		"[I]: ", // MSG_LEVEL_INFO
		"[V]: ", // MSG_LEVEL_VERBOSE
		"[D]: ", // MSG_LEVEL_DEBUG
  };
  //put the while code before to change the out_str
  while(huart2.gState!=HAL_UART_STATE_READY); 
	
  idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "%s", s[level]);
  if(idx >= (sizeof(out_str)))
    return;
  idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
  if(idx >= (sizeof(out_str)))
    return;
  idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
  if(idx >= (sizeof(out_str)))
    return;
	
  while(HAL_UART_Transmit_DMA(&huart2,(uint8_t*)out_str,idx)!=HAL_OK);
}

void nb_formate(const char * str, ...)
{
    va_list ap;
    va_start(ap, str);
    uart_nb_printer(str, ap);
    va_end(ap);
}

/**
  * @brief NB message print by uart
  * @retval None
  */
void uart_nb_printer(const char * str, va_list ap)
{
	static char out_str[256]; /* static to limit stack usage */
  uint16_t idx = 0;
	//put the while code before to change the out_str
	
	idx += vsnprintf(&out_str[idx], sizeof(out_str) - idx, str, ap);
  if(idx >= (sizeof(out_str)))
    return;
	  idx += snprintf(&out_str[idx], sizeof(out_str) - idx, "\r\n");
  if(idx >= (sizeof(out_str)))
    return;
	
	while(HAL_UART_Transmit_DMA(&huart1,(uint8_t*)out_str,idx)!=HAL_OK);
}

/**
* @}
*/

/*****END OF FILE****/
