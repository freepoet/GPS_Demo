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


/****************************************Copyright (c)****************************************************
 
**--------------File Info---------------------------------------------------------------------------------
** File name:          main.c
** Last modified Date: 2020/10/31   Ôö¼Óºº×ÖÏÔÊ¾×÷ÕßĞÅÏ¢
                                    ÏÔÊ¾Ö®Ç°´æ´¢ºÃµÄÎ»ÖÃĞÅÏ¢
																		ĞŞ1020 1024ÀïµÄbug:USART3ÀïÌí¼ÓÁËwhile(1)º¯Êı£¬µ¼ÖÂvalid satelliteÒ»Ö±Îª0

											 2020/10/22    ¹ì¼£»æÖÆ£¨´ı²âÊÔ£©
											 2020/10/21    Ìí¼ÓÖĞ¶Ï·şÎñ×Óº¯ÊıÀ´¼ì²â°´¼ü×´Ì¬ ÓÃÓÚ¹¦ÄÜÇĞ»»                       
                       2020/10/20    ÊµÏÖ¶¨Î»
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
/*
//UTC×ª»»Îª¶«°ËÇø(±±¾©)Ê±¼ä
nmea_utc_time UTCToBeijing(u16 UTCyear,u8 UTCmonth,u8 UTCday,u8 UTChour,u8 UTCminute,u8 UTCsecond)
{
	u16 year=0;
	u8 month=0,day=0,hour=0;
    u16 lastday = 0;// 
    u16 lastlastday = 0;//
	
	  year=UTCyear;
		month=UTCmonth;
	  day=UTCday;
	  hour=UTChour+8;//UTC+8
	
	  if(month==1 || month==3 || month==5 || month==7 || month==8 || month==10 || month==12)
		{
        lastday = 31;
        if(month == 3)
				{
            if((year%400 == 0)||(year%4 == 0 && year%100 != 0))
                lastlastday = 29;
            else
                lastlastday = 28;
        }
        if(month == 8)
            lastlastday = 31;
    }
    else 
		if(month == 4 || month == 6 || month == 9 || month == 11)
		{
        lastday = 30;
        lastlastday = 31;
    }
    else
		{
        lastlastday = 31;
        if((year%400 == 0)||(year%4 == 0 && year%100 != 0))
            lastday = 29;
        else
            lastday = 28;
    }
		if(hour >= 24)
		{
				hour -= 24;
				day += 1; 
				if(day > lastday)
				{ 
						day -= lastday;
						month += 1;

						if(month > 12)
						{
								month -= 12;
								year += 1;
						}
				}
		}	
//		Uart1_SendStr(bjttbuf);	
//		Uart1_SendStr("\r\n");
}
*/


int main(void)
{ 
	u16 i,rxlen;
	u16 lenx;

	u8 upload=0;

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
   		//LCD_ShowString(40,140,200,24,24,"NEO-6M Setting...");
		while((Ublox_Cfg_Rate(1000,1)!=0)&&key)	//³ÖĞøÅĞ¶Ï,Ö±µ½¿ÉÒÔ¼ì²éµ½NEO-6M,ÇÒÊı¾İ±£´æ³É¹¦
		{
			usart3_init(9600);				//³õÊ¼»¯´®¿Ú3²¨ÌØÂÊÎª9600(EEPROMÃ»ÓĞ±£´æÊı¾İµÄÊ±ºò,²¨ÌØÂÊÎª9600.)
	  	Ublox_Cfg_Prt(38400);			//ÖØĞÂÉèÖÃÄ£¿éµÄ²¨ÌØÂÊÎª38400
			usart3_init(38400);				//³õÊ¼»¯´®¿Ú3²¨ÌØÂÊÎª38400
			Ublox_Cfg_Tp(1000000,100000,1);	//ÉèÖÃPPSÎª1ÃëÖÓÊä³ö1´Î,Âö³å¿í¶ÈÎª100ms	    
			key=Ublox_Cfg_Cfg_Save();		//±£´æÅäÖÃ  
		}	  					 
	   //LCD_ShowString(40,140,200,24,24,"NEO-6M Set Done!!");
		delay_ms(500);
	   //LCD_Fill(30,120,30+200,120+16,WHITE);//Çå³ıÏÔÊ¾ 
	}

	while(1) 
	{	
    //timer.c ÀïµÄTIM3_IRQHandler()×÷Îª¶¨Ê±Æ÷ÖĞ¶Ï·şÎñ×Óº¯Êı
		delay_ms(5);
		if(USART3_RX_STA&0X8000)		//½ÓÊÕµ½Ò»´ÎÊı¾İÁË
		{
			//½ÓÊÜÊı¾İ³É¹¦ÔòD0ÉÁË¸
			if((lenx%500)==0)LED0=!LED0; 	    				 
			lenx++;	
			rxlen=USART3_RX_STA&0X7FFF;	//µÃµ½Êı¾İ³¤¶È
			for(i=0;i<rxlen;i++)USART1_TX_BUF[i]=USART3_RX_BUF[i];	   
 			USART3_RX_STA=0;		   	//Æô¶¯ÏÂÒ»´Î½ÓÊÕ
			USART1_TX_BUF[i]=0;			//×Ô¶¯Ìí¼Ó½áÊø·û
			GPS_Analysis(&gpsx,(u8*)USART1_TX_BUF);//·ÖÎö×Ö·û´®
			/*  ÕâÊÇÒ»¸öbug
			if(flag_draw)
			{
				LCD_Clear(WHITE);
				Draw_Path();    //»æÖÆ¹ì¼£
			}
			else
			{
				LCD_Clear(WHITE);
				//show_chinese(30,140,ı",RED,WHITE);  //YELLOW
				Gps_Msg_Show();				//ÏÔÊ¾ĞÅÏ¢
			*/
			if(!flag_demo) Gps_Msg_Show();				//ÏÔÊ¾ĞÅÏ¢
			
	   
			//}
			//if(upload)printf("\r\n%s\r\n",USART1_TX_BUF);//·¢ËÍ½ÓÊÕµ½µÄÊı¾İµ½´®¿Ú1
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
		



	}									    
}

