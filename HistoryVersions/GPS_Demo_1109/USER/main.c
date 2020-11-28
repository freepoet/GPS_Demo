#include "sys.h"
#include "delay.h"  
#include "usart.h"  
#include "led.h"
#include "tftlcd.h"
#include "usmart.h"	
#include "usart3.h" 	
#include "key.h" 	 
#include "string.h"	 	 
#include "gps.h"	 
#include "math.h"
#include "timer.h"
#include "stdlib.h"
/****************************************Copyright (c)****************************************************
 
**--------------File Info---------------------------------------------------------------------------------
** File name:          main.c
** Last modified Date: 2020/11/11
** Created date:       2020/10/19    
** Version:            V1.0
** Descriptions:       Null
**--------------------------------------------------------------------------------------------------------*/
/*				  	 
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//串口1,发送缓存区
nmea_msg gpsx; 											//GPS信息
__align(4) u8 dtbuf[50];   								//打印缓存器
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode字符串 
*/
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//串口1,发送缓存区
nmea_msg gpsx; 											//GPS信息
u8 key=0XFF;
u8 flag_draw=0;//绘制轨迹标志位
u8 flag_demo=0;//演示标志位

u32 xt,yt,delta_x,delta_y,longitude1,latitude1,longitude2,latitude2;//gpsx.longitude latitude是u32类型的 要统一

int main(void)
{ 
	u16 i,rxlen;
	u16 lenx;
	struct pos
	{
		u32 x;
		u32 y;
		u32 n;
	};
	//u8 upload=0;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);      	//初始化延时函数
	uart_init(9600);			//初始化串口波特率为9600 
	usart3_init(9600);			//初始化串口3波特率为9600
	usmart_dev.init(84); 		//初始化USMART		
	LED_Init();					//初始化LED
	KEY_Init();					//初始化按键
 	TFTLCD_Init();			 			//初始化LCD
	usmart_dev.init(72); 		//初始化USMART 	 
	TIM3_Init(100-1,8400-1);  //定时100ms	   通用定时器中断TIMER3
	Interface_Display();
	if(Ublox_Cfg_Rate(1000,1)!=0)	//设置定位信息更新速度为1000ms,顺便判断GPS模块是否在位. 
	{
   	LCD_ShowString(40,160,200,24,24,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//持续判断,直到可以检查到NEO-6M,且数据保存成功
		{
			usart3_init(9600);				//初始化串口3波特率为9600(EEPROM没有保存数据的时候,波特率为9600.)
	  	Ublox_Cfg_Prt(9600);			//重新设置模块的波特率为9600
			usart3_init(9600);				//初始化串口3波特率为9600
			Ublox_Cfg_Tp(1000000,100000,1);	//设置PPS为1秒钟输出1次,脉冲宽度为100ms	    
			key=Ublox_Cfg_Cfg_Save();		//保存配置  
		}	  					 
	   LCD_ShowString(40,160,200,24,24,"NEO-6M Set Done!!");
		delay_ms(500);
	   LCD_Fill(0,160,30+200,160+24,WHITE);//清除显示 
	}

	xt=tftlcd_data.width/2;
	yt=tftlcd_data.height/2;
	while(1) 
	{	
		u16 Res=2;//分辨率  米
			u16 K;//比例系数
		//u16 n=0;
		static u32 t=0;
		/*
		u32* x = (u32 *) malloc ( sizeof(u32) * 300 );
		u32* y = (u32 *) malloc ( sizeof(u32) * 300 );
		*/
    //timer.c 里的TIM3_IRQHandler()作为定时器中断服务子函数
		K=5*1.0/Res;
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//接收到一次数据了
		{
			rxlen=USART3_RX_STA&0X7FFF;	//得到数据长度
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//启动下一次接收
			USART1_TX_BUF[i]=0;			//自动添加结束符
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//分析字符串
			if(flag_draw)
			{
				/*
				if((n%10)==0)
				{
           x=gpsx.longitude;
					 y=gpsx.latitude;
				}//间隔1s记录当前位置坐标
				n++;	
				*/
				//绘制轨迹
			 // LCD_DrawFRONT_COLOR(xt,yt,RED);
				LCD_Draw_Circle(xt,yt,1);
		    longitude2=gpsx.longitude;
		    latitude2=gpsx.latitude;
				delta_x=-1*(longitude2-longitude1)*K;
				delta_y=(latitude2-latitude1)*K;
				xt=xt+delta_x;
				yt=yt+delta_y;
				longitude1=longitude2;
				latitude1=latitude2; 
        //draw_message_show()			
			}
			else
			{
				//show_chinese(30,140,�",RED,WHITE);  //YELLOW
				Gps_Msg_Show();				//显示信息
			}
			
			//if(!flag_demo) Gps_Msg_Show();				//显示信息
			//if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//发送接收到的数据到串口1
 		}
		//接受数据成功则D1闪烁
		if((lenx%100)==0)LED0=!LED0; 	    				 
		lenx++;	
	}
}
	
/*
		if(key==KEY0_PRES)
		{
			upload=!upload;
			FRONT_COLOR=RED;
			if(upload)LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:ON ");
			else LCD_ShowString(30,100,200,16,16,"NMEA Data Upload:OFF");
 		}
	*/	
								    


