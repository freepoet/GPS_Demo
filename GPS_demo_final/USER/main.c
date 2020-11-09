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
** Last modified Date: 2020/11/07
** Created date:       2020/10/19    
** Version:            V1.0
** Descriptions:       Null
**--------------------------------------------------------------------------------------------------------*/
/*				  	 
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//´®¿Ú1,·¢ËÍ»º´æÇø
nmea_msg gpsx; 											//GPSĞÅÏ¢
__align(4) u8 dtbuf[50];   								//´òÓ¡»º´æÆ÷
const u8*fixmode_tbl[4]={"Fail","Fail"," 2D "," 3D "};	//fix mode×Ö·û´® 
*/
u8 USART1_TX_BUF[USART3_MAX_RECV_LEN]; 					//´®¿Ú1,·¢ËÍ»º´æÇø
nmea_msg gpsx; 											//GPSĞÅÏ¢
u8 key=0XFF;
u8 flag_draw=0;//»æÖÆ¹ì¼£±êÖ¾Î»
u8 flag_demo=0;//ÑİÊ¾±êÖ¾Î»

u32 xt,yt,delta_x,delta_y,longitude1,latitude1,longitude2,latitude2;//gpsx.longitude latitudeÊÇu32ÀàĞÍµÄ ÒªÍ³Ò»

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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//ÉèÖÃÏµÍ³ÖĞ¶ÏÓÅÏÈ¼¶·Ö×é2
	delay_init(168);      	//³õÊ¼»¯ÑÓÊ±º¯Êı
	uart_init(115200);			//³õÊ¼»¯´®¿Ú²¨ÌØÂÊÎª115200 
	usart3_init(38400);			//³õÊ¼»¯´®¿Ú3²¨ÌØÂÊÎª38400
	usmart_dev.init(84); 		//³õÊ¼»¯USMART		
	LED_Init();					//³õÊ¼»¯LED
	KEY_Init();					//³õÊ¼»¯°´¼ü
 	TFTLCD_Init();			 			//³õÊ¼»¯LCD
	usmart_dev.init(72); 		//³õÊ¼»¯USMART 	 
	TIM3_Init(100-1,8400-1);  //¶¨Ê±100ms	   Í¨ÓÃ¶¨Ê±Æ÷ÖĞ¶ÏTIMER3
	Interface_Display();
	if(Ublox_Cfg_Rate(1000,1)!=0)	//ÉèÖÃ¶¨Î»ĞÅÏ¢¸üĞÂËÙ¶ÈÎª1000ms,Ë³±ãÅĞ¶ÏGPSÄ£¿éÊÇ·ñÔÚÎ». 
	{
   	LCD_ShowString(40,160,200,24,24,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//³ÖĞøÅĞ¶Ï,Ö±µ½¿ÉÒÔ¼ì²éµ½NEO-6M,ÇÒÊı¾İ±£´æ³É¹¦
		{
			usart3_init(9600);				//³õÊ¼»¯´®¿Ú3²¨ÌØÂÊÎª9600(EEPROMÃ»ÓĞ±£´æÊı¾İµÄÊ±ºò,²¨ÌØÂÊÎª9600.)
	  	Ublox_Cfg_Prt(38400);			//ÖØĞÂÉèÖÃÄ£¿éµÄ²¨ÌØÂÊÎª38400
			usart3_init(38400);				//³õÊ¼»¯´®¿Ú3²¨ÌØÂÊÎª38400
			Ublox_Cfg_Tp(1000000,100000,1);	//ÉèÖÃPPSÎª1ÃëÖÓÊä³ö1´Î,Âö³å¿í¶ÈÎª100ms	    
			key=Ublox_Cfg_Cfg_Save();		//±£´æÅäÖÃ  
		}	  					 
	   LCD_ShowString(40,160,200,24,24,"NEO-6M Set Done!!");
		delay_ms(500);
	   LCD_Fill(0,160,30+200,160+24,WHITE);//Çå³ıÏÔÊ¾ 
	}

	xt=tftlcd_data.width/2;
	yt=tftlcd_data.height/2;
	while(1) 
	{	
		u16 Res=1;//·Ö±æÂÊ  Ã×
			u16 K;//±ÈÀıÏµÊı
		//u16 n=0;
		static u32 t=0;
		/*
		u32* x = (u32 *) malloc ( sizeof(u32) * 300 );
		u32* y = (u32 *) malloc ( sizeof(u32) * 300 );
		*/
    //timer.c ÀïµÄTIM3_IRQHandler()×÷Îª¶¨Ê±Æ÷ÖĞ¶Ï·şÎñ×Óº¯Êı
		K=5*1.0/Res;
		delay_ms(1);
		if(USART3_RX_STA&0X8000)		//½ÓÊÕµ½Ò»´ÎÊı¾İÁË
		{
			rxlen=USART3_RX_STA&0X7FFF;	//µÃµ½Êı¾İ³¤¶È
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//Æô¶¯ÏÂÒ»´Î½ÓÊÕ
			USART1_TX_BUF[i]=0;			//×Ô¶¯Ìí¼Ó½áÊø·û
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//·ÖÎö×Ö·û´®
			if(flag_draw)
			{
				/*
				if((n%10)==0)
				{
           x=gpsx.longitude;
					 y=gpsx.latitude;
				}//¼ä¸ô1s¼ÇÂ¼µ±Ç°Î»ÖÃ×ø±ê
				n++;	
				*/
				//»æÖÆ¹ì¼£
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
				//show_chinese(30,140,ı",RED,WHITE);  //YELLOW
				Gps_Msg_Show();				//ÏÔÊ¾ĞÅÏ¢
			}
			
			//if(!flag_demo) Gps_Msg_Show();				//ÏÔÊ¾ĞÅÏ¢
			//if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//·¢ËÍ½ÓÊÕµ½µÄÊı¾İµ½´®¿Ú1
 		}
		//½ÓÊÜÊı¾İ³É¹¦ÔòD1ÉÁË¸
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
								    


