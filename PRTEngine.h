#include "d3dUtil.h"
#include "Mesh.h"
#include "Light.h"

class PRTEngine {
  public:
    PRTEngine(IDirect3DDevice9* device);
    virtual ~PRTEngine();

    void getCoefficientsForMesh(Mesh*, Light*);
    const D3DXSHMATERIAL** getMeshMaterial(Mesh *mesh);
    void ComputeShaderConstants( float* pSHCoeffsRed, float* pSHCoeffsGreen, float* pSHCoeffsBlue, Mesh* mesh );
    DWORD getOrder() { return mOrder; }
  protected:
    void ExtractCompressedDataForPRTShader(Mesh *mesh);

    IDirect3DDevice9* mDevice;
    ID3DXMesh *mMesh;
    ID3DXPRTEngine *mPRTEngine;
    DWORD mOrder;
};
