#ifndef PRTENGINE_H
#define PRTENGINE_H

#include "d3dUtil.h"
#include "Mesh.h"
#include "LightSource.h"

class PRTEngine {
public:
  PRTEngine(IDirect3DDevice9* device, DWORD order);
  virtual ~PRTEngine();

  HRESULT CalculateSHCoefficients(Mesh*);
  HRESULT ConvoluteSHCoefficients(Mesh* mesh, LightSource* lightSource);
  HRESULT CalculateDiffuseColor(Mesh* mesh);
  
  const D3DXSHMATERIAL** getMeshMaterial(Mesh *mesh);
  
  void ComputeShaderConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, 
                               float* pSHCoeffsBlue, Mesh* mesh );
  
  
  D3DXCOLOR GetPrecomputedDiffuseColor( int iClusterOffset, float *vPCAWeights, 
                                        DWORD numPCA, float *PRTConstants);
  
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

#endif // PRTENGINE_H
