#include "MeshDatastructures.h"

class Mesh
{
public:
  Mesh(IDirect3DDevice9*);
  Mesh();
	virtual ~Mesh();

  ID3DXMesh* getMesh();
 
  HRESULT LoadMesh();
  HRESULT CreateMeshFrom(std::vector<Vertex> vertices, std::vector<Face> faces);
  
  void drawMesh();
  void loadFX(ID3DXEffect *effect);
     
  DWORD GetNumVertices();
  DWORD GetNumFaces();
      
  void setPRTCompBuffer(ID3DXPRTCompBuffer* compBuffer) { mPRTCompBuffer = compBuffer; }
  void setPRTConstants(float* prtConstants) { mPRTConstants = prtConstants; }
  void setPRTConstantsInEffect();

  D3DXCOLOR getDiffuseMaterial(int i);

protected:
  HRESULT AdjustMeshDecl( IDirect3DDevice9* pd3dDevice, ID3DXMesh** pMesh );

  IDirect3DDevice9 *mDevice;
  ID3DXMesh* mMesh;  
  ID3DXEffect* mEffect;

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
};