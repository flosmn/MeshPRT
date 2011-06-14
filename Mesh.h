#ifndef MESH_H
#define MESH_H

#include "d3dUtil.h"
#include "MeshDatastructures.h"

class Mesh
{
public:
  Mesh(IDirect3DDevice9*);
  Mesh();
	virtual ~Mesh();

  ID3DXMesh* GetMesh();
 
  HRESULT LoadMesh(WCHAR* directory, WCHAR* name, WCHAR* extension);
  HRESULT LoadMesh(ID3DXMesh* mesh);
  
  void DrawMesh();
  void LoadFX(ID3DXEffect *effect);
     
  DWORD GetNumVertices();
  DWORD GetNumFaces();
      
  void SetPRTCompBuffer(ID3DXPRTCompBuffer* compBuffer) { mPRTCompBuffer = compBuffer; }
  ID3DXPRTCompBuffer* GetPRTCompBuffer() { return mPRTCompBuffer; }
  
  void SetPRTConstants(float* prtConstants);
  float* GetPRTConstants() { return mPRTConstants; }
  
  D3DXCOLOR GetDiffuseMaterial(int i);

  void SetDirectory(WCHAR* dir) { directory = dir; } 
  WCHAR* GetDirectory() { return directory; }

  void SetName(WCHAR* _name) { name = _name; } 
  WCHAR* GetName() { return name; }

  IDirect3DTexture9* GetTextures() { return mTextures[0]; }
  bool HasTextures() { return hasTextures; }

protected:
  HRESULT AdjustMeshDecl();
  HRESULT AttribSortMesh();
  HRESULT LoadTextures();

  bool DoesMeshHaveUsage( BYTE Usage );
  void CleanUpMesh();

  IDirect3DDevice9 *mDevice;
  ID3DXMesh* mMesh;  
  ID3DXEffect* mEffect;

  DWORD mNumMaterials;
  ID3DXBuffer* mMaterialBuffer;
  D3DXMATERIAL* mMaterials;

  std::vector<IDirect3DTexture9*> mTextures;
  bool hasTextures;

  float mRotationX;
  float mRotationY;
  float mRotationZ;
    
  D3DXMATRIX  mWorld;
	D3DXCOLOR   mDiffuseMtrl[3];
		
  D3DXHANDLE   mhWorld;
  D3DXHANDLE   mhWorldInverseTranspose;
    
  ID3DXPRTCompBuffer* mPRTCompBuffer;
  float* mPRTConstants;

  WCHAR* directory;
  WCHAR* name;
};

#endif // MESH_H
