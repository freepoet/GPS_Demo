#include "chinese.h"
#include "chfont.h"
#include "lcd.h"
//Mini STM32开发板——扩展实验
//自定义汉字显示 代码			 
//正点原子@ALIENTEK
//2010/7/6
			    
//在LCD上的（x，y）处画点
//color：点的颜色
void LCD_Draw_Point(u16 x,u16 y,u16 color)
	{
	u16 temp;
	temp=POINT_COLOR;
	POINT_COLOR=color;
	LCD_DrawPoint(x,y);
	POINT_COLOR=temp;
	}

//在指定位置 显示1个16*16的汉字
//(x,y):汉字显示的位置
//index:tfont数组里面的第几个汉字
//color:这个汉字的颜色
void Test_Show_CH_Font16(u16 x,u16 y,u8 index,u16 color)
	{   			    
	u8 temp,t,t1;
	u16 y0=y;				   
	for(t=0;t<32;t++)//每个16*16的汉字点阵 有32个字节(8+8)*2
		{   
		//二维数组每一维最大16字节
		if(t<16)temp=tfont16[index*2][t];      //前16个字节
		else temp=tfont16[index*2+1][t-16];    //后16个字节	                          
		for(t1=0;t1<8;t1++)	 //每个字节8位画点
			{
			if(temp&0x80)LCD_Draw_Point(x,y,color);//画实心点
			else LCD_Draw_Point(x,y,BACK_COLOR);   //画空白点（使用背景色）
			temp<<=1;
			y++;
			if((y-y0)==16)	 //达到点阵的最大值则开始新的一行
				{
				y=y0;
				x++;
				break;
				}
			}  	 
		}          
	}


//在指定位置 显示1个24*24的汉字
//(x,y):汉字显示的位置
//index:tfont数组里面的第几个汉字
//color:这个汉字的颜色
void Test_Show_CH_Font24(u16 x,u16 y,u8 index,u16 color)
	{   			    
	u8 temp,t,t1;
	u16 y0=y;				   
	for(t=0;t<72;t++)//每个24*24的汉字点阵 有72个字节
		{   
		//二维数组每一维最大24字节
		if(t<24)temp=tfont24[index*3][t];           //前24个字节
		else if(t<48)temp=tfont24[index*3+1][t-24]; //中24个字节	                          
		else temp=tfont24[index*3+2][t-48];         //后24个字节
		for(t1=0;t1<8;t1++)	 //每个字节8位画点
			{
			if(temp&0x80)LCD_Draw_Point(x,y,color);//画实心点
			else LCD_Draw_Point(x,y,BACK_COLOR);   //画空白点（使用背景色）
			temp<<=1;
			y++;
			if((y-y0)==24)	  //达到点阵的最大值则开始新的一行
				{
				y=y0;
				x++;
				break;
				}
			}  	 
		}          
	}

//测试2个汉字显示函数
void TEST_FONT(void)
{
	Test_Show_CH_Font16(200,50,1,RED);
	
	
	
}
/*
	{
	u8 t;
	u8 x,x1;
	x=x1=50;
	for(t=0;t<7;t++)//6个汉字
		{
		Test_Show_CH_Font16(x ,50,t,RED);
		Test_Show_CH_Font24(x1,70,t,RED);
		x+=16;
		x1+=24;
		}
	}

*/





