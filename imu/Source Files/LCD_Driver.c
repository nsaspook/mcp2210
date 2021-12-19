#include "../Header Files/LCD_Driver.h"
#include <plib.h>

#define	CS_LAT_BIT		LATDbits.LATD2
#define	RST_LAT_BIT		LATDbits.LATD3
#define	RS_LAT_BIT		LATDbits.LATD1

//=======================================
void delay(unsigned int count)
{
	while(--count);
}

//=======================================
void Write_Command_S(unsigned char command)
{
	mPORTDSetBits(BIT_5);
	mPORTEWrite(command);
	mPORTDClearBits(BIT_1 | BIT_4 | BIT_2);
	mPORTDSetBits(BIT_4 | BIT_2);
}

//=======================================
void Write_Data_S(unsigned char data)
{
	mPORTDSetBits(BIT_5);
	mPORTEWrite(data);
	mPORTDSetBits(BIT_1);
	mPORTDClearBits(BIT_4 | BIT_2);
	mPORTDSetBits(BIT_4 | BIT_2);
}

//=======================================
void Write_Command(BYTE command)
{       
	mPORTDClearBits(BIT_1);
	PMDIN = command;
}

//=======================================
void Write_Data(BYTE data1)
{       
	mPORTDSetBits(BIT_1);
	PMDIN = data1;
}

//=======================================
void Command_Write(BYTE reg, BYTE value)
{       
	Write_Command( reg );
	Write_Data( value );
}

//=======================================
void Init_SSD1963 (void){

	RST_LAT_BIT = 0;
	delay(10000);
	RST_LAT_BIT = 1;
	delay(10000);
	Write_Command_S(0x01);		//Software Reset
	delay(10000);

	Write_Command_S(0x01);
	delay(10000);

	Write_Command_S(0xe0);		//START PLL
	Write_Data_S(0x01);

	delay(10000);

	Write_Command_S(0xe0);		//LOCK PLL
	Write_Data_S(0x03);

	// We are now running from the high speed clock, so use the PMP
	// PMP setup 
	PMMODE = 0; PMAEN = 0; PMCON = 0;
	PMMODEbits.MODE   = 2;  // Intel 80 master interface
	PMMODEbits.WAITB  = 0;
	PMMODEbits.WAITM  = 0;
	PMMODEbits.WAITE  = 0;
	PMMODEbits.MODE16 = 0;  // 8 bit mode
	PMCONbits.PTRDEN = 1;  // enable RD line
	PMCONbits.PTWREN = 1;  // enable WR line
	PMCONbits.PMPEN  = 1;  // enable PMP

	CS_LAT_BIT = 0;        // enable SSD1963 

	Write_Command(0xb0);		//SET LCD MODE  SET TFT 18Bits MODE
	Write_Data(0x0c);			//SET TFT MODE & hsync+Vsync+DEN MODE
	Write_Data(0x80);			//SET TFT MODE & hsync+Vsync+DEN MODE
	Write_Data(0x01);			//SET horizontal size=320-1 HightByte
	Write_Data(0x3f);		    //SET horizontal size=320-1 LowByte
	Write_Data(0x00);			//SET vertical size=240-1 HightByte
	Write_Data(0xef);			//SET vertical size=240-1 LowByte
	Write_Data(0x00);			//SET even/odd line RGB seq.=RGB

	Write_Command(0xf0);		//SET pixel data I/F format=8bit
	Write_Data(0x00);

	Write_Command(0x3a);		// Set pixel format
	//Write_Data(0x60);			// 18 bits per pixel
	Write_Data(0x70);			// 24 bits per pixel


	Write_Command(0xe6);   		//SET PCLK freq=4.94MHz  ; pixel clock frequency
	Write_Data(0x0a);
	Write_Data(0x3d);
	Write_Data(0x70);

/*
	Write_Command(0xe6);   		//SET PCLK freq=6.4MHz  ; pixel clock frequency
	Write_Data(0x00);
	Write_Data(0xe7);
	Write_Data(0x4f);
*/

	Write_Command(0xb4);		//SET HBP, 
	Write_Data(0x01);			//SET HSYNC Total 440
	Write_Data(0xb8);
	Write_Data(0x00);			//SET HBP 68
	Write_Data(0x46);
	Write_Data(0x0f);			//SET VBP 16=15+1
	Write_Data(0x00);			//SET Hsync pulse start position
	Write_Data(0x00);
	Write_Data(0x00);			//SET Hsync pulse subpixel start position

	Write_Command(0xb6); 		//SET VBP, 
	Write_Data(0x01);			//SET Vsync total 265=264+1
	Write_Data(0x48);
	Write_Data(0x00);			//SET VBP=19
	Write_Data(0x0d);
	Write_Data(0x07);			//SET Vsync pulse 8=7+1
	Write_Data(0x00);			//SET Vsync pulse start position
	Write_Data(0x00);

	Write_Command(0x2a);		//SET column address
	Write_Data(0x00);			//SET start column address=0
	Write_Data(0x00);
	Write_Data(0x01);			//SET end column address=319
	Write_Data(0x3f);

	Write_Command(0x2b);		//SET page address
	Write_Data(0x00);			//SET start page address=0
	Write_Data(0x00);
	Write_Data(0x00);			//SET end page address=239
	Write_Data(0xef);

	Write_Command(0x13);		//SET normal mode
	Write_Command(0x29);		//SET display on
}

//=======================================
inline void SendData(unsigned int color)
{
	Write_Data((color)>>16);	//red
	Write_Data((color)>>8);		//green
	Write_Data(color);			//blue
}

//=======================================
void WindowSet(unsigned int s_x,unsigned int e_x,unsigned int s_y,unsigned int e_y)
{
	Write_Command(0x2a);	//SET page address
	Write_Data((s_x)>>8);	//SET start page address=0
	Write_Data(s_x);
	Write_Data((e_x)>>8);	//SET end page address=319
	Write_Data(e_x);
	
	Write_Command(0x2b);	//SET column address
	Write_Data((s_y)>>8);	//SET start column address=0
	Write_Data(s_y);
	Write_Data((e_y)>>8);	//SET end column address=239
	Write_Data(e_y);
}

//=======================================
void SetPixel(unsigned int x,unsigned int y, unsigned int color)
{
	WindowSet(x,x,y,y);
	Write_Command(0x2c);
	SendData(color);
}

//=======================================
void FillColor(unsigned int color)
{
	unsigned int i,j;
	WindowSet(0,319,0,239);
	Write_Command(0x2c);
	for(j=0;j<240;j++)
	{
		for(i=0;i<320;i++)
		{
			SendData(color);
		}
	}
}
