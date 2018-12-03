#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "string.h"
#include "usart3.h" 

void ESP8266_init(void);
void ESP8266_send_http_data(char *productId,char *location);

#endif
