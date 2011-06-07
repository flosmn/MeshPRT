#include "d3dUtil.h"
#include "Mesh.h"
#include "CubeMap.h"

class PRTEngine {
public:
  PRTEngine(IDirect3DDevice9* device, DWORD order);
  virtual ~PRTEngine();

  HRESULT CalculateSHCoefficients(Mesh*);
  HRESULT ConvoluteSHCoefficients(Mesh* mesh, LightSource* lightSource);
  const D3DXSHMATERIAL** getMeshMaterial(Mesh *mesh);
  void ComputeShaderConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue, Mesh* mesh );
  DWORD getOrder() { return mOrder; }
  
protected:
  void ExtractCompressedDataForPRTShader(Mesh *mesh);

  IDirect3DDevice9* mDevice;
  ID3DXMesh *mMesh;
  ID3DXPRTEngine *mPRTEngine;
  DWORD mOrder;
  DWORD mNumBounces;
  DWORD mNumRays;
  DWORD mNumChannels;
  DWORD mNumPCA;
};
