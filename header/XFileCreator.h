#ifndef XFILECREATOR_H
#define XFILECREATOR_H

#include "d3dUtil.h"
#include "MeshDatastructures.h"

class XFileCreator {
public:
  XFileCreator();
  ~XFileCreator();

  HRESULT CreateXFile(WCHAR* targetFile, 
                      std::vector<Vertex> vertices,
                      std::vector<Face> faces);

private:
  ID3DXMesh* mMesh;
  IDirect3D9* mDirect3D9;
  IDirect3DDevice9* mDevice;
  
  HRESULT CreateDevice();
  HRESULT CreateMesh(std::vector<Vertex> vertices, std::vector<Face> faces);
  HRESULT SaveMeshToFile(WCHAR* targetFile);

};

#endif // XFILECREATOR_H