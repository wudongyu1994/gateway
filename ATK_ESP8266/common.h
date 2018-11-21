#ifndef __COMMON_H__
#define __COMMON_H__	 
#include "sys.h"
#include "usart.h"		
#include "delay.h"	
#include "led.h"
#include "string.h"
#include "usart3.h" 
/////////////////////////////////////////////////////////////////////////////////////////////////////////// 
void ESP8266_init(void);
void ESP8266_send_http_data(char *productId,char *location);

//用户配置参数
extern const u8* portnum;			//连接端口
 
extern const u8* wifista_ssid;		//WIFI STA SSID
extern const u8* wifista_encryption;//WIFI STA 加密方式
extern const u8* wifista_password; 	//WIFI STA 密码

extern const u8* wifiap_ssid;		//WIFI AP SSID
extern const u8* wifiap_encryption;	//WIFI AP 加密方式
extern const u8* wifiap_password; 	//WIFI AP 密码

extern u8* kbd_fn_tbl[2];
extern const u8* ATK_ESP8266_CWMODE_TBL[3];
extern const u8* ATK_ESP8266_WORKMODE_TBL[3];
extern const u8* ATK_ESP8266_ECN_TBL[5];
#endif
