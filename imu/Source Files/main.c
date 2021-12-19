#include <plib.h>

#include "../Header Files/PIC_config.h"
#include "../Header Files/LCD_driver.h"
#include "../Header Files/LCD_3D.h"
#include "../Header Files/Frame_Buffer.h"

//=======================================
static int comp(const void* p1, const void* p2) {
  int* arr2 = (int*)p1;
  int* arr1 = (int*)p2;
  int diff1 = arr1[2] - arr2[2];
  if (diff1) return diff1;
  return arr1[1] - arr2[1];
}

TriList ZSort(TriList *InList)
{	
	TriList OutList = (*InList);
	qsort(OutList.Tri, OutList.NumTris, 3*3*sizeof(int), comp);
	return OutList;
}

//=======================================
int main()
{
	unsigned int cache_status;
	mBMXDisableDRMWaitState();
	mCheConfigure(3);
	cache_status = mCheGetCon();
	cache_status |= CHE_CONF_PF_ALL;
	mCheConfigure(cache_status);
	CheKseg0CacheOn(); 

	TRISB = 0x0000;
	LATB = 0x0000;
	TRISD = 0x0000;
	LATD = 0x0000;
	TRISE = 0x0000;
	LATE = 0x0000;
	AD1PCFG = 0xffff;

	Init_SSD1963();

	Mesh WorkingMesh;
	TriList N64TriList;
	int i;
	float j;

	while(1)
	{
		j = 0.0;
		for(i = 0; i < 200; i++)
		{
			j += 0.0314159265;

			NewFrame();
			WorkingMesh = TransformMesh(100, 100, 100, 160, 120, 100, j, j, j/2);
			N64TriList = Mesh_to_TriList(&WorkingMesh);
			N64TriList = ZSort(&N64TriList);

			for( FrameRowIndex = 0; FrameRowIndex < 240; DrawFrameRow())
			{
				FillFrameRow(0x000000);
				DrawTriList(&N64TriList);
				//DrawMesh(&WorkingMesh);
			}
		}
	}
}
