#include "common.h"
/************************************************
逻辑摘要
    0.外部仅仅需要调用 ESP8266_init() 即可完成初始化，然后调用 ESP8266_send_http_data(Product_Id,Product_Location) 即可发送数据。
	1.

************************************************/

/*智能仓储用户配置参数，用于ESP8266向指定的网络端口发送数据所需要的参数*/
char *WIFI_SSID="wdy-6s";//要连接的WIFI的ID号
char *WIFI_PASSWORD="22223333";//要连接的WIFI的密码
char *TCP_IP="120.76.219.196";//建立TCP连接的IP的地址
char *TCP_PORT="85";//网络端口号
char *Product_Id="101";//货物的ID号
char *Product_Location="A";//货物的坐标

//usmart支持部分
//将收到的AT指令应答数据返回给电脑串口
//mode:0,不清零USART3_RX_STA;
//     1,清零USART3_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(USART3_RX_STA&0X8000)		//接收到一次数据了
	{
		printf("-----esp8266-----\n");
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		printf("%s",USART3_RX_BUF);	//发送到串口
		printf("====================end\n");
		if(mode)USART3_RX_STA=0;
	}
}
//ATK-ESP8266发送命令后,检测接收到的应答
//str:期待的应答结果
//返回值:0,没有得到期待的应答结果
//    其他,期待应答结果的位置(str的位置)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//接收到一次数据了
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//添加结束符
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	}
	return (u8*)strx;
}
//向ATK-ESP8266发送命令
//cmd:发送的命令字符串
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 

	USART3_RX_STA=0;
	// printf("%s\r\n", cmd);
	// delay_ms(10);
	u3_printf("%s\r\n",cmd);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//得到有效数据
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
}

//ATK-ESP8266退出透传模式
//返回值:0,退出成功;
//       1,退出失败
u8 atk_8266_quit_trans(void)
{
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';      
	delay_ms(15);					//大于串口组帧时间(10ms)
	while((USART3->SR&0X40)==0);	//等待发送空
	USART3->DR='+';      
	delay_ms(500);					//等待500ms
	return atk_8266_send_cmd("AT","OK",20);//退出透传判断.
}

//配置ESP8266在STA模式下要连接的WIFI
//ssid:要连接的WIFI的ID号
//password:要连接的WIFI的密码
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,连接成功(得到了期待的应答结果)
//       1,连接失败
u8 ESP8266_set_wifi(char *ssid,char *password,u8 *ack,u16 waittime)
{
	u8 res=0;
	USART3_RX_STA=0;
	u3_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);	//发送命令

	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000)//接收到期待的应答结果
			{
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//得到有效数据
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)	res=1;
	}
	return res;
}

//配置ESP8266与指定服务器建立TCP连接
//ip:建立TCP连接的IP的地址
//port:网络端口号
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,连接成功(得到了期待的应答结果)
//       1,连接失败
u8 ESP8266_set_TCP(char *ip,char *port,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	u3_printf("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",ip,port);	//发送命令
	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000){		//接收到期待的应答结果
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//得到有效数据
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)	res=1;
	}
	return res;
}

//配置ESP8266要发送的HTTP数据
//productId:货物的ID号
//location:货物的坐标
//ack:期待的应答结果,如果为空,则表示不需要等待应答
//waittime:等待时间(单位:10ms)
//返回值:0,发送成功(得到了期待的应答结果)
//       1,发送失败
u8 ESP8266_set_http_data(char *productId,char *location,u8 *ack,u16 waittime)
{
	u8 res=0;
	u8 contentLength=0;
	USART3_RX_STA=0;
	
	contentLength=strlen(productId)+strlen(location)+20;
	printf("send http data.\n");
	INTX_DISABLE();
	u3_printf("POST /test/testLocation HTTP/1.1\r\n\
HOST: 120.76.219.196:85\r\n\
Accept: text/*\r\n\
Accept-Language: zh-cn\r\n\
User-Agent: Mozilla/5.0(Windows;U;WindowsNT6.1;en-us)AppleWebKit/534.50(KHTML,likeGecko)Version/5.1Safari/534.50\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Content-Length:%d\r\n\
\r\n\
productId=%s&location=%s\r\n",
contentLength,productId,location);	//发送命令
	INTX_ENABLE();
// 	u3_printf("POST /test/testLocation HTTP/1.1\r\n\
// HOST: 120.76.219.196:85\r\n\
// Accept: text/*\r\n\
// Accept-Language: zh-cn\r\n\
// User-Agent: Mozilla/5.0(Windows;U;WindowsNT6.1;en-us)AppleWebKit/534.50(KHTML,likeGecko)Version/5.1Safari/534.50\r\n\
// Content-Type: application/x-www-form-urlencoded\r\n\
// Content-Length:%d\r\n\
// \r\n\
// productId=%s&location=%s\r\n",
// contentLength,productId,location);	//发送命令

	if(ack&&waittime)		//需要等待应答
	{
		while(--waittime)	//等待倒计时
		{
			delay_ms(20);
			if(atk_8266_check_cmd(ack))
			{
				printf("check ok!\n");
				atk_8266_at_response(1);
				break;//得到有效数据 
			}
			atk_8266_at_response(1);
		}
		if(waittime==0){
			res=1; 
			USART3_RX_STA=0;
		}
	}
	return res;
}

// //ATK-ESP8266模块测试主函数
// void atk_8266_test(void)
// {
	// while(atk_8266_send_cmd("AT","OK",20))//检查WIFI模块是否在线
	// {
	// 	atk_8266_quit_trans();//退出透传
	// 	atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式	
	// 	printf("esp8266 hasn't detected!\n");
	// 	delay_ms(800);
	// 	printf("try again...\n");
	// }
	// while(atk_8266_send_cmd("ATE0","OK",20));//关闭回显
	// delay_ms(10); 
	// atk_8266_at_response(1);

	// atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//设置WIFI STA模式
	// atk_8266_send_cmd("AT+RST","OK",20);		
	// for(i=0;i<4;i++)
	// 	delay_ms(1000);         //延时3S等待重启成功
	// sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WIFI_SSID,WIFI_PASSWORD);//设置无线参数:ssid,密码
	// while(atk_8266_send_cmd(p,"WIFI GOT IP",300));					//连接目标路由器,并且获得IP
	// delay_ms(1000);

	// atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0：单连接，1：多连接


	// sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",TCP_IP,TCP_PORT);    //配置目标TCP服务器
	// while(atk_8266_send_cmd(p,"OK",200)){
	// 	printf("connect TCP failed!\n");
	// 	if(!atk_8266_send_cmd(p,"ALREADY CONNECTED",20))
	// 		atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//关闭TCP
	// 	printf("try TCP again\n");
	// }
	// atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //传输模式为：透传
	// USART3_RX_STA=0;
	// while(1)
	// {
	// 	atk_8266_quit_trans();
	// 	atk_8266_send_cmd("AT+CIPSEND","OK",20);         //开始透传

		// //发送HTTP协议的数据,方法为POST
		// while(ESP8266_set_http_data(Product_Id,Product_Location,"HTTP/1.1 200 OK",100)){
		// 	printf("send failed!\n");
		// 	printf("USART3_RX_STA=%x\n", USART3_RX_STA);
		// 	delay_ms(1000);
		// }
// 		printf("send http success!\n");
// 		t++;
// 		delay_ms(1000);
// 		LED0=!LED0;
// 		atk_8266_at_response(1);
// 		printf("1s passed\n");
// 	}
// }

//ESP8266初始化
void ESP8266_init(void)
{
	uart3_init(115200);//串口3初始化,波特率115200;用于单片机与ESP8266的数据传输
	while(atk_8266_send_cmd("AT","OK",20))//检查WIFI模块是否在线
	{
		atk_8266_quit_trans();//退出透传
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式
		printf("esp8266 hasn't detected!\n");
		delay_ms(800);
		printf("try AT again...\n");
	}
	atk_8266_at_response(1);
	while(atk_8266_send_cmd("AT+CWMODE=1","OK",50));		//设置WIFI STA模式	
	while(atk_8266_send_cmd("AT+RST","OK",20));				//模块复位
	delay_ms(1000);		//延时1s,等待复位
	atk_8266_at_response(1);
	while(ESP8266_set_wifi(WIFI_SSID,WIFI_PASSWORD,"OK",300))	//连接wifi_ssid
	{
		printf("connect wifi failed!\n");
		delay_ms(100);
		printf("try connect wifi again...\n");
	}
	delay_ms(1000);
	while(atk_8266_send_cmd("AT+CIPMUX=0","OK",100));		//0：单连接，1：多连接
}

//ESP8266向指定的网络端口发送数据
void ESP8266_send_http_data(char *productId,char *location)
{
	while(ESP8266_set_TCP(TCP_IP,TCP_PORT,"CONNECT",200)){
		printf("connect TCP failed!\n");
		atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//关闭TCP
		delay_ms(100);
		printf("try TCP again\n");
	}
	while(atk_8266_send_cmd("AT+CIPMODE=1","OK",50));	//传输模式为：透传
	while(atk_8266_send_cmd("AT+CIPSEND",">",50));		//开始透传
	while(ESP8266_set_http_data(productId,location,"HTTP/1.1 200 OK",100)){
		printf("send http data failed!\n");
		delay_ms(500);
		printf("send http data try again...\n");
	}
	atk_8266_at_response(1);
	printf("send http success!\n");	
	while(atk_8266_quit_trans());	//退出透传模式
	atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //关闭透传模式
	atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//关闭TCP
	atk_8266_at_response(1);
}
