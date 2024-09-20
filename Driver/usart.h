
#ifndef USART_H
#define USART_H


#include "stdint.h"	
#include "stdio.h"
#define USART_REC_LEN  			20*1024 //240K字节APP运行代码



extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern uint16_t USART_RX_STA;         		//接收状态标记	
extern uint32_t USART_RX_CNT;				//接收的字节数	
extern uint8_t  CodeUpdateFlag;

void usart_config(void);
#endif

