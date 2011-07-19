#ifndef MESHDATASTRUCTURES_H
#define MESHDATASTRUCTURES_H

#include "d3dUtil.h"

struct FULL_VERTEX {
    D3DXVECTOR3 position;
    D3DXVECTOR3 normal;
    D3DXVECTOR2 texCoords;
    float clusterID;
    D3DXCOLOR blendWeight1;
    D3DXCOLOR blendWeight2;
    D3DXCOLOR blendWeight3;
    D3DXCOLOR blendWeight4;
    D3DXCOLOR blendWeight5;
    D3DXCOLOR blendWeight6;
};

struct Vertex {
  D3DXVECTOR3 pos;
  D3DXVECTOR3 normal;
  D3DXCOLOR color;

  bool operator() (Vertex v1, Vertex v2) {
    return (
      v1.pos.x < v2.pos.x ||
      v1.pos.x == v2.pos.x && v1.pos.y < v2.pos.y  ||
      v1.pos.x == v2.pos.x && v1.pos.y == v2.pos.y && v1.pos.z < v2.pos.z ||
      v1.pos.x == v2.pos.x && v1.pos.y == v2.pos.y && v1.pos.z == v2.pos.z && v1.normal.x < v2.normal.x ||
      v1.pos.x == v2.pos.x && v1.pos.y == v2.pos.y && v1.pos.z == v2.pos.z && v1.normal.x == v2.normal.x  && v1.normal.y < v2.normal.y ||
      v1.pos.x == v2.pos.x && v1.pos.y == v2.pos.y && v1.pos.z == v2.pos.z && v1.normal.x == v2.normal.x  && v1.normal.y == v2.normal.y && v1.normal.z < v2.normal.z);
  }
};

struct Face {
  DWORD vertices[3];
};

#endif // MESHDATASTRUCTURES_H
