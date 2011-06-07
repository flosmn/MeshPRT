#ifndef DIRECTXMESH_H
#define DIRECTXMESH_H

#endif // DIRECTXMESH_H

#include "common/interfaces.h"

#include "windows.h"
#include "d3dUtil.h"
#include "hash_map"
#include "Mesh.h"

class DirectXMesh {
public:
  DirectXMesh();
  ~DirectXMesh();

  ID3DXMesh* GetMesh() { return mMesh; }
  HRESULT CreateDirectXMesh(const MeshModel &meshModel);
  HRESULT SaveMeshToFile(const WCHAR* filename);
  HRESULT CloneMesh(ID3DXMesh** target);
protected:
  ID3DXMesh* mMesh;
  IDirect3D9* pd3d9;
  IDirect3DDevice9* pd3d9Device;

  void CreateDevice();
  void ParseMesh(const MeshModel &m, std::vector<Vertex> &, std::vector<Face> &);
};

