/**--------------File Info---------------------------------------------------------------------------------
** File name:          main.c
** Last modified Date: 2020/11/25
** Created date:       2020/11/24   
** Version:            V1.0
** Descriptions:       Null
**--------------------------------------------------------------------------------------------------------*/	
/**********************************************************
 
	接线说明：
	STM32					MC20
	3.3V	------>	V_IO
	GND		<----->	GND
	PA3		<------	GSM_TX
	PA2		------>	GSM_RX
	

	//用于调试可不接
	STM32					USB-TTL模块
	GND		------>	GND
	TX1/PA9		------>	RXD
***********************************************************/

#include "sys.h"
#include "delay.h"
#include "usart.h"
#include "led.h"
#include "usart3.h"
#include "tftlcd.h"
//常量
#define Success 1U
#define Failure 0U

//定义变量
unsigned long  Time_Cont = 0;       //定时器计数器
////
	char send_buf[400] = {0};
	char text[100] = {0};
	char tmp[25] = {0};
	char lon_str_end[15] = {0};
	char lat_str_end[15] = {0};
	

	char lon_int_str[10]= {0};
	char lon_dec_str[10]= {0};
	
	long lon_int = 0;
	long lon_dec = 0;
	long lon_int_mm = 0;
	long lon_int_dd = 0;

	char lat_int_str[10]= {0};
	char lat_dec_str[10]= {0};
	long lat_int = 0;
	long lat_dec = 0;
	long lat_int_mm = 0;
	long lat_int_dd = 0;
/////


char OneNetServer[] = "183.230.40.33";       //不需要修改


char device_id[] = "651451568";    //修改为自己的设备ID
char API_KEY[] = "iywrUgMS2HcAoYTXouILUrbGJKI=";    //修改为自己的API_KEY
char sensor_gps[] = "location";				//不需要修改

unsigned int count = 0;

void errorLog(int num);
void phone(char *number);
unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry);
void sendMessage(char *number,char *msg);
void Sys_Soft_Reset(void);
void parseGpsBuffer(void);
void printGpsBuffer(void);
void postGpsDataToOneNet(char* API_VALUE_temp, char* device_id_temp, char* sensor_id_temp, char* lon_temp, char* lat_temp);

int main(void)
{	
	delay_init(168);
	
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(9600);	 //串口初始化为9600
	usart3_init(9600);	//串口2波特率9600
	TFTLCD_Init();			 			//初始化LCD
	u3_printf("AT\r\n");
	//Init_LEDpin();

	printf("Welcome to use!\r\n");
	Interface_Display();
	LCD_ShowString(40,110,300,24,24,"Initializing..."); 
	if (sendCommand("AT\r\n", "OK\r\n", 3000, 10) == Success);
	else errorLog(1);
	delay_ms(10);

	if (sendCommand("AT+CPIN?\r\n", "READY", 3000, 10) == Success);
	else errorLog(3);
	delay_ms(10);

	if (sendCommand("AT+CREG?\r\n", ",1", 3000, 10) == Success);	//本地SIM卡
	else if(sendCommand("AT+CREG?\r\n", ",5", 3000, 10) == Success	);//漫游SIM卡
	else	errorLog(3);
	delay_ms(10);

	if (sendCommand("AT+QGNSSC?\r\n", "+QGNSSC: 1", 5000, 1) == Success);
	else if (sendCommand("AT+QGNSSC=1\r\n", "OK\r\n", 5000, 10) == Success);
	else errorLog(5);

	clrStruct();
	LCD_Fill(40,110,320,110+24,WHITE);//清除显示 
	LCD_ShowString(40,110,300,24,24,"Initialized OK!");   
	delay_ms(100);
	
	while(1)
	{
		if (sendCommand("AT+QGNSSRD=\"NMEA/RMC\"\r\n", "OK\r\n", 2000, 10) == Success);
		else errorLog(6);

		Save_Data.isGetData = true;
		memset(Save_Data.GPS_Buffer, 0, GPS_Buffer_Length);      //清空
		memcpy(Save_Data.GPS_Buffer, USART3_RX_BUF, point2);
		
		printf("\r\n==========Save_Data.GPS_Buffer==========\r\n");
		printf(Save_Data.GPS_Buffer); //输出接收到的信息
		printf("\r\n");
		parseGpsBuffer();
		printGpsBuffer();
		
	//	delay_ms(10);
	//	Gps_Msg_Show();
	}
	
}

void parseGpsBuffer()
{
	char *subString;
	char *subStringNext;
	char i = 0;
	if (Save_Data.isGetData)
	{
		Save_Data.isGetData = false;
		printf("**************\r\n");
		printf(Save_Data.GPS_Buffer);

		
		for (i = 0 ; i <= 6 ; i++)
		{
			if (i == 0)
			{
				if ((subString = strstr(Save_Data.GPS_Buffer, ",")) == NULL)
					errorLog(1);	//解析错误
			}
			else
			{
				subString++;
				if ((subStringNext = strstr(subString, ",")) != NULL)
				{
					char usefullBuffer[2]; 
					switch(i)
					{
						case 1:memcpy(Save_Data.UTCTime, subString, subStringNext - subString);break;	//获取UTC时间
						case 2:memcpy(usefullBuffer, subString, subStringNext - subString);break;	//获取UTC时间
						case 3:memcpy(Save_Data.latitude, subString, subStringNext - subString);break;	//获取纬度信息
						case 4:memcpy(Save_Data.N_S, subString, subStringNext - subString);break;	//获取N/S
						case 5:memcpy(Save_Data.longitude, subString, subStringNext - subString);break;	//获取经度信息
						case 6:memcpy(Save_Data.E_W, subString, subStringNext - subString);break;	//获取E/W

						default:break;
					}

					subString = subStringNext;
					Save_Data.isParseData = true;
					if(usefullBuffer[0] == 'A')
						Save_Data.isUsefull = true;
					else if(usefullBuffer[0] == 'V')
						Save_Data.isUsefull = false;

				}
				else
				{
					errorLog(2);	//解析错误
				}
			}


		}
	}
}

void printGpsBuffer()
{
	if (Save_Data.isParseData)
	{
		Save_Data.isParseData = false;
		
		printf("Save_Data.UTCTime = ");
		printf(Save_Data.UTCTime);
		printf("\r\n");

		if(Save_Data.isUsefull)
		{
			Save_Data.isUsefull = false;
			printf("Save_Data.latitude = ");
			printf(Save_Data.latitude);
			printf("\r\n");


			printf("Save_Data.N_S = ");
			printf(Save_Data.N_S);
			printf("\r\n");

			printf("Save_Data.longitude = ");
			printf(Save_Data.longitude);
			printf("\r\n");

			printf("Save_Data.E_W = ");
			printf(Save_Data.E_W);
			printf("\r\n");
			
			
			postGpsDataToOneNet(API_KEY, device_id, sensor_gps, Save_Data.longitude, Save_Data.latitude);		//发送数据到Onenet
			
			LED1 = 0;
			delay_ms(100);
			LED1 = 1;
		}
		else
		{
			printf("GPS DATA is not usefull!\r\n");
		}
		
	}
}



void postGpsDataToOneNet(char* API_VALUE_temp, char* device_id_temp, char* sensor_id_temp, char* lon_temp, char* lat_temp)
{

	
	int i = 0;
	char sendCom[2] = {0x1A,0x00};

	//换算经度
	sscanf(lon_temp,"%ld.%ld",&lon_int,&lon_dec);
	
	sprintf(lon_dec_str,"%ld",lon_dec);		//获取小数字符串
	sprintf(lon_int_str,"%ld",lon_int);		//获取整数字符串
	
	printf("\r\nlon_int_str:%s,lon_dec_str:%s",lon_int_str,lon_dec_str);
	//获取分的整数，然后放大 小数个数 的倍数
	lon_int_mm = lon_int%100;
	for(i = 0 ; i <strlen(lon_dec_str) ; i++)
	{
		lon_int_mm *= 10;
	}
	//整合分
	lon_int_mm += lon_dec;
	//获取度
	lon_int_dd = lon_int/100;
	//转换分为度,小数末尾做四舍五入
	lon_int_mm = (float)lon_int_mm/60 + 0.5;
	sprintf(lon_str_end,"%ld.%ld",lon_int_dd,lon_int_mm);

	//换算纬度
	sscanf(lat_temp,"%ld.%ld",&lat_int,&lat_dec);
	sprintf(lat_int_str,"%ld",lat_int);
	sprintf(lat_dec_str,"%ld",lat_dec);
	
	printf("\r\nlat_int_str:%s,lat_dec_str:%s",lat_int_str,lat_dec_str);
	//获取分的整数，然后放大 小数个数 的倍数
	lat_int_mm = lat_int%100;
	for(i = 0 ; i <strlen(lat_dec_str) ; i++)
	{
		lat_int_mm *= 10;
	}
	//整合分
	lat_int_mm += lat_dec;
	//获取度
	lat_int_dd = lat_int/100;
	//转换分为度,小数末尾做四舍五入
	lat_int_mm = (float)lat_int_mm/60 + 0.5;
	sprintf(lat_str_end,"%ld.%ld",lat_int_dd,lat_int_mm);

	//连接服务器
	memset(send_buf, 0, 400);    //清空
	strcpy(send_buf, "AT+QIOPEN=\"TCP\",\"");
	strcat(send_buf, OneNetServer);
	strcat(send_buf, "\",\"80\"\r\n");
	if (sendCommand(send_buf, "CONNECT", 10000, 3) == Success);
	else errorLog(7);

	//发送数据
	if (sendCommand("AT+QISEND\r\n", ">", 3000, 1) == Success);
	else errorLog(8);

	memset(send_buf, 0, 400);    //清空

	/*准备JSON串*/
	//ARDUINO平台不支持sprintf的double的打印，只能转换到字符串然后打印
	sprintf(text, "{\"datastreams\":[{\"id\":\"%s\",\"datapoints\":[{\"value\":{\"lon\":%s,\"lat\":%s}}]}]}"
	        , sensor_id_temp, lon_str_end, lat_str_end);

	/*准备HTTP报头*/
	send_buf[0] = 0;
	strcat(send_buf, "POST /devices/");
	strcat(send_buf, device_id_temp);
	strcat(send_buf, "/datapoints HTTP/1.1\r\n"); //注意后面必须加上\r\n
	strcat(send_buf, "api-key:");
	strcat(send_buf, API_VALUE_temp);
	strcat(send_buf, "\r\n");
	strcat(send_buf, "Host:");
	strcat(send_buf, OneNetServer);
	strcat(send_buf, "\r\n");
	sprintf(tmp, "Content-Length:%d\r\n\r\n", strlen(text)); //计算JSON串长度
	strcat(send_buf, tmp);
	strcat(send_buf, text);

	if (sendCommand(send_buf, send_buf, 3000, 1) == Success);
	else errorLog(9);

	
	if (sendCommand(sendCom, "\"succ\"}", 3000, 1) == Success);
	else errorLog(10);

	delay_ms(100);
	if (sendCommand("AT+QICLOSE\r\n", "OK\r\n", 3000, 1) == Success);
	else errorLog(11);
	delay_ms(100);

	if (sendCommand("AT+QIDEACT\r\n", "OK\r\n", 3000, 1) == Success);
	else errorLog(11);
	delay_ms(100);

}


void sendMessage(char *number,char *msg)
{
	char send_buf[20] = {0};
	memset(send_buf, 0, 20);    //清空
	strcpy(send_buf, "AT+CMGS=\"");
	strcat(send_buf, number);
	strcat(send_buf, "\"\r\n");
	if (sendCommand(send_buf, ">", 3000, 10) == Success);
	else errorLog(6);


	if (sendCommand(msg, msg, 3000, 1) == Success);
	else errorLog(7);
	delay_ms(100);

	memset(send_buf, 0, 100);    //清空
	send_buf[0] = 0x1a;
	if (sendCommand(send_buf, "OK\r\n", 10000, 5) == Success);
	else errorLog(8);
	delay_ms(100);
}

void errorLog(int num)
{
	printf("ERROR%d\r\n",num);
	while (1)
	{
		if (sendCommand("AT\r\n", "OK", 100, 10) == Success)
		{
			Sys_Soft_Reset();
		}
		delay_ms(200);
	}
}

void Sys_Soft_Reset(void)
{  
    SCB->AIRCR =0X05FA0000|(u32)0x04;      
}

void phone(char *number)
{
	char send_buf[20] = {0};
	memset(send_buf, 0, 20);    //清空
	strcpy(send_buf, "ATD");
	strcat(send_buf, number);
	strcat(send_buf, ";\r\n");

	if (sendCommand(send_buf, "SOUNDER", 10000, 10) == Success);
	else errorLog(4);
}

unsigned int sendCommand(char *Command, char *Response, unsigned long Timeout, unsigned char Retry)
{
	unsigned char n;
	USART3_CLR_Buf();
	for (n = 0; n < Retry; n++)
	{
		u3_printf(Command); 		//发送GPRS指令
		
		printf("\r\n***************send****************\r\n");
		printf((char *)Command);
		
		Time_Cont = 0;
		while (Time_Cont < Timeout)
		{
			delay_ms(100);
			Time_Cont += 100;
			if (strstr((char *)USART3_RX_BUF, Response) != NULL)
			{				
				printf("\r\n***************receive****************\r\n");
				printf((char *)USART3_RX_BUF);
				return Success;
			}
			
		}
		Time_Cont = 0;
	}
	printf("\r\n***************receive****************\r\n");
	printf((char *)USART3_RX_BUF);
	USART3_CLR_Buf();
	return Failure;
}







