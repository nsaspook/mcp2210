#include <string.h>

#include "../Header Files/LCD_Text.h"

#include "../Header Files/LCD_Driver.h"
#include "../Data Files/F8X8A.h"

//=======================================
void DrawCh(unsigned int x, unsigned int y, unsigned char ch, long fg, long bg)
{
	unsigned int i,j;
	WindowSet(x,x+7,y,y+7);
	Write_Command(0x2c);
	for(j=0;j<8;j++){
		for(i=0;i<8;i++){
			if((F8X8A[ch][i]>>j)%2)
				SendData(fg);
			else
				SendData(bg);
		}
	}
}

//=======================================
void DrawStr(unsigned int x, unsigned int y, unsigned char * ch, long fg, long bg)
{
	unsigned int i;
	for(i=0;i<strlen(ch);i++){
		DrawCh(x+(i*8), y, ch[i], fg, bg);
	}
}
