#include "sys.h"
#include "led.h"
#include "delay.h"
#include "usart.h"
#include "usart2.h" 
#include "usart3.h"
#include "lora_app.h"
#include "common.h"

/********************************************************************
				  0			  1			  2			  3
				  
			 0  1  2  3  4  5  6  7  8  9 10 11  12 13 14 15
			 ___________ ___________ ___________ ___________
		0	|			|			|			|			|
  0		1	|	A		|	B		|		C 	|	  D 	|
		2	|			|			|			|			|
		3	|___________|___________|___________|___________|
		4	|			|			|			|			|		
  1		5	|	E		|	F		|		G 	|	  H 	|
		6	|			|			|			|			|
		7	|___________|___________|___________|___________|

********************************************************************/
char* locDefine[2][4]={	{"A","B","C","D"},
						{"E","F","G","H"}};

//主函数
int main(void)
{
    u8 temp,i;
	u8 data_rev[256];
            //      0,  1,   2,   3,   4
    u8 sendData[]={0x80,0x01,0x50,0x51,0x81};
    u8 len= sizeof(sendData)/sizeof(sendData[0]);
	char *Product_Id="101";//货物的ID号
	char *Product_Location="A";//货物的坐标

	delay_init();//延时初始化函数
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);//串口1初始化,波特率115200;用于观测ESP8266的运行状态
	LED_Init();
	ESP8266_init();//ESP8266向指定的网络端口发送数据
	lora_atk_init();

	// ESP8266_send_http_data(Product_Id,Product_Location);
	printf("start while(1)!\n");
	while(1){
		delay_ms(100);
		printf("USART2_RX_STA=%x\n", USART2_RX_STA);
		if(USART2_RX_STA&0X8000){
            printf("data receive= ");
            for(i=0;i<(USART2_RX_STA&0x7fff);i++){
                printf("%x ",USART2_RX_BUF[i]);
                data_rev[i]=USART2_RX_BUF[i];
            }
            printf("\n");
            if(data_rev[0]==0x80 && data_rev[1]==0x07 && data_rev[2]==0x11 && data_rev[10]==0x81){
                for(i=1,temp=0; i<9; i++)
                    temp+=data_rev[i];
                if(data_rev[9]==temp){
                    //code here:
                    //store key data                    
                    delay_ms(100);
                    LoRa_SendData(OBJ_ADDRH,OBJ_ADDRL,OBJ_CHN,sendData,len);
                    // printf("lora ack= %x %x %x %x %x\n", sendData[0], sendData[1], sendData[2], sendData[2], sendData[4]);
                    ESP8266_send_http_data(Product_Id,locDefine[data_rev[7]][data_rev[8]]);
                    // ESP8266_send_http_data(Product_Id,locDefine[data_rev[7]][data_rev[8]]);
                }
                memset(data_rev,0,256);
            }
			USART2_RX_STA=0;
		}
	}
}
