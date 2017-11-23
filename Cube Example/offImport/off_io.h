
#ifndef __off_io_h__
#define __off_io_h__

#include "common.h"
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

class off_io
{
public:
  off_io() {}

  bool Load(std::istream & is, LoadData *iloadData); // false return on error
  //LoadData loadData;
  
protected:
  bool ReadHeader(std::istream &is);
  bool ReadDataCount(std::istream &is);
  bool ReadData(std::istream &is, LoadData *iloadData);
  bool calculateBoundingbox(LoadData *iloadData);
  bool calculateNormal(LoadData *iloadData);
  bool calculateVertexColor(LoadData *iloadData);
  
  unsigned int nVerts, nFaces;
  std::vector<unsigned int> noofconnectedfaces;
  std::vector<vec3> vertnormalcalc;
  std::vector<vec3> vertcolorcalc;
    
};

#endif
