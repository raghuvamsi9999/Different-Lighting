
#include "off_io.h"
#include <cassert>
#include <sstream>
#include <string>
#include <iostream>
#include <istream>
#include <vector>
#include <math.h>
#define DEBUG 0
 
using namespace std;

bool off_io::Load(std::istream & is, LoadData *iloadData){
  bool success = ReadHeader(is);
  if(!success) { 
  	if(DEBUG) std::cerr << "Error in header\n";
		return false; 
	}

  success = ReadDataCount(is);
  if(!success) { return false; }
  
  success = ReadData(is, iloadData);
  if(!success) { return false; }

  if(DEBUG) std::cerr << "OFFIO : Vertices = " << iloadData->verts.size() 
											<<" , tris = "<< iloadData->tris.size()<<endl ;
  
  iloadData->min.x = 0.0; iloadData->min.y = 0.0; iloadData->min.z = 0.0;
  iloadData->max.x = 0.0; iloadData->max.y = 0.0; iloadData->max.z = 0.0;
  
  calculateBoundingbox(iloadData);
  calculateNormal(iloadData);
  calculateVertexColor(iloadData);
  if(DEBUG) std::cerr << "OFFIO : Min= ("<< iloadData->min.x <<", "<< iloadData->min.y 
											<<", "<< iloadData->min.z <<") Max =("<< iloadData->max.x <<", "
											<< iloadData->max.y <<", "<< iloadData->max.z <<")"<<endl ;
	return true;
}

bool off_io::ReadHeader(std::istream & is){
  std::string lineBuf, buf;
  std::stringstream s;
  bool good = false;
  // just read to the first line starting with a 'O' in 'OFF'
  if(!is.eof()){
    getline(is, lineBuf);
    s << lineBuf;
    s >> buf;
    if(!buf.compare("OFF"))
    {
        good = true;
    }
  }
  if(is.good() && good)
    return true;
  else
    return false;
}

bool off_io::ReadDataCount(std::istream & is){
  std::string lineBuf;
  std::stringstream s;
  int nEdges = 0;
  // just read to the second line, get the no of vertices and faces.
  if(!is.eof()){
      getline(is, lineBuf);
      s << lineBuf;
      s >> nVerts >> nFaces >> nEdges;
  }
  if(is.good())
    return true;
  else
    return false;
}

bool off_io::ReadData(std::istream & is, LoadData *iloadData){
  std::string lineBuf;
  unsigned int vertexcount = 0, facecount = 0, noofconnectedface = 0;
  while(!is.eof() && vertexcount < nVerts){
  	getline(is, lineBuf); // get the line
        iloadData->verts.push_back(Vector3<float>(lineBuf));
        vec3 N = {0.0,0.0,0.0};
        vertnormalcalc.push_back(N);
        vec3 C = {0.0,0.0,0.0};
        vertcolorcalc.push_back(C);
        noofconnectedfaces.push_back(noofconnectedface);
        vertexcount +=1;
  }
  
  while(!is.eof() && facecount < nFaces){
        getline(is, lineBuf); // get the line
  		std::stringstream s;
        s << lineBuf;
        int geom = 0;
        s >> geom;
        switch (geom) {
        case 3:					// TRI
            unsigned int t1,t2,t3;
            s >> t1;s >> t2;s >> t3;
            iloadData->tris.push_back(Vector3<unsigned int>(t1, t2, t3));
            float tc1,tc2,tc3;
            s >> tc1;s >> tc2;s >> tc3;
            iloadData->tricolor.push_back(Vector3<float>(tc1, tc2, tc3));
              break;
        case 4:					// QUAD
            unsigned int q1,q2,q3,q4;
            s >> q1;s >> q2;s >> q3;s >> q4;
            iloadData->tris.push_back(Vector3<unsigned int>(q1,q2,q3));
            iloadData->tris.push_back(Vector3<unsigned int>(q1,q3,q4));
            float qc1,qc2,qc3;
            s >> qc1;s >> qc2;s >> qc3;
            iloadData->tricolor.push_back(Vector3<float>(qc1,qc2,qc3));
            iloadData->tricolor.push_back(Vector3<float>(qc1,qc2,qc3));
          break;
       case 5:
            unsigned int p1,p2,p3,p4,p5;
            s >> p1;s >> p2;s >> p3;s >> p4;s >> p5;
            iloadData->tris.push_back(Vector3<unsigned int>(p1, p2, p3));
            iloadData->tris.push_back(Vector3<unsigned int>(p1, p3, p4));
            iloadData->tris.push_back(Vector3<unsigned int>(p1, p4, p5));
            float pc1,pc2,pc3;
            s >> pc1;s >> pc2;s >> pc3;
            iloadData->tricolor.push_back(Vector3<float>(pc1, pc2, pc3));
            iloadData->tricolor.push_back(Vector3<float>(pc1, pc2, pc3));
            iloadData->tricolor.push_back(Vector3<float>(pc1, pc2, pc3));
          break;
        }
        facecount+=1;
  }
  
  return true;
}

bool off_io::calculateBoundingbox(LoadData *iloadData)
{
	unsigned int nVerts = iloadData->verts.size();
	for(unsigned int i=0; i<nVerts; i++)
	{
		Vector3<float> v = iloadData->verts[i];
		if(v[0] < iloadData->min.x) iloadData->min.x = v[0] ;
		if(v[1] < iloadData->min.y) iloadData->min.y = v[1] ;
		if(v[2] < iloadData->min.z) iloadData->min.z = v[2] ;
		if(v[0] > iloadData->max.x) iloadData->max.x = v[0] ;
		if(v[1] > iloadData->max.y) iloadData->max.y = v[1] ;
		if(v[2] > iloadData->max.z) iloadData->max.z = v[2] ;
	}
	return true;
}
bool off_io::calculateNormal(LoadData *iloadData)
{
	unsigned int ntris = iloadData->tris.size();
	for(unsigned int i=0; i<ntris; i++)
	{
		Vector3<unsigned int> vid = iloadData->tris[i];
		Vector3<float> v[3] = {iloadData->verts[vid[0]],iloadData->verts[vid[2]],iloadData->verts[vid[1]]};
		
		vec3 U = {v[1][0]-v[0][0], v[1][1]-v[0][1], v[1][2]-v[0][2]};
		vec3 V = {v[2][0]-v[1][0], v[2][1]-v[1][1], v[2][2]-v[1][2]};
		vec3 N = {U.y*V.z - U.z*V.y, U.z*V.x - U.x*V.z, U.x*V.y - U.y*V.x};
		float len = sqrt(N.x*N.x + N.y*N.y + N.z*N.z);
		N.x /= len;
		N.y /= len;
		N.z /= len;
		
		iloadData->trinormal.push_back(Vector3<float>(N.x, N.y, N.z));

		// vertex normal calculations		
		vertnormalcalc[vid[0]].x += N.x;
		vertnormalcalc[vid[0]].y += N.y;
		vertnormalcalc[vid[0]].z += N.z;
		noofconnectedfaces[vid[0]] += 1;
		vertnormalcalc[vid[1]].x += N.x;
		vertnormalcalc[vid[1]].y += N.y;
		vertnormalcalc[vid[1]].z += N.z;
		noofconnectedfaces[vid[1]] += 1;
		vertnormalcalc[vid[2]].x += N.x;
		vertnormalcalc[vid[2]].y += N.y;
		vertnormalcalc[vid[2]].z += N.z;
		noofconnectedfaces[vid[2]] += 1;
	}
	
	unsigned int nverts = iloadData->verts.size();
	for(unsigned int i=0; i<nverts; i++)
	{
		vertnormalcalc[i].x /= noofconnectedfaces[i];
		vertnormalcalc[i].y /= noofconnectedfaces[i];
		vertnormalcalc[i].z /= noofconnectedfaces[i];
		iloadData->vertnormal.push_back(Vector3<float>(vertnormalcalc[i].x, vertnormalcalc[i].y, vertnormalcalc[i].z));
	}
	return true;
}
bool off_io::calculateVertexColor(LoadData *iloadData)
{
	unsigned int ntris = iloadData->tris.size();
	for(unsigned int i=0; i<ntris; i++)
	{
		Vector3<unsigned int> vid = iloadData->tris[i];
		Vector3<float> c = iloadData->tricolor[i];

		// vertex color calculations		
		vertcolorcalc[vid[0]].x += c[0];
		vertcolorcalc[vid[0]].y += c[1];
		vertcolorcalc[vid[0]].z += c[2];
		noofconnectedfaces[vid[0]] += 1;
		vertcolorcalc[vid[1]].x += c[0];
		vertcolorcalc[vid[1]].y += c[1];
		vertcolorcalc[vid[1]].z += c[2];
		noofconnectedfaces[vid[1]] += 1;
		vertcolorcalc[vid[2]].x += c[0];
		vertcolorcalc[vid[2]].y += c[1];
		vertcolorcalc[vid[2]].z += c[2];
		noofconnectedfaces[vid[2]] += 1;
	}
	
	unsigned int nverts = iloadData->verts.size();
	for(unsigned int i=0; i<nverts; i++)
	{
		vertcolorcalc[i].x /= noofconnectedfaces[i];
		vertcolorcalc[i].y /= noofconnectedfaces[i];
		vertcolorcalc[i].z /= noofconnectedfaces[i];
		iloadData->vertcolor.push_back(Vector3<float>(vertcolorcalc[i].x, vertcolorcalc[i].y, vertcolorcalc[i].z));
	}
	return true;
}
