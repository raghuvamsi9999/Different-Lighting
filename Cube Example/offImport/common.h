#ifndef __common_h__
#define __common_h__

#include <iostream>
#include <fstream>
#include <vector>
#include "Vector3.h"
#include "Vector4.h"

#define PI 3.1415926535

using namespace std;

typedef struct {
    float x;
    float y;
    float z;
} vec3;
typedef struct {
    float r;
    float g;
    float b;
    float a;
} color4;

typedef float mat4[16];

struct LoadData{
	std::vector<Vector3<float> > verts;
	std::vector<Vector3<float> > vertnormal;
	std::vector<Vector3<float> > vertcolor;
	std::vector<Vector3<unsigned int> > tris;
    std::vector<Vector3<float> > tricolor;
    std::vector<Vector3<float> > trinormal;
    vec3 min,max;
} ;

typedef struct
{
	unsigned int *pIndices, nVertices, nTris;
	float *pVertices;
	float *pNormals;
	float *pColors;
	float *pTextures;
	float Bbox[6];
}graphicData;

typedef struct
{
	unsigned int type;
	float	 angle;
  float	 t[3];
}TransData;

#endif
