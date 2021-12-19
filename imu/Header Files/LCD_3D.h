typedef struct Mesh
{
	unsigned char NumVerts;
	unsigned char NumTris;
	unsigned char f[128][3];
	int v[128][3];
} Mesh;

typedef struct TriList
{
	unsigned char NumTris;
	int Tri[128][3][3];
} TriList;

void DrawTriList(TriList *Tris);
TriList Mesh_to_TriList(Mesh *Mesh);
Mesh TransformMesh(float sx, float sy, float sz, float ox, float oy, float oz, float rx, float ry, float rz);
