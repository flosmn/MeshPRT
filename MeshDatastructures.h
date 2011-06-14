#ifndef MESHDATASTRUCTURES_H
#define MESHDATASTRUCTURES_H

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

struct Color {
  float r;
  float g;
  float b;
  float a;

  bool operator() (Color c1, Color c2) {
    return (c1.r < c2.r ||
            c1.r == c2.r && c1.g < c2.g  ||
            c1.r == c2.r && c1.g == c2.g && c1.b < c2.b);
  }
};

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
