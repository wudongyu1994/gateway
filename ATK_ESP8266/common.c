#include "common.h"
/************************************************
�߼�ժҪ
    0.�ⲿ������Ҫ���� ESP8266_init() ������ɳ�ʼ����Ȼ����� ESP8266_send_http_data(Product_Id,Product_Location) ���ɷ������ݡ�
	1.

************************************************/

/*���ִܲ��û����ò���������ESP8266��ָ��������˿ڷ�����������Ҫ�Ĳ���*/
char *WIFI_SSID="wdy-6s";//Ҫ���ӵ�WIFI��ID��
char *WIFI_PASSWORD="22223333";//Ҫ���ӵ�WIFI������
char *TCP_IP="120.76.219.196";//����TCP���ӵ�IP�ĵ�ַ
char *TCP_PORT="85";//����˿ں�
char *Product_Id="101";//�����ID��
char *Product_Location="A";//���������

//usmart֧�ֲ���
//���յ���ATָ��Ӧ�����ݷ��ظ����Դ���
//mode:0,������USART3_RX_STA;
//     1,����USART3_RX_STA;
void atk_8266_at_response(u8 mode)
{
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{
		printf("-----esp8266-----\n");
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		printf("%s",USART3_RX_BUF);	//���͵�����
		printf("====================end\n");
		if(mode)USART3_RX_STA=0;
	}
}
//ATK-ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����
//    ����,�ڴ�Ӧ������λ��(str��λ��)
u8* atk_8266_check_cmd(u8 *str)
{
	
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	}
	return (u8*)strx;
}
//��ATK-ESP8266��������
//cmd:���͵������ַ���
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 atk_8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 

	USART3_RX_STA=0;
	// printf("%s\r\n", cmd);
	// delay_ms(10);
	u3_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//�õ���Ч����
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)res=1; 
	}
	return res;
}

//ATK-ESP8266�˳�͸��ģʽ
//����ֵ:0,�˳��ɹ�;
//       1,�˳�ʧ��
u8 atk_8266_quit_trans(void)
{
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';      
	delay_ms(15);					//���ڴ�����֡ʱ��(10ms)
	while((USART3->SR&0X40)==0);	//�ȴ����Ϳ�
	USART3->DR='+';      
	delay_ms(500);					//�ȴ�500ms
	return atk_8266_send_cmd("AT","OK",20);//�˳�͸���ж�.
}

//����ESP8266��STAģʽ��Ҫ���ӵ�WIFI
//ssid:Ҫ���ӵ�WIFI��ID��
//password:Ҫ���ӵ�WIFI������
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ӳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 ESP8266_set_wifi(char *ssid,char *password,u8 *ack,u16 waittime)
{
	u8 res=0;
	USART3_RX_STA=0;
	u3_printf("AT+CWJAP=\"%s\",\"%s\"\r\n",ssid,password);	//��������

	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//�õ���Ч����
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)	res=1;
	}
	return res;
}

//����ESP8266��ָ������������TCP����
//ip:����TCP���ӵ�IP�ĵ�ַ
//port:����˿ں�
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ӳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
u8 ESP8266_set_TCP(char *ip,char *port,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	u3_printf("AT+CIPSTART=\"TCP\",\"%s\",%s\r\n",ip,port);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(20);
			if(USART3_RX_STA&0X8000){		//���յ��ڴ���Ӧ����
				if(atk_8266_check_cmd(ack))
				{
					atk_8266_at_response(1);
					break;//�õ���Ч����
				}
				atk_8266_at_response(1);
			}
		}
		if(waittime==0)	res=1;
	}
	return res;
}

//����ESP8266Ҫ���͵�HTTP����
//productId:�����ID��
//location:���������
//ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��
//waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����)
//       1,����ʧ��
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
contentLength,productId,location);	//��������
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
// contentLength,productId,location);	//��������

	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			delay_ms(20);
			if(atk_8266_check_cmd(ack))
			{
				printf("check ok!\n");
				atk_8266_at_response(1);
				break;//�õ���Ч���� 
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

// //ATK-ESP8266ģ�����������
// void atk_8266_test(void)
// {
	// while(atk_8266_send_cmd("AT","OK",20))//���WIFIģ���Ƿ�����
	// {
	// 	atk_8266_quit_trans();//�˳�͸��
	// 	atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ	
	// 	printf("esp8266 hasn't detected!\n");
	// 	delay_ms(800);
	// 	printf("try again...\n");
	// }
	// while(atk_8266_send_cmd("ATE0","OK",20));//�رջ���
	// delay_ms(10); 
	// atk_8266_at_response(1);

	// atk_8266_send_cmd("AT+CWMODE=1","OK",50);		//����WIFI STAģʽ
	// atk_8266_send_cmd("AT+RST","OK",20);		
	// for(i=0;i<4;i++)
	// 	delay_ms(1000);         //��ʱ3S�ȴ������ɹ�
	// sprintf((char*)p,"AT+CWJAP=\"%s\",\"%s\"",WIFI_SSID,WIFI_PASSWORD);//�������߲���:ssid,����
	// while(atk_8266_send_cmd(p,"WIFI GOT IP",300));					//����Ŀ��·����,���һ��IP
	// delay_ms(1000);

	// atk_8266_send_cmd("AT+CIPMUX=0","OK",20);   //0�������ӣ�1��������


	// sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",TCP_IP,TCP_PORT);    //����Ŀ��TCP������
	// while(atk_8266_send_cmd(p,"OK",200)){
	// 	printf("connect TCP failed!\n");
	// 	if(!atk_8266_send_cmd(p,"ALREADY CONNECTED",20))
	// 		atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//�ر�TCP
	// 	printf("try TCP again\n");
	// }
	// atk_8266_send_cmd("AT+CIPMODE=1","OK",200);      //����ģʽΪ��͸��
	// USART3_RX_STA=0;
	// while(1)
	// {
	// 	atk_8266_quit_trans();
	// 	atk_8266_send_cmd("AT+CIPSEND","OK",20);         //��ʼ͸��

		// //����HTTPЭ�������,����ΪPOST
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

//ESP8266��ʼ��
void ESP8266_init(void)
{
	uart3_init(115200);//����3��ʼ��,������115200;���ڵ�Ƭ����ESP8266�����ݴ���
	while(atk_8266_send_cmd("AT","OK",20))//���WIFIģ���Ƿ�����
	{
		atk_8266_quit_trans();//�˳�͸��
		atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ
		printf("esp8266 hasn't detected!\n");
		delay_ms(800);
		printf("try AT again...\n");
	}
	atk_8266_at_response(1);
	while(atk_8266_send_cmd("AT+CWMODE=1","OK",50));		//����WIFI STAģʽ	
	while(atk_8266_send_cmd("AT+RST","OK",20));				//ģ�鸴λ
	delay_ms(1000);		//��ʱ1s,�ȴ���λ
	atk_8266_at_response(1);
	while(ESP8266_set_wifi(WIFI_SSID,WIFI_PASSWORD,"OK",300))	//����wifi_ssid
	{
		printf("connect wifi failed!\n");
		delay_ms(100);
		printf("try connect wifi again...\n");
	}
	delay_ms(1000);
	while(atk_8266_send_cmd("AT+CIPMUX=0","OK",100));		//0�������ӣ�1��������
}

//ESP8266��ָ��������˿ڷ�������
void ESP8266_send_http_data(char *productId,char *location)
{
	while(ESP8266_set_TCP(TCP_IP,TCP_PORT,"CONNECT",200)){
		printf("connect TCP failed!\n");
		atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//�ر�TCP
		delay_ms(100);
		printf("try TCP again\n");
	}
	while(atk_8266_send_cmd("AT+CIPMODE=1","OK",50));	//����ģʽΪ��͸��
	while(atk_8266_send_cmd("AT+CIPSEND",">",50));		//��ʼ͸��
	while(ESP8266_set_http_data(productId,location,"HTTP/1.1 200 OK",100)){
		printf("send http data failed!\n");
		delay_ms(500);
		printf("send http data try again...\n");
	}
	atk_8266_at_response(1);
	printf("send http success!\n");	
	while(atk_8266_quit_trans());	//�˳�͸��ģʽ
	atk_8266_send_cmd("AT+CIPMODE=0","OK",200);  //�ر�͸��ģʽ
	atk_8266_send_cmd("AT+CIPCLOSE","OK",100);	//�ر�TCP
	atk_8266_at_response(1);
}
