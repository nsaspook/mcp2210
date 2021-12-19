#include "../Header Files/Frame_Buffer.h"
#include "../Header Files/LCD_Driver.h"

//=======================================
void NewFrame()
//Starts a new Frame
{
	WindowSet(0,319,0,239);
	Write_Command(0x2c);
}

//=======================================
void DrawFrameRow()
//After the current FrameRow is filled, draw it and increment the FrameRowIndex
{
	unsigned int i;
	for(i = 0; i < 320; i++)
	{
		SendData(FrameRow[i]);
	}
	FrameRowIndex++;
}

//=======================================
void FillFrameRow(int color)
//Fill the current FrameRow with a color
{
	unsigned int i;
	for(i=0;i<320;i++)
	{
		FrameRow[i] = color;
	}
}
