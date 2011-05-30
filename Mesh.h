#include "MeshDatastructures.h"

class Mesh
{
public:
  Mesh(IDirect3DDevice9*);
  Mesh();
	virtual ~Mesh();

  ID3DXMesh* getMesh();
 
  HRESULT LoadMesh(WCHAR* directory, WCHAR* name, WCHAR* extension);
  
  void drawMesh();
  void loadFX(ID3DXEffect *effect);
     
  DWORD GetNumVertices();
  DWORD GetNumFaces();
      
  void setPRTCompBuffer(ID3DXPRTCompBuffer* compBuffer) { mPRTCompBuffer = compBuffer; }
  void setPRTConstants(float* prtConstants) { mPRTConstants = prtConstants; }
  HRESULT setPRTConstantsInEffect();

  D3DXCOLOR getDiffuseMaterial(int i);

  void SetDirectory(WCHAR* dir) { directory = dir; } 
  WCHAR* GetDirectory() { return directory; }

  void SetName(WCHAR* _name) { name = _name; } 
  WCHAR* GetName() { return name; }

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
  float mSpecularPower;
  
  D3DXMATRIX  mWorld;
	D3DXCOLOR   mDiffuseMtrl[3];
	D3DXCOLOR   mSpecularMtrl;
	
  D3DXHANDLE   mhDiffuseMtrl;
  D3DXHANDLE   mhSpecularMtrl;
  D3DXHANDLE   mhSpecularPower;
  D3DXHANDLE   mhWorld;
  D3DXHANDLE   mhWorldInverseTranspose;
    
  ID3DXPRTCompBuffer* mPRTCompBuffer;
  float* mPRTConstants;

  WCHAR* directory;
  WCHAR* name;
};