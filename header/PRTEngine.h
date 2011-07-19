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
  HRESULT CalculateDiffuseColor(Mesh* mesh, LightSource* light);
  
  void InitMeshMaterial(Mesh *mesh, DWORD numMeshes,
                        D3DXSHMATERIAL* material, 
                        D3DXSHMATERIAL** materials);
  
  void ComputeShaderConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, 
                               float* pSHCoeffsBlue, Mesh* mesh );
  
  
  // this will be on the GPU
  D3DXCOLOR GetPrecomputedDiffuseColor( int clusterID, 
                                                 float *vPCAWeights, 
                                                 DWORD numPCA,
                                                 int numCoeffs,
                                                 int numChannels,
                                                 float* prtClusterBases,
                                                 float* redLightCoeff,
                                                 float* greenLightCoeff,
                                                 float* blueLightCoeff);
  
  DWORD getOrder() { return mOrder; }

protected:
  IDirect3DDevice9* mDevice;
    
  DWORD mOrder;
  DWORD mNumBounces;
  DWORD mNumRays;
  DWORD mNumChannels;
  DWORD mNumPCA;
};

#endif // PRTENGINE_H
