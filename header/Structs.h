#ifndef STRUCTS_H
#define STRUCTS_H

#include <d3dx9.h>

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

struct NNCandidate {
		Vertex vertex;
		float weight;
		float value;
		int index;
};

struct FULL_VERTEX {
  D3DXVECTOR3 position;
  D3DXVECTOR3 normal;
  D3DXVECTOR2 texCoords;
	
	D3DXCOLOR shCoeff1;
  D3DXCOLOR shCoeff2;
  D3DXCOLOR shCoeff3;
  D3DXCOLOR shCoeff4;
  D3DXCOLOR shCoeff5;
  D3DXCOLOR shCoeff6;
	D3DXCOLOR shCoeff7;
	D3DXCOLOR shCoeff8;
	D3DXCOLOR shCoeff9;

	D3DXVECTOR3 exactSHColor;

	D3DXVECTOR3 index;
};

struct Face {
  DWORD vertices[3];
};

#endif // STRUCTS_H