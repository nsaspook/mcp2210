#include "../Header Files/LCD_2D.h"
#include "../Header Files/Frame_Buffer.h"

//=======================================
void DrawHLine(int s_x, int e_x, int color)
//Draws a horizontal line of "color" on the current FrameRow
//(Per FrameRow)
{
	if(s_x > e_x)
	{
		int t = s_x; s_x = e_x; e_x = t;
	}
	int i;
	for(i=0;i<(e_x-s_x);i++)
		FrameRow[s_x+i] = color;
}

//=======================================
void DrawTriangle(int Tri[3][3], int color)
//Draws the line of the triangle that is on the current FrameRow
//(Per FrameRow)
{
	int xA = Tri[0][0];
	int xB = Tri[1][0];
	int xC = Tri[2][0];
	int yA = Tri[0][1];
	int yB = Tri[1][1];
	int yC = Tri[2][1];

	int normal = (xC-xA)*(yC-yB)-(yC-yA)*(xC-xB);
	if(normal < 0)
	{
		//sort triangle vertices a,c,b
		int t;
		if (yC < yA){
			t = xA; xA = xC; xC = t;
			t = yA; yA = yC; yC = t;
		}
		if (yB < yC){
			t = xB; xB = xC; xC = t;
			t = yB; yB = yC; yC = t;
		}
		if (yC < yA){
			t = xA; xA = xC; xC = t;
			t = yA; yA = yC; yC = t;
		}
	
		int y = FrameRowIndex;
		if((y >= yA) && (y <= yB)){
	
			//find the change in y
			int dyCA = (yA-yC);
			int dyCB = (yB-yC);
			int dyAB = (yB-yA);
			
			//find the change in x
			int dxCA = (xA-xC);
			int dxCB = (xB-xC);
			int dxAB = (xB-xA);
			
			//m = dY/dX
			//x = (y-y1)/m+x1
			//x = (y-y1)/(dY/dX)+x1
			//x = dX(y-y1)/dY+x1
	
			int s_x;
			int e_x =(dxAB*(y-yA))/dyAB+xA;
	
			if(y < yC)
				s_x = (dxCA*(y-yC))/dyCA+xC;	//find x for each y of side CA
			else if(y > yC)
				s_x = (dxCB*(y-yC))/dyCB+xC;	//find x for each y of side CB
			else if(y == yC)
				s_x = xC;
	
			int ShadedColor = color & (0x010101*(((xA+xB+xC)/3)*255/320));
			DrawHLine(s_x, e_x, ShadedColor);	//draw a line from (y,s_x) to (y,e_x)
		}
	}
}
