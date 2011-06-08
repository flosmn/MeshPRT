#ifndef MESHDATASTRUCTURES_H
#define MESHDATASTRUCTURES_H

#include "d3dUtil.h"

struct Vertex {
  float x;
  float y;
  float z;

  bool operator() (Vertex v1, Vertex v2) {
    return (v1.x < v2.x ||
            v1.x == v2.x && v1.y < v2.y  ||
            v1.x == v2.x && v1.y == v2.y && v1.z < v2.z);
  }
};

struct Face {
  DWORD vertices[3];
};

#endif // MESHDATASTRUCTURES_H
