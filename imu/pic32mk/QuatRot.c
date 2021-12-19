/***********************************************************************/
/*                                                                     */
/* Quaternion Rotation Demo                                            */
/* ------------------------                                            */
/*                                                                     */

/**************************************************************/
/*  Copyright 2006, Trustees of Indiana University            */
/*  Author: Andrew J. Hanson                                  */
/*  Last modified: 14 January 2006                            */
/*  License available at: visualizingquaternions_license.txt  */
/**************************************************************/


/* Unix/Linux Compilation: for Linux, SGI-IRIX, SUN-SOLARIS:

cc -o QuatRot.linux QuatRot.c -lglut -lGLU -lGL -lm
cc -o QuatRot.sgi QuatRot.c -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm 
cc -o QuatRot.sun QuatRot.c -L/localglut/lib -I/localglut/include -lglut -lGLU -lGL -lXmu -lXext -lX11 -lm

*/

/* Updates and Features                                                */

/* - Tue Nov 29 20:06:01 EST 2005                                      */
/*   - 'x' 'y' 'z' 'w' choose projection to 3D                         */
/* - Sat Apr 21 13:01:27 EST 2001                                      */
/*   added by Philip                                                   */
/*   - trails                                                          */
/*   - mode : examine                                                  */
/*   - 'r' reset all                                                   */
/*   - 'k' to set keyframe                                             */
/*   - different curve type                                            */
/*   - display 1st/2nd derivative                                      */
/*   - no lighting on points or trail                                  */
/*   - 'z' toggle viewing frame                                        */
/*   - q0 first in 'i'                                                 */
/*   - '0' to show a teapot at the corner                              */
/*   - '9' to show colored cube instead                                */
/*   - update help menu                                                */
/*   - fix the Catmull Rom / Uniform B spline (2nd derivatives)        */
/*   - fix the Beizer spline                                           */
/*   - compare keyframe by keyframe dot product and                    */
/*     frame by frame dot product                                      */
/*   - show marker                                                     */
/*                                                                     */
/*                                                                     */
/*   - rotate the teapot/cube by shift left mouse                      */
/*                                                                     */
/***********************************************************************/




/***********************************************************************/

/* Display: value of qz at left: 
            3-vector q at center with shaded if qz < 0.
            Full 3D frame at right, with lit cube to give context.

 * Interaction: Left-drag: rolling ball rotate frame
 *              Middle: reset
 *              Right: Menu: 
 *                /toggle trail/1D:2D:3D sphere/toggle exp vs sphere/
 */
/***********************************************************************/







/***********************************************************************/
/**************************   includes   *******************************/
/***********************************************************************/

#include <stdlib.h>      /* defines NULL = 0L  */
#include <stdio.h>
#include <math.h>
#include <GL/glut.h>

// for windows platform
#ifdef _WIN32
#include <windows.h>
#endif




/***********************************************************************/
/**************************   useful utility macros  *******************/
/***********************************************************************/

#define Sign(A)  ((A) < 0.0 ? -1.0 : ((A) > 0.0 ? 1.0 : 0.0))
#define Min(A,B) ((A) <= (B) ? (A) : (B))
#define Max(A,B) ((A) >= (B) ? (A) : (B))
#define Inbetween(A,X,B) ((Min(A,B) <= (X)) && ((X) <= Max(A,B)))

// for windows platform
#ifdef _WIN32
#define M_PI		3.14159265358979323846
#endif

#define _CATMULL	0
#define _BEZIER		1
#define _BSPLINE	2

// Near and Far planes
#define _NEAR		5.0f
#define _FAR		25.0f




/***********************************************************************/
/**************************   global variables   ***********************/
/***********************************************************************/

#define _ROTSCALE		1.0

#define _MAX_TRAIL_LENGTH	300
#define _MAX_KEYFRAME		256

GLint     mouseModifiers = 0;
GLint     buttonCode = 0;
GLint     objectID;
GLint     dx, dy, xold=0, yold=0;
GLint     wid, hi;
GLfloat   XC, YC, ZC;
int       cylList;
GLboolean worldAxesToggle, viewAxesToggle, sphereToggle, trailToggle;
GLboolean curveToggle, startDevToggle, endDevToggle, kkDotProdToggle, markerToggle;
GLint Project4D;

GLfloat light_ambient[]     = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat light_diffuse[]     = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_specular[]    = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat light_position[]    = {-10.0f,-10.0f,-10.0f, 0.0f };	/* At infinity */
GLfloat material_ambient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
GLfloat material_diffuse[]  = { 0.7f, 0.7f, 0.0f, 1.0f };
GLfloat material_specular[] = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat less_specular[]     = { 0.8f, 0.8f, 0.8f, 1.0f };
GLfloat material_shininess  = 20.0f;
GLfloat less_shininess      =  0.0f;
GLfloat shoulder_diffuse[]  = { 0.0f, 0.0f, 1.0f, 1.0f };
GLfloat elbow_diffuse[]     = { 1.0f, 1.0f, 0.0f, 1.0f };
GLfloat white_diffuse[]     = { 1.0f, 1.0f, 1.0f, 1.0f };
GLfloat bla_diffuse[]       = { 0.0f, 0.8f, 0.8f, 1.0f };
GLfloat black_diffuse[]     = { 0.0f, 0.0f, 0.0f, 1.0f };
GLfloat orange_diffuse[]    = { 0.8f, 0.6f, 0.2f, 1.0f };
GLfloat purple_diffuse[]    = { 0.8f, 0.2f, 0.8f, 1.0f };
GLfloat red_diffuse[]       = { 0.8f, 0.2f, 0.2f, 1.0f };
GLfloat green_diffuse[]     = { 0.2f, 0.8f, 0.2f, 1.0f };
GLfloat blue_diffuse[]      = { 0.2f, 0.2f, 0.8f, 1.0f };




/***********************************************************************/
/* Quaternion and Rotation utilities                                   */
/***********************************************************************/

typedef struct Point {
  double x, y, z;
} Point;

typedef struct Point4D {
  double x, y, z, w;
} Point4D;

/* NOTE - inverted zeroth element order */
typedef struct tag_GL_QUAT { 
  double w, x, y, z;
} GLQuat;

// Comet trail to trace the quaternion we have made
GLQuat QTotal[_MAX_TRAIL_LENGTH];
int    currQTotal,nextQTotal,trailLength;

// Quaternion Curves
GLQuat QCurve[_MAX_KEYFRAME],qMarker;
int    nKeyFrame,curveType;
double markerFrame;
typedef void (*CurveFuncPtr)(GLQuat *, GLQuat *, GLQuat *, GLQuat *, double, GLQuat *);
CurveFuncPtr curveFunc;

GLQuat QTmp;
Point  XAxis,YAxis,ZAxis;
double DegToRad = (M_PI/180.0);
double RadToDeg = (180.0/M_PI);
Point  zero = { 0.0, 0.0, 0.0 };

Point4D shoulder[7][7] = {
{
{ 0         ,-0.707107 , 0        , 0.707107 },
{ 0         ,-0.5      , 0        , 0.866025 },
{ 0         ,-0.258819 , 0        , 0.965926 },
{ 0         , 0        , 0        , 1.       },
{ 0         , 0.258819 , 0        , 0.965926 },
{ 0         , 0.5      , 0        , 0.866025 },
{ 0         , 0.707107 , 0        , 0.707107 }
},
{
{ 0.092296  ,-0.701057 , 0.092296 , 0.701057 },
{ 0.0652631 ,-0.495722 , 0.113039 , 0.858616 },
{ 0.0337827 ,-0.256605 , 0.126079 , 0.957662 },
{ 0         , 0        , 0.130526 , 0.991445 },
{-0.0337827 , 0.256605 , 0.126079 , 0.957662 },
{-0.0652631 , 0.495722 , 0.113039 , 0.858616 },
{-0.092296  , 0.701057 , 0.092296 , 0.701057 }
},
{
{ 0.183013  ,-0.683013 , 0.183013 , 0.683013 },
{ 0.12941   ,-0.482963 , 0.224144 , 0.836516 },
{ 0.0669873 ,-0.25     , 0.25     , 0.933013 },
{ 0         , 0        , 0.258819 , 0.965926 },
{-0.0669873 , 0.25     , 0.25     , 0.933013 },
{-0.12941   , 0.482963 , 0.224144 , 0.836516 },
{-0.183013  , 0.683013 , 0.183013 , 0.683013 }
},
{
{ 0.270598  ,-0.653281 , 0.270598 , 0.653281 },
{ 0.191342  ,-0.46194  , 0.331414 , 0.800103 },
{ 0.0990458 ,-0.239118 , 0.369644 , 0.892399 },
{ 0         , 0        , 0.382683 , 0.92388  },
{-0.0990458 , 0.239118 , 0.369644 , 0.892399 },
{-0.191342  , 0.46194  , 0.331414 , 0.800103 },
{-0.270598  , 0.653281 , 0.270598 , 0.653281 }
},
{
{ 0.353553  ,-0.612372 , 0.353553 , 0.612372 },
{ 0.25      ,-0.433013 , 0.433013 , 0.75     },
{ 0.12941   ,-0.224144 , 0.482963 , 0.836516 },
{ 0         , 0        , 0.5      , 0.866025 },
{-0.12941   , 0.224144 , 0.482963 , 0.836516 },
{-0.25      , 0.433013 , 0.433013 , 0.75     },
{-0.353553  , 0.612372 , 0.353553 , 0.612372 }
},
{
{ 0.430459  ,-0.560986 , 0.430459 , 0.560986 },
{ 0.304381  ,-0.396677 , 0.527203 , 0.687064 },
{ 0.157559  ,-0.205335 , 0.588018 , 0.76632  },
{ 0         , 0        , 0.608761 , 0.793353 },
{-0.157559  , 0.205335 , 0.588018 , 0.76632  },
{-0.304381  , 0.396677 , 0.527203 , 0.687064 },
{-0.430459  , 0.560986 , 0.430459 , 0.560986 }
},
{
{ 0.5       ,-0.5      , 0.5      , 0.5      },
{ 0.353553  ,-0.353553 , 0.612372 , 0.612372 },
{ 0.183013  ,-0.183013 , 0.683013 , 0.683013 },
{ 0         , 0        , 0.707107 , 0.707107 },
{-0.183013  , 0.183013 , 0.683013 , 0.683013 },
{-0.353553  , 0.353553 , 0.612372 , 0.612372 },
{-0.5       , 0.5      , 0.5      , 0.5      }
}
};

Point4D elbow[7][7] = {
{
{ 0         , 0        , 0        , 1.       },
{-0.258819  , 0        , 0        , 0.965926 },
{-0.5       , 0        , 0        , 0.866025 },
{-0.707107  , 0        , 0        , 0.707107 },
{-0.866025  , 0        , 0        , 0.5      },
{-0.965926  , 0        , 0        , 0.258819 },
{-1.        , 0        , 0        , 0        }
},
{
{0          ,-0.258819 , 0        , 0.965926 },
{-0.25      ,-0.25     ,-0.0669873, 0.933013 },
{-0.482963  ,-0.224144 ,-0.12941  , 0.836516 },
{-0.683013  ,-0.183013 ,-0.183013 , 0.683013 },
{-0.836516  ,-0.12941  ,-0.224144 , 0.482963 },
{-0.933013  ,-0.0669873,-0.25     , 0.25     },
{-0.965926  , 0        ,-0.258819 , 0        }
},
{
{ 0         ,-0.5      , 0        , 0.866025 },
{-0.224144  ,-0.482963 ,-0.12941  , 0.836516 },
{-0.433013  ,-0.433013 ,-0.25     , 0.75     },
{-0.612372  ,-0.353553 ,-0.353553 , 0.612372 },
{-0.75      ,-0.25     ,-0.433013 , 0.433013 },
{-0.836516  ,-0.12941  ,-0.482963 , 0.224144 },
{-0.866025  , 0        ,-0.5      , 0        }
},
{
{ 0         ,-0.707107 , 0        , 0.707107 },
{-0.183013  ,-0.683013 ,-0.183013 , 0.683013 },
{-0.353553  ,-0.612372 ,-0.353553 , 0.612372 },
{-0.5       ,-0.5      ,-0.5      , 0.5      },
{-0.612372  ,-0.353553 ,-0.612372 , 0.353553 },
{-0.683013  ,-0.183013 ,-0.683013 , 0.183013 },
{-0.707107  , 0        ,-0.707107 , 0        }
},
{
{ 0         ,-0.866025 , 0        , 0.5      },
{-0.12941   ,-0.836516 ,-0.224144 , 0.482963 },
{-0.25      ,-0.75     ,-0.433013 , 0.433013 },
{-0.353553  ,-0.612372 ,-0.612372 , 0.353553 },
{-0.433013  ,-0.433013 ,-0.75     , 0.25     },
{-0.482963  ,-0.224144 ,-0.836516 , 0.12941  },
{-0.5       , 0        ,-0.866025 , 0        }
},
{
{ 0         ,-0.965926 , 0        , 0.258819 },
{-0.0669873 ,-0.933013 ,-0.25     , 0.25     },
{-0.12941   ,-0.836516 ,-0.482963 , 0.224144 },
{-0.183013  ,-0.683013 ,-0.683013 , 0.183013 },
{-0.224144  ,-0.482963 ,-0.836516 , 0.12941  },
{-0.25      ,-0.25     ,-0.933013 , 0.0669873},
{-0.258819  , 0        ,-0.965926 , 0        }
},
{
{ 0         ,-1.       , 0        , 0 },
{ 0         ,-0.965926 ,-0.258819 , 0 },
{ 0         ,-0.866025 ,-0.5      , 0 },
{ 0         ,-0.707107 ,-0.707107 , 0 },
{ 0         ,-0.5      ,-0.866025 , 0 },
{ 0         ,-0.258819 ,-0.965926 , 0 },
{ 0         , 0        ,-1.       , 0 }
}
};




/***********************************************************************/
/* Utilities                                                           */
/***********************************************************************/

void
printQuat(FILE *stream, GLQuat q)
{
    fprintf(stream,"%f %f %f %f",q.w,q.x,q.y,q.z);
}

void
printQuatN(FILE *stream, GLQuat q)
{
    printQuat(stream,q);
    fprintf(stream,"\n");
}


void
MakeRotMat3(Point *n, double theta, double rotor[3][3])
{
    static double cos_theta, sin_theta,one_cos_theta;

    cos_theta     = cos(theta);
    sin_theta     = sin(theta);
    one_cos_theta = (1.0 - cos_theta);

    rotor[0][0] = cos_theta + n->x * n->x * one_cos_theta;
    rotor[0][1] = n->x * n->y * one_cos_theta - n->z * sin_theta;
    rotor[0][2] = n->x * n->z * one_cos_theta + n->y * sin_theta;
    rotor[1][0] = n->y * n->x * one_cos_theta + n->z * sin_theta;
    rotor[1][1] = cos_theta + n->y * n->y * one_cos_theta;
    rotor[1][2] = n->y * n->z * one_cos_theta - n->x * sin_theta;
    rotor[2][0] = n->z * n->x * one_cos_theta - n->y * sin_theta;
    rotor[2][1] = n->z * n->y * one_cos_theta + n->x * sin_theta;
    rotor[2][2] = cos_theta + n->z * n->z * one_cos_theta;
}


void
UnitVec(Point *v)
{
    double l = sqrt(v->x*v->x + v->y*v->y + v->z*v->z);
    if (l != 0.0) {
	v->x /= l;
	v->y /= l;
	v->z /= l;
    }
}


/* Make  a 3 vector quaternion */
/* Compute normalized cross-product, swap new for old current axis */

void
NCross2(Point *v1, Point *v2, Point *cross)
{
    Point tmp;
    double scale;

    tmp.x = v1->y*v2->z - v1->z*v2->y;
    tmp.y = v1->z*v2->x - v1->x*v2->z;
    tmp.z = v1->x*v2->y - v1->y*v2->x;

    scale = sqrt((double)( tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z));
    if(scale > 0.0) {
	tmp.x /= scale; tmp.y /= scale; tmp.z /= scale;
    }

    *cross = tmp;
}


/* v2 - v1 cross v3 - v2 */

void
NCross3(Point *v1, Point *v2, Point *v3, Point *cross)
{
    Point tmp;
    double scale;

    tmp.x = (v2->y - v1->y)*(v3->z - v2->z) - (v2->z - v1->z)*(v3->y - v2->y);
    tmp.y = (v2->z - v1->z)*(v3->x - v2->x) - (v2->x - v1->x)*(v3->z - v2->z);
    tmp.z = (v2->x - v1->x)*(v3->y - v2->y) - (v2->y - v1->y)*(v3->x - v2->x);

    scale = sqrt((double)( tmp.x*tmp.x + tmp.y*tmp.y + tmp.z*tmp.z));
    if(scale > 0.0) {
	tmp.x = tmp.x/scale; tmp.y = tmp.y/scale; tmp.z = tmp.z/scale;
    }

    *cross = tmp;
}


/* sdotr = (rx*sx + ry*sy + rz*sz);
   sdotr = (fabs(sdotr)>1.0) ? Sign(sdotr) : sdotr;
   theta = (180.0/M_PI)*acos(sdotr);
 */

void
MatTimesPoint3(Point *v,  double m[3][3], Point *v0)
{
    double x,y,z;

    x    = m[0][0]*v0->x + m[0][1]*v0->y + m[0][2]*v0->z;
    y    = m[1][0]*v0->x + m[1][1]*v0->y + m[1][2]*v0->z;
    z    = m[2][0]*v0->x + m[2][1]*v0->y + m[2][2]*v0->z;
    v->x = x;
    v->y = y;
    v->z = z;
}


/*---------------------------------------------------------------------*/
/* Quaternion Utilities:  Projection to 3D subspace, Unit, Flip, etc   */
/*---------------------------------------------------------------------*/
void
ChooseProjection(int projID, GLQuat *input, GLQuat *output)
{
  switch (projID){
  case 0: 			/* XYZ */
    output->w = input->w;
    output->x = input->x;
    output->y = input->y;
    output->z = input->z;
    break;
  case 1: 			/* WYZ */
    output->w = input->x;
    output->x = input->w;
    output->y = input->y;
    output->z = input->z;
    break;
  case 2: 			/* WXZ */
    output->w = input->y;
    output->x = input->w;
    output->y = input->x;
    output->z = input->z;
    break;
  case 3: 			/* WXY */
    output->w = input->z;
    output->x = input->w;
    output->y = input->x;
    output->z = input->y;
    break;
  }
}


void
UnitQuat(GLQuat *v)
{
    double l = sqrt(v->w*v->w + v->x*v->x + v->y*v->y + v->z*v->z);
    if (l != 0.0) {
	v->x /= l;
	v->y /= l;
	v->z /= l;
	v->w /= l;
    }
}

void
QuatFlip(GLQuat *qOut)
{
    qOut->x = -qOut->x;
    qOut->y = -qOut->y;
    qOut->z = -qOut->z;
    qOut->w = -qOut->w;
}

double
QuatDot(GLQuat *q1, GLQuat *q2)
{
    return ( (q1->x * q2->x)
           + (q1->y * q2->y)
           + (q1->z * q2->z)
           + (q1->w * q2->w) );
}

void
PureQuat(GLQuat *q, Point *v)
{
    v->x = q->x;
    v->y = q->y;
    v->z = q->z;
}

void
Point4ToVec(Point4D *p4, Point *v3)
{
    v3->x = p4->x;
    v3->y = p4->y;
    v3->z = p4->z;
}

void
QuatToMat(GLQuat *  quat, double m[4][4])
{
    GLdouble wx, wy, wz, xx, yy, yz, xy, xz, zz, x2, y2, z2;

    x2 = quat->x + quat->x; y2 = quat->y + quat->y; z2 = quat->z + quat->z;
    xx = quat->x * x2;      xy = quat->x * y2;      xz = quat->x * z2;
    yy = quat->y * y2;      yz = quat->y * z2;      zz = quat->z * z2;
    wx = quat->w * x2;      wy = quat->w * y2;      wz = quat->w * z2;

    m[0][0] = 1.0 - (yy + zz);
    m[0][1] = xy - wz;
    m[0][2] = xz + wy;
    m[0][3] = 0.0;
 
    m[1][0] = xy + wz;
    m[1][1] = 1.0 - (xx + zz);
    m[1][2] = yz - wx;
    m[1][3] = 0.0;

    m[2][0] = xz - wy;
    m[2][1] = yz + wx;
    m[2][2] = 1.0 - (xx + yy);
    m[2][3] = 0.0;

    m[3][0] = 0;
    m[3][1] = 0;
    m[3][2] = 0;
    m[3][3] = 1;
}

void
QuatMult(GLQuat* q1, GLQuat* q2, GLQuat* res)
{
    GLQuat tmp;

    tmp.x = q1->w * q2->x + q1->x * q2->w + q1->y * q2->z - q1->z * q2->y;
    tmp.y = q1->w * q2->y + q1->y * q2->w + q1->z * q2->x - q1->x * q2->z;
    tmp.z = q1->w * q2->z + q1->z * q2->w + q1->x * q2->y - q1->y * q2->x;
    tmp.w = q1->w * q2->w - q1->x * q2->x - q1->y * q2->y - q1->z * q2->z;

    *res = tmp;

    // make sure the resulting quaternion is a unit quat.
    UnitQuat(res);
}




/***********************************************************************/
/* Utilities for Quaternion Interpolation                              */
/***********************************************************************/

// Copied from Graphics Gem source code

void
slerp(GLQuat *q1, GLQuat *q2, double alpha, int spin, GLQuat *qOut)
{
    double beta;			// complementary interp parameter
    double theta;			// angle between A and B
    double sin_t, cos_t;		// sine, cosine of theta
    double phi;				// theta plus spins
    int    bflip;			// use negation of B?

    // cosine theta = dot product of A and B
    cos_t = QuatDot(q1,q2);

    // if B is on opposite hemisphere from A, use -B instead
    if (cos_t < 0.0) {
	cos_t = -cos_t;
	bflip = 1;
    } else
	bflip = 0;

    /* if B is (within precision limits) the same as A,
     * just linear interpolate between A and B.
     * Can't do spins, since we don't know what direction to spin.
     */
    if (1.0 - cos_t < 1e-7) {
	beta = 1.0 - alpha;
    } else {				/* normal case */
	theta = acos(cos_t);
	phi   = theta + spin * M_PI;
	sin_t = sin(theta);
	beta  = sin(theta - alpha*phi) / sin_t;
	alpha = sin(alpha*phi) / sin_t;
    }

    if (bflip)
	alpha = -alpha;

    /* interpolate */
    qOut->x = beta*q1->x + alpha*q2->x;
    qOut->y = beta*q1->y + alpha*q2->y;
    qOut->z = beta*q1->z + alpha*q2->z;
    qOut->w = beta*q1->w + alpha*q2->w;

    UnitQuat(qOut);
}

void
CatmullQuat(GLQuat *q00, GLQuat *q01, GLQuat *q02, GLQuat *q03,
            double t, GLQuat *qOut)
{
    GLQuat q10,q11,q12,q20,q21;

    slerp(  q00,  q01,  t+1,   0, &q10 );
    slerp(  q01,  q02,   t,    0, &q11 );
    slerp(  q02,  q03,  t-1,   0, &q12 );

    slerp( &q10, &q11,(t+1)/2, 0, &q20 );
    slerp( &q11, &q12,  t/2,   0, &q21 );

    slerp( &q20, &q21,   t,    0, qOut );
}

void
BezierQuat(GLQuat *q00, GLQuat *q01, GLQuat *q02, GLQuat *q03,
           double t, GLQuat *qOut)
{
    GLQuat q10,q11,q12,q20,q21;

    slerp(  q00,  q01, t, 0, &q10 );
    slerp(  q01,  q02, t, 0, &q11 );
    slerp(  q02,  q03, t, 0, &q12 );

    slerp( &q10, &q11, t, 0, &q20 );
    slerp( &q11, &q12, t, 0, &q21 );

    slerp( &q20, &q21, t, 0, qOut );
}

void
UniformBSplineQuat(GLQuat *q00, GLQuat *q01, GLQuat *q02, GLQuat *q03,
                   double t, GLQuat *qOut)
{
    GLQuat q10,q11,q12,q20,q21;

    slerp(  q00,  q01, (t+2.0)/3.0, 0, &q10 );
    slerp(  q01,  q02, (t+1.0)/3.0, 0, &q11 );
    slerp(  q02,  q03,   t/3.0,     0, &q12 );

    slerp( &q10, &q11, (t+1.0)/2.0, 0, &q20 );
    slerp( &q11, &q12,    t/2.0,    0, &q21 );

    slerp( &q20, &q21, t, 0,qOut);
}




/***********************************************************************/
/* Initialization Routines                                             */
/***********************************************************************/


void
resetAxes()
{
    XAxis.x = 1.0; XAxis.y = 0.0; XAxis.z = 0.0;
    YAxis.x = 0.0; YAxis.y = 1.0; YAxis.z = 0.0;
    ZAxis.x = 0.0; ZAxis.y = 0.0; ZAxis.z = 1.0;
}

void
resetTrail()
{
    int i;

    // Initialize Quaternion stuff
    currQTotal  = 0;
    trailLength = 50;

    for (i=0; i<_MAX_TRAIL_LENGTH; i++) {
	QTotal[i].x = 0.0;
	QTotal[i].y = 0.0;
	QTotal[i].z = 0.0;
	QTotal[i].w = 1.0;
    }
}

void
resetKeyFrame()
{   
    nKeyFrame = 0;

    qMarker.w = 1.0;
    qMarker.x = 0.0;
    qMarker.y = 0.0;
    qMarker.z = 0.0;
}

void
resetQuat()
{
    QTotal[currQTotal].x = 0.0;
    QTotal[currQTotal].y = 0.0;
    QTotal[currQTotal].z = 0.0;
    QTotal[currQTotal].w = 1.0;
}

void
resetView()
{
    XC = 0.0; YC = 0.0; ZC = -10.0;
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(XC,YC,ZC);
}

void
setCurveFunc()
{
    switch(curveType) {

      case _CATMULL :
	curveFunc = &CatmullQuat;
	fprintf(stderr,"Use Catmull Rom Spline.\n");
	break;

      case _BEZIER :
	curveFunc = &BezierQuat;
	fprintf(stderr,"Use Bezier Spline.\n");
	break;

      case _BSPLINE :
	curveFunc = &UniformBSplineQuat;
	fprintf(stderr,"Use Uniform B Spline.\n");
	break;
    }
}

void
myInit()
{
    objectID        = 1;
    trailToggle     = 1;
    sphereToggle    = 0;
    worldAxesToggle = 0;
    viewAxesToggle  = 1;
    curveToggle     = 1;
    startDevToggle  = 0;
    endDevToggle    = 0;
    kkDotProdToggle = 0;
    markerToggle    = 1;

    markerFrame = 0.0;

    Project4D = 0; 		/* default to omit q0 */

    curveType = 0;
    setCurveFunc();

    resetTrail();
    resetAxes();
}


void
myGLInit()
{
    void InitTube(double height, double radius);

    glClearColor(0.1f,0.35f,0.35f,1.0f);
    glEnable(GL_DEPTH_TEST);

    InitTube(1.0, 1.0);

    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,material_diffuse);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    // Anti-aliasing
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glPopMatrix();
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    resetView();
}




/***********************************************************************/
/* Drawing Routines                                                    */
/***********************************************************************/

/* Make a shaded tube to use for axes, etc. 
   cylList is the global display list 
  void CALLBACK errorCallback(GLenum errorCode) */

void  errorCallback(GLenum errorCode)
//void errorCallback(GLenum errorCode)
{
    const GLubyte *estring;
    estring = gluErrorString(errorCode);
    fprintf(stderr, "Quadric Error: %s\n", estring);
    exit(EXIT_FAILURE);
}

/* Default tube runs from origin to height along Z axis
   see GL book p. 492  */

void InitTube(double height, double radius)
{
    GLUquadricObj *qobj;

    cylList = glGenLists(4);	/* glCallList(list), (list+1), etc */
    qobj = gluNewQuadric();

    #ifdef __sgi
    gluQuadricCallback(qobj, GLU_ERROR, (GLvoid (*)()) errorCallback);
    #else
    gluQuadricCallback(qobj, GLU_ERROR, errorCallback);
    #endif
    gluQuadricDrawStyle(qobj, GLU_FILL);

    /* smooth shade vs _POINT, _LINE, _SILHOUETTE */
    gluQuadricNormals(qobj, GLU_SMOOTH); /* smooth vs _FLAT, _NONE */
    /* gluQuadricTexture(qobj, GL_TRUE); */ /*  vs GL_FALSE */
    /* gluQuadricOrientation(qobj, GL_OUTSIDE); */ /*  vs GL_INSIDE */
    glNewList(cylList, GL_COMPILE);
      gluCylinder(qobj, radius, radius, height, 16, 1);
    glEndList();

    gluQuadricDrawStyle(qobj, GLU_FILL);
    gluQuadricNormals(qobj, GLU_SMOOTH); /* smooth vs _FLAT, _NONE */
    glNewList(cylList+1, GL_COMPILE);
      gluDisk(qobj,0.0,radius,16,1);
    glEndList();

    gluQuadricDrawStyle(qobj, GLU_FILL);
    gluQuadricNormals(qobj, GLU_SMOOTH); /* smooth vs _FLAT, _NONE */

    glNewList(cylList+2, GL_COMPILE);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
      glTranslatef(0.0,0.0,height);
      gluDisk(qobj,0.0,radius,16,1);
    glPopMatrix();
    glEndList();
}

/* Dummy 3D line routine to test geometry */
void
DrawTubeX(Point *v1, Point *v2, double radius)
{
    glLineWidth(radius);
    glBegin(GL_LINES);
      glVertex3d(v1->x,v1->y,v1->z);
      glVertex3d(v2->x,v2->y,v2->z);
    glEnd();
    glLineWidth(1.0);
}

/* Routine to show 3D line as a shaded tube to give depth;
   angle = z.(v2-v1)hat, dir = (z x (v2-v1)hat):hat */
void
DrawTube(Point *v1, Point *v2, double radius)
{
    double deg,rad,x,y,z;
    Point  vec,axis,zhat;
    double length;

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glTranslated(v1->x,v1->y,v1->z);

    x = v2->x - v1->x; y = v2->y - v1->y; z = v2->z - v1->z; 
    length = sqrt(x*x + y*y + z*z);
    if(length != 0.0){
	vec.x  = x/length;
	vec.y  = y/length;
	vec.z  = z/length;
	zhat.x = 0.0;
	zhat.y = 0.0;
	zhat.z = 1.0;
	NCross2(&zhat,&vec,&axis);
	UnitVec(&axis);
	rad = acos(vec.z);
	deg = RadToDeg*rad;
	/* Z axis is the core of tube */
	if(vec.z < -.99999) {
	    glRotated(180.0,0.0,1.0,0.0);
	}
	else
	if (vec.z > 0.99999) { ; }
	else {
	    glRotated(deg,axis.x,axis.y,axis.z);
	}
    }
    else {
	length = 0.01;
    }

    glScalef(radius,radius,length);
    glCallList(cylList);
    glCallList(cylList+1); 
    glCallList(cylList+2); 
    glPopMatrix();
}

void
ShowQ0(double hiddenpart)
{
    Point v1, v2;

    /* set matrices */
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(-1.2,1.2,-1.2,1.2);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    /* set color */
    (hiddenpart < 0.0 )?
	glColor3f(0.2f,1.0f,1.0f) : 
	glColor3f(1.0f,1.0f,0.2f);
    (hiddenpart < 0.0) ?
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,bla_diffuse) :
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);

    v1.x = -1.0;
    v1.y =  0.0;
    v1.z =  0.0;
    v2.x = -1.0;
    v2.y =  hiddenpart;
    v2.z =  0.0;

    /* wrong matrix cause major damage!! */
    DrawTube(&v1, &v2, .05f);

    /* reset */
    glMatrixMode(GL_PROJECTION);   
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glEnable(GL_LIGHTING);
}



/*---------------------------------------------------------------------*/
/* Event Handler: Display Window function                              */
/*---------------------------------------------------------------------*/

void
drawCometTrail()
{
    int i,k;


    if (!trailToggle)
	return;

    if (trailToggle == 1) {
	glPointSize(3.0);
	glBegin(GL_POINTS);
    } else
    if (trailToggle == 2) {
	glLineWidth(2.0);
	glBegin(GL_LINE_STRIP);
    }

    k = currQTotal;

    for (i=0; i<trailLength; i++) {
      ChooseProjection(Project4D, &QTotal[k], &QTmp);
      (QTmp.w > 0.0) ?
	    glColor4fv(elbow_diffuse) :
	    glColor4fv(bla_diffuse);
	    //glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,
	glVertex3d( QTmp.x, QTmp.y, QTmp.z );
	k = (k+_MAX_TRAIL_LENGTH-1) % _MAX_TRAIL_LENGTH;
    }
    glEnd();

    if (trailToggle == 1)
	glPointSize(1.0);
    else
    if (trailToggle == 2)
	glLineWidth(1.0);
}


void
drawQuatCurve()
{
    int    i,k,lastIndex,nSegment,markerDrawn;
    GLQuat q0,q1,q2,q3,qOut,qTmp0,qTmp1,qTmp2,qPrev,qNearMarker;
    Point  dev1a,dev1b,dev2;
    double t,markerT;


    if (!curveToggle || !nKeyFrame)
	return;


    ////////////////////////////////////////////////////////////
    // (1) draw points at keyFrames

    glPointSize(8.0);
    glColor4fv(purple_diffuse);
    glBegin(GL_POINTS);
    for (i=0; i<nKeyFrame; i++) {
      ChooseProjection(Project4D, &QCurve[i], &QTmp);
	glVertex3d( QTmp.x, QTmp.y, QTmp.z );
      //	glVertex3d( QCurve[i].x, QCurve[i].y, QCurve[i].z );
    }
    glEnd();
    glPointSize(1.0);


    ////////////////////////////////////////////////////////////
    // (2) initialize markerT

    markerT     = markerFrame;
    markerDrawn = 0;

    if (curveType == _BEZIER)
	nSegment = ((nKeyFrame-1) / 3);
    else
	nSegment = nKeyFrame-3;

    if (markerT >= (double) nSegment)
	markerT  = (double) nSegment;


    ////////////////////////////////////////////////////////////
    // (3) draw the curve and derivatives

    // must have at least 4 points
    if (nKeyFrame < 4)
	return;

    // draw the curve
    q0 = QCurve[0];
    q1 = QCurve[1];
    q2 = QCurve[2];
    q3 = QCurve[3];

    if ( kkDotProdToggle && QuatDot( &q0,&q1 ) < 0.0 )
	QuatFlip( &q1 );
    if ( kkDotProdToggle && QuatDot( &q1,&q2 ) < 0.0 )
	QuatFlip( &q2 );
    if ( kkDotProdToggle && QuatDot( &q2,&q3 ) < 0.0 )
	QuatFlip( &q3 );

    // first point to be drawn
    (*curveFunc)( &q0, &q1, &q2, &q3, 0.0f, &qPrev );

    lastIndex = 3;

    glLineWidth(2.0);

    while (lastIndex < nKeyFrame) {


	#define _DELTA	1e-5


	////////////////////////////////////////////////
	// draw the derivative at the starting point

	if (startDevToggle) {

	    // calculate first derivative
	    (*curveFunc)( &q0, &q1, &q2, &q3, 0.0, &qTmp0 );
	    if ( !kkDotProdToggle && QuatDot( &qPrev,&qTmp0 ) < 0.0 )
		QuatFlip( &qTmp0 );
	    (*curveFunc)( &q0, &q1, &q2, &q3, (double) _DELTA, &qTmp1 );
	    if ( !kkDotProdToggle && QuatDot( &qTmp0,&qTmp1 ) < 0.0 )
		QuatFlip( &qTmp1 );
	    (*curveFunc)( &q0, &q1, &q2, &q3, ((double) (2.0*_DELTA)), &qTmp2 );
	    if ( !kkDotProdToggle && QuatDot( &qTmp1,&qTmp2 ) < 0.0 )
		QuatFlip( &qTmp2 );

	    dev1a.x = qTmp1.x - qTmp0.x;
	    dev1a.y = qTmp1.y - qTmp0.y;
	    dev1a.z = qTmp1.z - qTmp0.z;
	    UnitVec(&dev1a);

	    dev1b.x = qTmp2.x - qTmp1.x;
	    dev1b.y = qTmp2.y - qTmp1.y;
	    dev1b.z = qTmp2.z - qTmp1.z;
	    UnitVec(&dev1b);

	    // calculate 2nd derivative
	    dev2.x = dev1b.x - dev1a.x;
	    dev2.y = dev1b.y - dev1a.y;
	    dev2.z = dev1b.z - dev1a.z;
	    UnitVec(&dev2);

	    glBegin(GL_LINES);
	      glColor4fv(red_diffuse);
	      glVertex3f( qTmp0.x, qTmp0.y, qTmp0.z );
	      glVertex3f( qTmp0.x + dev1a.x * 0.2,
	                  qTmp0.y + dev1a.y * 0.2,
	                  qTmp0.z + dev1a.z * 0.2 );
	      glColor4fv(green_diffuse);
	      glVertex3f( qTmp0.x, qTmp0.y, qTmp0.z );
	      glVertex3f( qTmp0.x + dev2.x * 0.2,
	                  qTmp0.y + dev2.y * 0.2,
	                  qTmp0.z + dev2.z * 0.2 );
	    glEnd();
	}


	////////////////////////////////////////////////
	// draw this segment

	if (!markerDrawn && 0.0 <= markerT && markerT <= 1.0) {
	    k = (int) (markerT/0.04 + 0.5);
	} else {
	    k = -1;
	}

	glColor4fv(orange_diffuse);
	glBegin(GL_LINE_STRIP);
	for (t=0.0; t<1.00001; t+=0.04, k--) {

	    (*curveFunc)( &q0, &q1, &q2, &q3, t, &qOut );
	    if ( !kkDotProdToggle && QuatDot( &qPrev,&qOut ) < 0.0 )
		QuatFlip( &qOut );

	    // save the quaternion nearest to the marker
	    if (k==0) qNearMarker = qOut;

	    glVertex3f( qOut.x, qOut.y, qOut.z );
	    qPrev = qOut;
	}
	glEnd();


	////////////////////////////////////////////////
	// computer qMarker and draw marker

	if (!markerDrawn && 0.0 <= markerT && markerT <= 1.0) {

	    markerDrawn = 1;

	    (*curveFunc)( &q0, &q1, &q2, &q3, markerT, &qMarker );
	    if ( !kkDotProdToggle && QuatDot( &qNearMarker,&qMarker ) < 0.0 )
		 QuatFlip( &qMarker );

	    //printf("draw marker at t = [%f]\n",markerT);

	    if (markerToggle) {
	      ChooseProjection(Project4D, &qMarker, &QTmp);

		glLineWidth(1.0);
		glColor4fv(blue_diffuse);
		glPushMatrix();
		//		glTranslatef( qMarker.x, qMarker.y, qMarker.z );
		glTranslatef( QTmp.x, QTmp.y, QTmp.z );
		glutWireCube(0.1);
		//glutWireSphere(0.1,8,4);
		glPopMatrix();
		glLineWidth(2.0);
	    }
	}
	markerT -= 1.0;


	////////////////////////////////////////////////
	// draw the derivative at the end point

	if (endDevToggle) {

	    // calculate first derivative
	    (*curveFunc)( &q0, &q1, &q2, &q3, 1.0, &qTmp0 );
	    if ( !kkDotProdToggle && QuatDot( &qPrev,&qTmp0 ) < 0.0 )
		QuatFlip( &qTmp0 );
	    (*curveFunc)( &q0, &q1, &q2, &q3, 1.0 - (double) _DELTA, &qTmp1 );
	    if ( !kkDotProdToggle && QuatDot( &qTmp0,&qTmp1 ) < 0.0 )
		QuatFlip( &qTmp1 );
	    (*curveFunc)( &q0, &q1, &q2, &q3, 1.0 - ((double) (2.0*_DELTA)), &qTmp2 );
	    if ( !kkDotProdToggle && QuatDot( &qTmp1,&qTmp2 ) < 0.0 )
		QuatFlip( &qTmp2 );

	    dev1a.x = qTmp1.x - qTmp0.x;
	    dev1a.y = qTmp1.y - qTmp0.y;
	    dev1a.z = qTmp1.z - qTmp0.z;
	    UnitVec(&dev1a);

	    dev1b.x = qTmp2.x - qTmp1.x;
	    dev1b.y = qTmp2.y - qTmp1.y;
	    dev1b.z = qTmp2.z - qTmp1.z;
	    UnitVec(&dev1b);

	    // calculate 2nd derivative
	    dev2.x = dev1b.x - dev1a.x;
	    dev2.y = dev1b.y - dev1a.y;
	    dev2.z = dev1b.z - dev1a.z;
	    UnitVec(&dev2);

	    glBegin(GL_LINES);
	      glColor4fv(red_diffuse);
	      glVertex3f( qTmp0.x, qTmp0.y, qTmp0.z );
	      glVertex3f( qTmp0.x + dev1a.x * 0.2,
	                  qTmp0.y + dev1a.y * 0.2,
	                  qTmp0.z + dev1a.z * 0.2 );
	      glColor4fv(green_diffuse);
	      glVertex3f( qTmp0.x, qTmp0.y, qTmp0.z );
	      glVertex3f( qTmp0.x + dev2.x * 0.2,
	                  qTmp0.y + dev2.y * 0.2,
	                  qTmp0.z + dev2.z * 0.2 );
	    glEnd();
	}


	////////////////////////////////////////////////
	// next segment

	if (curveType == _BEZIER) {

	    lastIndex += 3;
	    if (lastIndex >= nKeyFrame)
		break;

	    q0 = q3;
	    q1 = QCurve[lastIndex-2];
	    q2 = QCurve[lastIndex-1];
	    q3 = QCurve[lastIndex];

	    if ( kkDotProdToggle && QuatDot( &q0,&q1 ) < 0.0 )
		QuatFlip( &q1 );
	    if ( kkDotProdToggle && QuatDot( &q1,&q2 ) < 0.0 )
		QuatFlip( &q2 );
	    if ( kkDotProdToggle && QuatDot( &q2,&q3 ) < 0.0 )
		QuatFlip( &q3 );
 
	} else {

	    lastIndex++;
	    if (lastIndex >= nKeyFrame)
		break;

	    q0 = q1;
	    q1 = q2;
	    q2 = q3;
	    q3 = QCurve[lastIndex];

	    if ( kkDotProdToggle && QuatDot( &q2,&q3 ) < 0.0 )
		QuatFlip( &q3 );
	}


    } // end of while loop

    glLineWidth(1.0);
}


void
drawAxes()
{
    double r,s;


    if (!viewAxesToggle)
	return;

    r = 0.033;
    glLineWidth(3.0);

     // drawAxes
    glColor3f(1.0f,0.0f,0.0f);
    DrawTube(&zero,&XAxis,r);
    glColor3f(0.2f,1.0f,0.2f);
    DrawTube(&zero,&YAxis,r);
    glColor3f(0.0f,0.0f,1.0f);
    DrawTube(&zero,&ZAxis,r);

    s = 0.3;
    glBegin(GL_TRIANGLES);
      glColor3f(1.0f,1.0f,0.0f);
      glNormal3f(ZAxis.x,ZAxis.y,ZAxis.z);
      glVertex3f(0.0f,0.0f,0.0f); glVertex3f(s*XAxis.x,s*XAxis.y,s*XAxis.z);
      glVertex3f(s*YAxis.x,s*YAxis.y,s*YAxis.z);
      glColor3f(0.0f,1.0f,1.0f);
      glNormal3f(XAxis.x,XAxis.y,XAxis.z);
      glVertex3f(0.0f,0.0f,0.0f); glVertex3f(s*YAxis.x,s*YAxis.y,s*YAxis.z);
      glVertex3f(s*ZAxis.x,s*ZAxis.y,s*ZAxis.z);
      glColor3f(1.0f,0.0f,1.0f);
      glNormal3f(YAxis.x,YAxis.y,YAxis.z);
      glVertex3f(0.0f,0.0f,0.0f); glVertex3f(s*ZAxis.x,s*ZAxis.y,s*ZAxis.z);
      glVertex3f(s*XAxis.x,s*XAxis.y,s*XAxis.z);
    glEnd();
}

void
drawColorCube()
{
    const float corners[][3] = {
	{ -0.5f, -0.5f,  0.5f },
	{  0.5f, -0.5f,  0.5f },
	{  0.5f,  0.5f,  0.5f },
	{ -0.5f,  0.5f,  0.5f },
	{ -0.5f, -0.5f, -0.5f },
	{  0.5f, -0.5f, -0.5f },
	{  0.5f,  0.5f, -0.5f },
	{ -0.5f,  0.5f, -0.5f }
    };

    glDisable(GL_LIGHTING);

    glBegin(GL_QUADS);

      // X-faces
      glColor3f(1.0,0.0,0.0);
      glVertex3fv(corners[1]);
      glVertex3fv(corners[5]);
      glVertex3fv(corners[6]);
      glVertex3fv(corners[2]);
      glColor3f(0.0,1.0,1.0);
      glVertex3fv(corners[0]);
      glVertex3fv(corners[3]);
      glVertex3fv(corners[7]);
      glVertex3fv(corners[4]);

      // Y-faces
      glColor3f(0.0,1.0,0.0);
      glVertex3fv(corners[2]);
      glVertex3fv(corners[6]);
      glVertex3fv(corners[7]);
      glVertex3fv(corners[3]);
      glColor3f(1.0,0.0,1.0);
      glVertex3fv(corners[0]);
      glVertex3fv(corners[4]);
      glVertex3fv(corners[5]);
      glVertex3fv(corners[1]);

      // Z-faces
      glColor3f(0.0,0.0,1.0);
      glVertex3fv(corners[0]);
      glVertex3fv(corners[1]);
      glVertex3fv(corners[2]);
      glVertex3fv(corners[3]);
      glColor3f(1.0,1.0,0.0);
      glVertex3fv(corners[4]);
      glVertex3fv(corners[7]);
      glVertex3fv(corners[6]);
      glVertex3fv(corners[5]);

    glEnd();
}

void
drawObject()
{
    GLdouble mat[4][4];

    // set the matrices
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(-1.0,1.0,-1.0,1.0,_NEAR,_FAR);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.6f,-0.6f,-7.0f);
    glScalef(0.5f,0.5f,0.5f);

    // rotate the object according to qMarker
    QuatToMat(&qMarker,mat);
    glMultMatrixd((GLdouble *)mat);


    // put in the object
    switch (objectID) {
      case 0 :
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,material_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	//glColor4fv(orange_diffuse);
	glutSolidTeapot(0.5);
	break;
      case 9 :
	drawColorCube();
	break;
    }

    // reset the matrices
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

void
Display(void)
{
    int   i,j;
    double tmpw;
    Point tmp,v1,v2,v3,tcrs;

    /*   GLint wid,hi; */
    /*   wid = glutGet(GLUT_WINDOW_WIDTH); */
    /*   hi = glutGet(GLUT_WINDOW_HEIGHT); */
    /*   printf("Display \n"); */


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    /* Surrounding decorations */
    if (worldAxesToggle) {

	glDisable(GL_LIGHTING);
	/* glColor3f(0.0,0.0,1.0); glutWireCube(1.0); */
	glLineWidth(2.0);

	// Draw Axes
	glBegin(GL_LINES);
	  glColor3f(1.0,0.0,0.0);
	  glVertex3f(0.0,0.0,0.0); glVertex3f(1.5,0.0,0.0);
	  glColor3f(0.0,1.0,0.0);
	  glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,1.5,0.0);
	  glColor3f(0.0,0.0,1.0);
	  glVertex3f(0.0,0.0,0.0); glVertex3f(0.0,0.0,1.5);
	glEnd();

	glEnable(GL_LIGHTING);
    }

    if (sphereToggle) {
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,black_diffuse);
	glLineWidth(1.0);
	glutWireSphere(1.0, 16,8);
    }

    /* Central Model Objects  */
    switch (objectID){

      case 0:
      case 1:
      case 9:

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,material_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material_shininess);

	glEnable(GL_LIGHTING);
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_ambient);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,1.0);

	/* Switch projections based on XYZ =>  Project4D = 0; */
	ChooseProjection(Project4D, &QTotal[currQTotal], &QTmp);
	tmp.x = QTmp.x;	tmp.y = QTmp.y;	tmp.z = QTmp.z; tmpw = QTmp.w;

			 //	  tmp.x = QTotal[currQTotal].x;
			 //	  tmp.y = QTotal[currQTotal].y;
			 //	  tmp.z = QTotal[currQTotal].z;

	//	(QTotal[currQTotal].w > 0.0) ?

	(tmpw > 0.0) ?
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse) :
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,bla_diffuse);
	DrawTube(&zero, &tmp, .075f);

      glDisable(GL_LIGHTING);

	// draw comet trail
	drawCometTrail();

	// draw keyFrames
	drawQuatCurve();

	// draw axes
	drawAxes();

	// show Q0 as a bar
	ShowQ0(tmpw); 

      glEnable(GL_LIGHTING);

	// show teapot or colored cube
	if (objectID == 0 || objectID == 9)
	    drawObject();

        break;

      case 2:

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,shoulder_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material_shininess);
	glPolygonMode(GL_FRONT,GL_FILL);
	glPolygonMode(GL_BACK,GL_FILL);
	  /* glPolygonMode(GL_BACK,GL_LINE); */
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glShadeModel(GL_SMOOTH);

	for (i=0; i<6; i++) {
	    glBegin(GL_QUAD_STRIP);
	    for (j=0; j<6; j++) {
		Point4ToVec(&shoulder[i][j],&v1);
		Point4ToVec(&shoulder[i+1][j],&v2);
		Point4ToVec(&shoulder[i][j+1],&v3);
		NCross3(&v1,&v2,&v3,&tcrs);
		glNormal3d(tcrs.x,tcrs.y,tcrs.z);
		glVertex3d(shoulder[i][j].x,    shoulder[i][j].y,    shoulder[i][j].z);
		glVertex3d(shoulder[i+1][j].x,  shoulder[i+1][j].y,  shoulder[i+1][j].z);
		glVertex3d(shoulder[i][j+1].x,  shoulder[i][j+1].y,  shoulder[i][j+1].z);
		glVertex3d(shoulder[i+1][j+1].x,shoulder[i+1][j+1].y,shoulder[i+1][j+1].z);
	    }
	    glEnd();
	}

	/* glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); */
	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); 
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,black_diffuse);

	for (i=0; i<6; i++) {
	    glBegin(GL_QUAD_STRIP);
	    for (j=0; j<6; j++) {
		glVertex3d(shoulder[i][j].x,    shoulder[i][j].y,    shoulder[i][j].z);
		glVertex3d(shoulder[i+1][j].x,  shoulder[i+1][j].y,  shoulder[i+1][j].z);
		glVertex3d(shoulder[i][j+1].x,  shoulder[i][j+1].y,  shoulder[i][j+1].z);
		glVertex3d(shoulder[i+1][j+1].x,shoulder[i+1][j+1].y,shoulder[i+1][j+1].z);
	    }
	    glEnd();
	}
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
	break;

      case 3:

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material_shininess);

	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
	/* glPolygonMode(GL_BACK,GL_LINE); */
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glShadeModel(GL_SMOOTH);

	for (i=0; i<6; i++) {
	    glBegin(GL_QUAD_STRIP);
	    for (j=0; j<6; j++) {
		Point4ToVec(&elbow[i][j],  &v1);
		Point4ToVec(&elbow[i+1][j],&v2);
		Point4ToVec(&elbow[i][j+1],&v3);
		NCross3(&v1,&v2,&v3,&tcrs);
		glNormal3d(tcrs.x,tcrs.y,tcrs.z);
		glVertex3d(elbow[i][j].x,    elbow[i][j].y,    elbow[i][j].z);
		glVertex3d(elbow[i+1][j].x,  elbow[i+1][j].y,  elbow[i+1][j].z);
		glVertex3d(elbow[i][j+1].x,  elbow[i][j+1].y,  elbow[i][j+1].z);
		glVertex3d(elbow[i+1][j+1].x,elbow[i+1][j+1].y,elbow[i+1][j+1].z);
	    }
	    glEnd();
	}

	/* glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE); */
	glLineWidth(1.0);
	glPolygonMode(GL_FRONT_AND_BACK,GL_LINE); 
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,white_diffuse);

	for (i=0; i<6; i++) {
	    glBegin(GL_QUAD_STRIP);
	    for (j=0;j<6;j++) {
		glVertex3d(elbow[i][j].x,    elbow[i][j].y,    elbow[i][j].z);
		glVertex3d(elbow[i+1][j].x,  elbow[i+1][j].y,  elbow[i+1][j].z);
		glVertex3d(elbow[i][j+1].x,  elbow[i][j+1].y,  elbow[i][j+1].z);
		glVertex3d(elbow[i+1][j+1].x,elbow[i+1][j+1].y,elbow[i+1][j+1].z);
	    }
	   glEnd();
	}
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL); 
	break;

      case 4:			/* 2D unit vector */

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black_diffuse);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,less_shininess);

	(ZAxis.z > 0.0) ?
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse) :
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,bla_diffuse);
	DrawTube(&zero, &ZAxis, .05f);
	ShowQ0(ZAxis.z);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glRotatef(90.0,1.0,0.0,0.0); 
	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,black_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	glutSolidTorus(.01,1.02,8,32);
	glPopMatrix();
	break;

      case 5:

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,black_diffuse);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,less_shininess);

	(ZAxis.z > 0.0) ?
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse) :
	    glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,bla_diffuse);
	DrawTube(&zero, &ZAxis, .05f);
	ShowQ0(ZAxis.z);
	break;

      case 8:

	glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,elbow_diffuse);
	glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,material_specular);
	glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,material_shininess);
	glCallList(cylList);	/* glCallList(list), (list+1), etc */
	glCallList(cylList+1);
	break;

      default:

	glutSolidIcosahedron();
	break;
    }

    glutSwapBuffers();
}




/***********************************************************************/
/* Other Callbacks                                                     */
/***********************************************************************/

// For Movie Frame Advancing
void
specKey(int key, int x, int y)
{
    // any least 4 keyframes
    if (nKeyFrame < 4)
	return;

    if (key == GLUT_KEY_RIGHT) {
	markerFrame += 0.02;
	if (markerFrame > nKeyFrame - 3)
	    markerFrame = (double) (nKeyFrame - 3);
	glutPostRedisplay();
	//printf("marker at t = %f\n",markerFrame);
    }
    else
    if (key == GLUT_KEY_LEFT) {
	markerFrame -= 0.02;
	if (markerFrame < 0.0)
	    markerFrame = 0.0;
	glutPostRedisplay();
	//printf("marker at t = %f\n",markerFrame);
    }
}

void
Keyboard(unsigned char key, int x, int y)
{
    int isUp;

    void helpMenu();


    // from upper case to lower case

    if ('A' <= key && key <= 'Z') {
	key  = key-'A'+'a';
	isUp = 1;
    } else
	isUp = 0;


    // each key

    switch(key) {

      case 'h' :
	helpMenu();
	break;

      case 'q' : case 27 :
	fprintf(stderr,"Normal exit.\n");
	exit(EXIT_SUCCESS);
	break;

      case 't' :
	if (isUp)
	    trailToggle = (trailToggle - 1 + 3) % 3;
	else
	    trailToggle = (trailToggle + 1) % 3;
	if (trailToggle == 0) fprintf(stderr,"Comet trail off.\n");
	if (trailToggle == 1) fprintf(stderr,"Comet trail on (points).\n");
	if (trailToggle == 2) fprintf(stderr,"Comet trail on (lines).\n");
	break;

      case '+' :
	trailLength++;
	if (trailLength > _MAX_TRAIL_LENGTH) trailLength = _MAX_TRAIL_LENGTH;
	printf("%d points on comet trail.\n",trailLength);
	break;

      case '-' :
	trailLength--;
	if (trailLength < 0) trailLength = 0;
	printf("%d points on comet trail.\n",trailLength);
	break;

      case 'a' :
	if (isUp)
	  {	fprintf(stderr,"Toggle View Axes.\n");
	viewAxesToggle = ! viewAxesToggle; }
	else
	{fprintf(stderr,"Toggle World Axes.\n");
	worldAxesToggle = ! worldAxesToggle;}
	break;

      case 'w' :
	fprintf(stderr,"Project XYZ.\n");
        Project4D = 0;
	break;
      case 'x' :
	fprintf(stderr,"Project WYZ.\n");
        Project4D = 1;
	break;
      case 'y' :
	fprintf(stderr,"Project WXZ.\n");
        Project4D = 2;
	break;
      case 'z' :
	fprintf(stderr,"Project WXY.\n");
        Project4D = 3;
	break;

      case 's' :
	fprintf(stderr,"Toggle Wire Sphere.\n");
	sphereToggle = ! sphereToggle;
	break;

      case 'p' :
	fprintf(stderr,"Toggle Quaternion Curve.\n");
	curveToggle = ! curveToggle;
	break;

      case 'f' :
	kkDotProdToggle = ! kkDotProdToggle;
	if (kkDotProdToggle)
	    fprintf(stderr,"Keyframe by keyframe dot product.\n");
	else
	    fprintf(stderr,"Frame by frame dot product.\n");
	break;

      case 'm' :
	fprintf(stderr,"toggle the frame marker on the Quaternion Curve.\n");
	markerToggle = ! markerToggle;
	break;

      case '[' :
	fprintf(stderr,"showing derivatives at starting points of each Quaternion Curve segments.\n");
	startDevToggle = ! startDevToggle;
	break;

      case ']' :
	fprintf(stderr,"showing derivatives at end points of each Quaternion Curve segments.\n");
	endDevToggle = ! endDevToggle;
	break;

      case 'k' :
	if (nKeyFrame >= _MAX_KEYFRAME)
	    fprintf(stderr,"Maximum number of Keyframes exceeded.\n");
	else {
	    fprintf(stderr,"Keyframe added.\n");
	    QCurve[nKeyFrame++] = QTotal[currQTotal];
	}
	break;

      case 'd' :
	if (nKeyFrame == 0)   
	    fprintf(stderr,"No more Keyframe to be deleted.\n");
	else {
	    fprintf(stderr,"Keyframe deleted.\n");
	    nKeyFrame--;
	}
	break;

      case 'v' : 
	if (isUp)
	    curveType = (curveType-1+3) % 3;
	else
	    curveType = (curveType+1) % 3;
	setCurveFunc();
	break;

      case 'i' :
	fprintf(stderr,"Current Quaternion (w,x,y,z) : ");
	printQuatN(stderr,QTotal[currQTotal]);
	if (markerToggle) {
	    printf("Marker at t = %f (Quat : ",markerFrame);
	    printQuat(stderr,qMarker);
	    printf(")\n");
	}
	break;

      case 'r' :
	fprintf(stderr,"Coord Reset.\n");
	resetAxes();
	resetQuat();
	resetView();
	break;

      case 'c' :
	fprintf(stderr,"Clear.\n");
	resetAxes();
	resetTrail();
	resetKeyFrame();
	resetView();
	break;

      case '1' : case '2' : case '3' : case '4' : case '5' : 
      case '6' : case '7' : case '8' : case '9' : case '0' :
	objectID = key - '0';					// 0 - 9
	fprintf(stderr,"Show object [%d]\n",objectID);
	break;
    }

    glutPostRedisplay();
}


/* Reshape to get coord system right */

void
Reshape(int w, int h)
{
    wid = w;
    hi = h;

    printf("\nEntered  Reshape with: %d %d; glutGet: %d %d\n", w, h,
	glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));

    glutReshapeWindow(Max(w,200), Max(h,200));
    glViewport(0, 0, Max(w,200), Max(h,200));

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    /* gluOrtho2D(0.0,(double)Max(w,200), (double)Max(h,200), 0.0);  */
    glFrustum(-1.0,1.0,-1.0,1.0,_NEAR,_FAR);
}




/*--------------------------------------------------------------*/
/* Event Handler: Mouse Click function                          */
/*--------------------------------------------------------------*/

void
Button(int button, int state, int x, int y)
{
    y = hi - 1 - y;

    buttonCode     = buttonCode^(1<<(button));
    mouseModifiers = glutGetModifiers();

    xold = x;
    yold = y;

    /* printf("Code: %d; button: %d %s, %d, %d\n", buttonCode,
	button, state == GLUT_UP ? "up" : "down", x, y);
	GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_DOWN, GLUT_UP
    */
}


/*--------------------------------------------------------------*/
/* Event Handler: Mouse Motion function                          */
/*--------------------------------------------------------------*/

/* Implement Rolling Ball */

void
Motion(int x, int y)
{
    float  mat[16];
    double tx,ty,nx,ny,scale,angle,rad,hsin;
    Point  n;
    double rotor[3][3];

    y    = hi - 1 - y;
    dx   = x - xold;
    dy   = y - yold;
    xold = x;
    yold = y;
  
    glMatrixMode(GL_MODELVIEW);


    switch (buttonCode) {


      ////////////////////////////////////////
      // LEFT BUTTON

      case 0x1:


	///////////////////////////////////
	// + SHIFT : movie control

	if (mouseModifiers == GLUT_ACTIVE_SHIFT) {

	    markerFrame += dx / ((float) (wid/4));

	    if (markerFrame > nKeyFrame - 3)
		markerFrame = (float) (nKeyFrame - 3);
	    if (markerFrame < 0.0)
		markerFrame = 0.0;

	    glutPostRedisplay();
	    //printf("marker at t = %f\n",markerFrame);
	}


	///////////////////////////////////
	// Rotating the viewing frame

	else {

	    nx    = -dy;
	    ny    =  dx;
	    scale =  sqrt(nx*nx + ny*ny);
	    if (scale < 1e-7)
		return;

	    nx    = nx / scale;
	    ny    = ny / scale;
	    angle = scale * _ROTSCALE;

	    if (objectID == 4) {
		nx = 0.0;

		ny = Sign(dx);
		if (ny == 0.0) {
		    angle = 0.0;
		    ny    = 1.0;
		} else {
		    angle = abs(dx) * _ROTSCALE;
		}
	    }
	    rad = DegToRad*angle;

	    /* Rotate Current Axes */
	    n.x = nx;
	    n.y = ny;
	    n.z = 0.0;

	    MakeRotMat3(&n, rad,  rotor);
	    MatTimesPoint3(&XAxis, rotor, &XAxis);
	    MatTimesPoint3(&YAxis, rotor, &YAxis);
	    MatTimesPoint3(&ZAxis, rotor, &ZAxis);

	    /* Implement corresponding quaternion transform */
	    hsin   = sin(rad*0.5);
	    QTmp.x = nx*hsin;
	    QTmp.y = ny*hsin;
	    QTmp.z = 0.0;
	    QTmp.w = sqrt(1.0 -hsin*hsin);

	    // accumulate QTotal
	    nextQTotal = (currQTotal+1) % _MAX_TRAIL_LENGTH;
	    QuatMult(&QTmp,&(QTotal[currQTotal]),&(QTotal[nextQTotal]));
	    currQTotal = nextQTotal;
	}

	break;
    

      ////////////////////////////////////////
      // MIDDLE BUTTON

      case 0x2:

	nx    = dx;
	scale = sqrt(nx*nx);
	angle = Sign(nx) * scale * _ROTSCALE;
	rad   = DegToRad * angle;

	/* Rotate Current Axes */
	n.x = 0.0;
	n.y = 0.0;
	n.z = 1.0;
	MakeRotMat3(&n, rad,  rotor);
	MatTimesPoint3(&XAxis, rotor, &XAxis);
	MatTimesPoint3(&YAxis, rotor, &YAxis);
	MatTimesPoint3(&ZAxis, rotor, &ZAxis);

	/* Implement corresponding quaternion transform */
	hsin   = sin(rad*0.5);
	QTmp.x = 0.0;
	QTmp.y = 0.0;
	QTmp.z = hsin;
	QTmp.w = sqrt(1.0 -hsin*hsin);

	// accumulate QTotal
	nextQTotal = (currQTotal+1) % _MAX_TRAIL_LENGTH;
	QuatMult(&QTmp,&(QTotal[currQTotal]),&(QTotal[nextQTotal]));
	currQTotal = nextQTotal;

	break;


      ////////////////////////////////////////
      // RIGHT BUTTON

      case 0x4:

	/* MUST HAVE math.h or this is REALLY bad */
	nx    = -dy;
	ny    =  dx;
	scale =  sqrt(nx*nx + ny*ny);
	if (scale < 1e-7)
	    return;

	nx    = nx / scale;
	ny    = ny / scale;
	angle = scale * _ROTSCALE;
	rad   = DegToRad * angle;

	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	glLoadIdentity();
	glTranslatef(XC,YC,ZC);
	glRotated(angle,nx,ny,0.0); 
	glTranslatef(-XC,-YC,-ZC);
	glMultMatrixf(mat);

	break;


      ////////////////////////////////////////
      // LEFT+MIDDLE BUTTON

      case 0x3:

	// Translation XZ
	tx = 0.01 * dx;
	ty = 0.01 * dy;
	XC = XC + tx;
	ZC = ZC - ty;
	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	glLoadIdentity();
	glTranslated(tx,0.0,-ty);
	glMultMatrixf(mat);

	break;


      ////////////////////////////////////////
      // BUTTON?

      case 0x20:

	printf("22!!!\n");

	nx    = dx;
	scale = sqrt(nx*nx);
	angle = Sign(nx) * scale * _ROTSCALE;
	rad   = DegToRad * angle;

	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	glLoadIdentity();
	glTranslatef(XC,YC,ZC);
	glRotated(angle,0.0,0.0,1.0); 
	glTranslatef(-XC,-YC,-ZC);
	glMultMatrixf(mat);

	/* Implement corresponding quaternion transform */
	hsin   = sin(rad);
	QTmp.x = 0.0;
	QTmp.y = 0.0;
	QTmp.z = hsin;
	QTmp.w = sqrt(1.0 -hsin*hsin);

	// accumulate QTotal
	nextQTotal = (currQTotal+1) % _MAX_TRAIL_LENGTH;
	QuatMult(&QTmp,&(QTotal[currQTotal]),&(QTotal[nextQTotal]));
	currQTotal = nextQTotal;

	break;

      case 0x21:

	printf("21!!!\n");
	tx = 0.01 * dx;
	ty = 0.01 * dy;
	XC = XC + tx;
	YC = YC + ty;
	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	glLoadIdentity();
	glTranslated(tx, ty, 0.0);
	glMultMatrixf(mat);

	break;

      case 0x22:

	printf("22!!!\n");
	tx = 0.01 * dx;
	ty = 0.01 * dy;
	XC = XC + tx;
	ZC = ZC - ty;
	glGetFloatv(GL_MODELVIEW_MATRIX,mat);
	glLoadIdentity();
	glTranslated(tx,0.0,-ty);
	glMultMatrixf(mat);
	break;
    }

    glutPostRedisplay();
}


void
helpMenu()
{
    fprintf(stderr,"\n");
    fprintf(stderr,"Help\n");
    fprintf(stderr,"----\n");
    fprintf(stderr,"\n");

    fprintf(stderr,"[Mouse]\n");
    fprintf(stderr,"  LEFT    : x-y rotation\n");
    fprintf(stderr,"   +SHIFT : change views along the quaternion curve\n");
    fprintf(stderr,"  MID     :  z  rotation\n");
    fprintf(stderr,"  RIGHT   : world rotation\n");
    fprintf(stderr,"  L+M     : world x-z translation\n");
    fprintf(stderr," \n");

    fprintf(stderr,"[Keyboard]\n");
    fprintf(stderr,"  'h'     : this help\n");
    fprintf(stderr,"  'q'/ESC : quit\n");
    fprintf(stderr,"  't'     : toggle comet trail\n");
    fprintf(stderr,"  +/-     : comet trail length\n");
    fprintf(stderr,"  'a'     : toggle world axes\n");
    fprintf(stderr,"  'A'     : toggle viewing frame axes\n");
    fprintf(stderr,"  's'     : toggle wire sphere\n");
    fprintf(stderr,"  'p'     : toggle quaternion curve display\n");
    fprintf(stderr,"  'f'     : toggle frame-by-frame or keyframe-by-keyframe dot product\n");
    fprintf(stderr,"  'm'     : toggle marker on quaternion curve\n");
    fprintf(stderr,"  '['     : toggle quaternion curve 1st derivative\n");
    fprintf(stderr,"  ']'     : toggle quaternion curve 2nd derivative\n");
    fprintf(stderr,"  'k'     : add a keyframe\n");
    fprintf(stderr,"  'd'     : delete the previous added keyframe\n");
    fprintf(stderr,"  'v'     : toggle quaternion curve type\n");
    fprintf(stderr,"  'i'     : print out current quaternion\n");
    fprintf(stderr,"  'r'     : reset\n");
    fprintf(stderr,"  'c'     : clear keyframe/trail\n");
    fprintf(stderr,"  '1'     : no object\n");
    fprintf(stderr,"  '9'     : draw color cube\n");
    fprintf(stderr,"  '0'     : draw teapot\n");
    fprintf(stderr," '2-8'    : objects for other demos\n\n");
    fprintf(stderr,"  'w'     : default: XYZ projection (omit q0) \n");
    fprintf(stderr,"  'x'     : WYZ projection (omit qx) \n");
    fprintf(stderr,"  'y'     : WXZ projection (omit qy) \n");
    fprintf(stderr,"  'z'     : WXY projection (omit qz) \n");
    fprintf(stderr," \n");
}




/*--------------------------------------------------------------*/
/* Main program                                                 */
/*--------------------------------------------------------------*/

int
main(int argc, char **argv)
{
    /* GLUT initialization: */
    glutInit(&argc, argv);
    glutInitWindowSize(512, 512);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);

    /* create a new window and give a title to it */
    glutCreateWindow("Quaternion Multiplication demo");
    helpMenu();

    /* Initialize Non-OpenGL and OpenGL stuff */
    myInit();
    myGLInit();

    /* Post Event Handlers */
    glutReshapeFunc(Reshape);
    glutDisplayFunc(Display);

    glutKeyboardFunc(Keyboard);		/* Keyboard: Default exit = ESC key */
    glutSpecialFunc(specKey);
    glutMouseFunc(Button);
    glutMotionFunc(Motion);
    glutPassiveMotionFunc(NULL);

    glutMainLoop();			/* Does not return */

    return(EXIT_SUCCESS);		/* Unix shell success code
                                           see /usr/include/stdlib.h */

} /* main() */
