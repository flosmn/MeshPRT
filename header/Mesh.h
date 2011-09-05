#ifndef MESH_H
#define MESH_H

#include "d3dUtil.h"
#include "Structs.h"

class Mesh
{
public:
  Mesh(IDirect3DDevice9*);
  Mesh();
	virtual ~Mesh();

  ID3DXMesh* GetMesh();
 
  HRESULT LoadMesh(WCHAR* directory, WCHAR* name, WCHAR* extension);
  HRESULT LoadMesh(ID3DXMesh* mesh);
  
	HRESULT CreateTopologyFromMesh();

  void DrawMesh();
  void LoadFX(ID3DXEffect *effect);
     
  DWORD GetNumVertices();
  DWORD GetNumFaces();
      
  void InitialiseMappingDatastructures();
  void InitialiseSHDataStructures();

  HRESULT FillVertexBufferWithSHCoefficients();

  int* GetMappingIndices() { return mMappingIndices; }
  float* GetMappingWeights() { return mMappingWeights; }

  void SetPRTCompBuffer(ID3DXPRTCompBuffer* compBuffer) { 
    mPRTCompBuffer = compBuffer;
  }
  
  ID3DXPRTCompBuffer* GetPRTCompBuffer() { return mPRTCompBuffer; }
  
  void SetPRTClusterBases(float* prtClusterBases);
  float* GetPRTClusterBases() { return mPRTClusterBases; }

  void SetPcaWeights(float* pcaWeights);
  float* GetPcaWeights() { return mPCAWeights; }

  void SetClusterIds(int* clusterIds);
  int* GetClusterIds() { return mClusterIds; }

  void SetSHCoefficients(float* shCoefficients);
  float* GetSHCoefficients() { return mSHCoefficients; }
		
  float* GetInterpolatedSHCoefficients() { return mInterpolatedSHCoefficients; }
  
  D3DXCOLOR GetDiffuseMaterial(int i);

	Vertex* GetVertices() { return mVertices; }
	DWORD* GetFaces() { return mFaces; }

	std::vector<DWORD>* GetVertexFaceAdjazency() { return mVertexFaceAdjazency; }

  void SetDirectory(WCHAR* dir) { directory = dir; } 
  WCHAR* GetDirectory() { return directory; }

  void SetName(WCHAR* _name) { name = _name; } 
  WCHAR* GetName() { return name; }

  IDirect3DTexture9* GetTextures() { return mTextures[0]; }
  bool HasTextures() { return hasTextures; }

	D3DXMATRIX GetWorldTransformation() { return mWorld; }
	D3DXMATRIX GetRotationMatrix() { return mRotation; };
	D3DXMATRIX GetRotationInverseMatrix() { return mRotationInverse; }

	void SetWorldTransformation(D3DXMATRIX matrix) {
    mWorld = matrix;
  }

	void SetRotationMatrix(D3DXMATRIX matrix) {
    mRotation = matrix;
		D3DXMatrixInverse(&mRotationInverse, 0, &matrix);
  }

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
    
  D3DXMATRIX  mWorld;
	D3DXMATRIX  mRotation;
	D3DXMATRIX  mRotationInverse;
  D3DXCOLOR   mDiffuseMtrl[3];
		
  D3DXHANDLE   mhWorld;
  D3DXHANDLE   mhWorldInverseTranspose;
    
  ID3DXPRTCompBuffer* mPRTCompBuffer;
  float* mPRTClusterBases;
  float* mPCAWeights;
  int* mClusterIds;

  float* mSHCoefficients;
	float* mInterpolatedSHCoefficients;

  int* mMappingIndices;
  float* mMappingWeights;

  WCHAR* directory;
  WCHAR* name;

	DWORD* mFaces;
	Vertex* mVertices;
	std::vector<DWORD>* mVertexFaceAdjazency;
};

#endif // MESH_H
