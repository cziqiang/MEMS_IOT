/*****************************************************************************
* @file:   APP/Inc/msg_queue.h
* @author: Cheng Ziqiang
* @Email:  ziqiang.cheng@qq.com
* @date:   M07/Y21
* @brief:  msg_queue headfile 
******************************************************************************
* Revision History:
* Rev. Date         Who                 Changes
* 1   M07/Y21    Cheng Ziqiang       New Created.
*******************************************************************************/
#ifndef __MSG_QUEUE_H
#define __MSG_QUEUE_H 

#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#define NB_USART(...)				nb_formate(__VA_ARGS__)

#if defined(DEBUGMSG_DISABLE)
	#define MSG(level, ...)           (void)0
	#define MSG_SETUP(level, printer) (void)0
	#define MSG_SETUP_LEVEL(level)    (void)0
	#define MSG_LEVEL                 BB_MSG_LEVEL_OFF
#else
	#define MSG_USART(level, ...)           msg_formate(level, __VA_ARGS__)
 	#define MSG_SETUP(level, printer) msg_setup(level, printer)
	#define MSG_LEVEL_SETUP(level)    msg_level_setup(level)
	#define MSG_GET_LEVEL                 msg_get_level()
#endif

typedef void (*MSG_FUNCTION)(void);

typedef struct __msg_fun_st 
{ 
    const char *msg_type;//msg type 
    MSG_FUNCTION fun_ptr;//msg function point
}msg_fun_st;

typedef enum 
{
  MSG_OK       = 0x00U,
  MSG_ERROR    = 0x01U,
  MSG_BUSY     = 0x02U,
  MSG_TIMEOUT  = 0x03U
} MSG_StatusTypeDef;

enum msg_level {
	MSG_LEVEL_OFF = 0,
	MSG_LEVEL_NULL,
	MSG_LEVEL_ERROR,
	MSG_LEVEL_WARNING,
	MSG_LEVEL_INFO,
	MSG_LEVEL_VERBOSE,
	MSG_LEVEL_DEBUG,
	MSG_LEVEL_MAX
};

/*****************************************************************************
 *  NB Message Function
 *****************************************************************************/
//typedef void (*nb_printer_t)(const char * str, va_list ap);

void nb_formate(const char * str, ...);
void uart_nb_printer(const char * str, va_list ap);

/*****************************************************************************
 *  Debug Message Function
 *****************************************************************************/
typedef void (*msg_printer_t)(int level, const char * str, va_list ap);

void msg_setup(int level, msg_printer_t printer);
void msg_formate(int level, const char * str, ...);
int msg_get_level(void);
void msg_level_setup(int level);

void uart_debug_printer(int level, const char * str, va_list ap);

#endif
/*****END OF FILE****/	
