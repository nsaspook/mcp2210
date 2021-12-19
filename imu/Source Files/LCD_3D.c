#include "../Header Files/LCD_3D.h"
#include "../Header Files/LCD_2D.h"

#include "../Data Files/Ring.h"

//=======================================
void DrawMesh(Mesh *InMesh) 
//Draws a "Mesh" type directly (slow)
//(Per FrameRow)
{
	int tri[3][3];
	int i, j, k;
	for(i = 0; i < (*InMesh).NumTris; i++){
		for(j = 0; j < 3; j++){
			for(k = 0; k < 3; k++){
				tri[j][k] = (*InMesh).v[(*InMesh).f[i][j]][k];
			}
		}
		DrawTriangle(tri, 0xFF0000);
	}
}

//=======================================
void DrawTriList(TriList *InTriList) 
//Draws a "TriList" type (faster than drawing a Mesh type directly)
//(Per FrameRow)
{
	int i;
	for( i = 0; i < (*InTriList).NumTris; i++ )
		DrawTriangle((*InTriList).Tri[i], 0xFF0000);
}

//=======================================
TriList Mesh_to_TriList(Mesh *InMesh) 
//Converts a "Mesh" to a "TriList"
//(Per Frame)
{
	TriList OutTriList;
	OutTriList.NumTris = (*InMesh).NumTris;

	int i, j, k;
	for(i = 0; i < (*InMesh).NumTris; i++){
		for(j = 0; j < 3; j++){
			for(k = 0; k < 3; k++){
				OutTriList.Tri[i][j][k] = (*InMesh).v[(*InMesh).f[i][j]][k];
			}
		}
	}
	return OutTriList;
}

//=======================================
Mesh TransformMesh(float sx, float sy, float sz, float ox, float oy, float oz, float rx, float ry, float rz)
//Transforms the N64 raw mesh (rotate, offset, scale) then returns a new Mesh
//(Per Frame)
{	
	Mesh OutMesh;

	float v[3];
	float vp[3];
	int vi[3];

	float sinrx, sinry, sinrz, cosrx, cosry, cosrz;
	sinrz = sin(rz);
	cosrz = cos(rz);
	sinry = sin(ry);
	cosry = cos(ry);
	sinrx = sin(rx);
	cosrx = cos(rx);

	int i,j;
	int fl = 5;

	OutMesh.NumVerts = 0;
	int NumVerts = 24;
	int NumTris = 48;

	for( i = 0; i <= NumVerts; i++ )
	{
		for(j = 0; j < 3; j++){
			v[j] = v1[i][j];
			vp[j] = v[j];
		}

		//-------------------------------
		v[0] = vp[0] * cosrz - vp[1] * sinrz;
		v[1] = vp[0] * sinrz + vp[1] * cosrz;

		for(j = 0; j < 3; j++){
			vp[j] = v[j];
		}

		//-------------------------------
		v[2] = vp[2] * cosry - vp[0] * sinry;
		v[0] = vp[2] * sinry + vp[0] * cosry;

		for(j = 0; j < 3; j++){
			vp[j] = v[j];
		}

		//-------------------------------
		v[1] = vp[1] * cosrx - vp[2] * sinrx;
		v[2] = vp[1] * sinrx + vp[2] * cosrx;		

		//-------------------------------
		vi[0] = (int)(v[0]*sx*fl/(fl+v[2]) + ox);
		vi[1] = (int)(v[1]*sy*fl/(fl+v[2]) + oy);
		vi[2] = (int)(v[2]*sz + oz);

		for(j = 0; j < 3; j++){
			OutMesh.v[OutMesh.NumVerts][j] = vi[j];
		}
		OutMesh.NumVerts++;
	}

	OutMesh.NumTris = NumTris;
	for(i = 0; i < OutMesh.NumTris; i++){
		for(j = 0; j < 3; j++){
			OutMesh.f[i][j] = f1[i][j];
		}
	}
	return OutMesh;
}
