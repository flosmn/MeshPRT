#include <list>

#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Camera.h"
#include "PRTEngine.h"


class MeshPRT : public D3DApp
{
public:
  MeshPRT(std::string winCaption, D3DDEVTYPE devType,
          DWORD requestedVP);
  ~MeshPRT();

  bool checkDeviceCaps();
  void onLostDevice();
  void onResetDevice();
  void updateScene(float dt);
  void drawScene();

  // Helper methods
  void buildFX(Mesh* mesh);
  void buildViewMtx();
  void buildProjMtx();
  
private:
  HRESULT UpdateLighting();
  void SetTechnique();

  bool phongShading;
  bool environmentLighting;

  float reflectivity;

  GfxStats* mGfxStats;

  ID3DXEffect* mFX;
  D3DXHANDLE   mhPerVertexLightingTechnique;
  D3DXHANDLE   mhPerPixelLightingTechniqueWithTexture;
  D3DXHANDLE   mhPerPixelLightingTechniqueWithoutTexture;
  D3DXHANDLE   mhPRTLightingTechniqueWithTexture;
  D3DXHANDLE   mhPRTLightingTechniqueWithoutTexture;
  D3DXHANDLE   mhView;
  D3DXHANDLE   mhProjection;
  D3DXHANDLE   mhEyePosW;
  D3DXHANDLE   mhLightPositionW;
  D3DXHANDLE   mhLightVecW;
  D3DXHANDLE   mhLightColor;
  D3DXHANDLE   mhReflectivity;

  D3DXMATRIX mWorld;
  D3DXMATRIX mProj;

  Mesh *mMesh;
  Camera *mCamera;
  PRTEngine* mPRTEngine;
  CubeMap* mCubeMap;
  Light* mLight;
};

bool StartDirectX() {
  // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  MeshPRT app( "MeshPRT", D3DDEVTYPE_HAL,  
               D3DCREATE_HARDWARE_VERTEXPROCESSING);
  gd3dApp = &app;

  DirectInput di( DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, 
                  DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
  gDInput = &di;

  return gd3dApp->run();
}

MeshPRT::MeshPRT( std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP )
  : D3DApp(winCaption, devType, requestedVP)
{
  if(!checkDeviceCaps())
  {
    MessageBox(0, L"checkDeviceCaps() Failed", 0, 0);
    PostQuitMessage(0);
  }

  phongShading = false;
  environmentLighting = true;
  reflectivity = 0.5f;

  HRESULT hr;

  DWORD order = 6;

  mMesh = new Mesh(gd3dDevice);
  hr = mMesh->LoadMesh(L"models/", L"bigship1", L".x");
  PD(hr, L"load mesh");

  mCubeMap = new CubeMap(gd3dDevice);
  hr = mCubeMap->LoadCubeMap(L"cubemaps/", L"stpeters_cross", L".dds");
  PD(hr, L"load cube map");
  hr = mCubeMap->CalculateSHCoefficients(order);
  PD(hr, L"calculate SHCoefficients of cube map light");
     
  mLight = new Light( D3DXVECTOR3(0.0f, 5.0f, -5.0f), 
                      D3DXVECTOR3(-1.0f, -1.0f, 1.0f), 
                      D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );
  
  hr = mLight->CalculateSHCoefficients(order);
  PD(hr, L"calculate SHCoefficients of directional light");

  mPRTEngine = new PRTEngine(gd3dDevice, order);
  mPRTEngine->CalculateSHCoefficients(mMesh);
   
  buildFX(mMesh);
  mMesh->LoadFX(mFX);

  UpdateLighting();

  mGfxStats = new GfxStats();

  D3DXMatrixIdentity(&mWorld);
  
  onResetDevice();

  mCamera = new Camera();  
}

MeshPRT::~MeshPRT()
{
  delete mGfxStats;
  delete mMesh;
  delete mPRTEngine;
  delete mLight;
  delete mCamera;
}

bool MeshPRT::checkDeviceCaps()
{
  D3DCAPS9 caps;
  gd3dDevice->GetDeviceCaps(&caps);

  // Check for vertex shader version 2.0 support.
  if( caps.VertexShaderVersion < D3DVS_VERSION(2, 0) )
    return false;

  // Check for pixel shader version 2.0 support.
  if( caps.PixelShaderVersion < D3DPS_VERSION(2, 0) )
    return false;

  return true;
}

void MeshPRT::onLostDevice()
{
  mGfxStats->onLostDevice();
  mFX->OnLostDevice();
}

void MeshPRT::onResetDevice()
{
  mGfxStats->onResetDevice();
  mFX->OnResetDevice();

  // The aspect ratio depends on the backbuffer dimensions, which can
  // possibly change after a reset.  So rebuild the projection matrix.
  buildProjMtx();
}

void MeshPRT::updateScene(float dt)
{
  mGfxStats->setVertexCount(mMesh->GetNumVertices());
  mGfxStats->setTriCount(mMesh->GetNumFaces());
  mGfxStats->update(dt);
  mCamera->update(dt);

  // Get snapshot of input devices.
  gDInput->poll();

  if( gDInput->keyDown(DIK_P) )	 
    phongShading = true;

  if( gDInput->keyDown(DIK_O) )	 
    phongShading = false;

  if( gDInput->keyDown(DIK_N) && !environmentLighting )	 
  {
    environmentLighting = true;
    UpdateLighting();
  }

  if( gDInput->keyDown(DIK_M) && environmentLighting )
  {
    environmentLighting = false;
    UpdateLighting();
  }

  if( gDInput->keyDown(DIK_Z) )
    reflectivity = reflectivity - 0.0001f;

  if( gDInput->keyDown(DIK_X) )
    reflectivity = reflectivity + 0.0001f;
}

HRESULT MeshPRT::UpdateLighting(){
  HRESULT hr;
    
  if(environmentLighting) {
    hr = mPRTEngine->ConvoluteSHCoefficients(mMesh, mCubeMap);
  }
  else{
    hr = mPRTEngine->ConvoluteSHCoefficients(mMesh, mLight);
  }

  PD(hr, L"convolute sh coefficients");
  if(FAILED(hr)) return hr;

  mMesh->SetPRTConstantsInEffect();

  return D3D_OK;
}


void MeshPRT::drawScene()
{
  D3DXCOLOR color = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.0f);
  gd3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0);
  gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, color, 1.0f, 0);

  gd3dDevice->BeginScene();
     
  if(environmentLighting) {
    D3DXMATRIX mWVP = mCamera->view() * mProj;
    mCubeMap->DrawCubeMap(&mWVP);
  }
  
  SetTechnique();

  mFX->SetTexture( "EnvMap", mCubeMap->GetTexture() );
  mFX->SetFloat( "gReflectivity", reflectivity );
  mFX->SetValue(mhEyePosW, &(mCamera->pos()), sizeof(D3DXVECTOR3));
  mFX->SetMatrix(mhView, &(mCamera->view()));  
  mFX->SetMatrix(mhProjection, &mProj);

  mFX->SetValue(mhLightVecW, &mLight->GetLightDirection(), sizeof(D3DXVECTOR3));
  mFX->SetValue(mhLightPositionW, &mLight->GetLightPosition(), sizeof(D3DXCOLOR));
  mFX->SetValue(mhLightColor, &mLight->GetLightColor(), sizeof(D3DXCOLOR));
  
  // Begin passes.
  UINT numPasses = 0;
  mFX->Begin(&numPasses, 0);
  for(UINT i = 0; i < numPasses; ++i)
  {
    mFX->BeginPass(i);
    mMesh->DrawMesh();
    mFX->EndPass();
  }
  mFX->End();


  mGfxStats->display();

  gd3dDevice->EndScene();

  // Present the backbuffer.
  gd3dDevice->Present(0, 0, 0, 0);
}

void MeshPRT::SetTechnique() {
  if( mMesh->HasTextures() )
  {
    if(phongShading) {
      mFX->SetTechnique(mhPerPixelLightingTechniqueWithTexture);
    }
    else {
      mFX->SetTechnique(mhPRTLightingTechniqueWithTexture);
    }    
  }
  else
  {
    if(phongShading) {
      mFX->SetTechnique(mhPerPixelLightingTechniqueWithoutTexture);
    }
    else {
      mFX->SetTechnique(mhPRTLightingTechniqueWithoutTexture);
    }    
  }
}

void MeshPRT::buildFX(Mesh* mesh)
{
  UINT dwNumChannels = mesh->GetPRTCompBuffer()->GetNumChannels();
  UINT dwNumClusters = mesh->GetPRTCompBuffer()->GetNumClusters();
  UINT dwNumPCA = mesh->GetPRTCompBuffer()->GetNumPCA();

  // The number of vertex consts need by the shader can't exceed the 
  // amount the HW can support
  DWORD dwNumVConsts = dwNumClusters * ( 1 + dwNumChannels * dwNumPCA / 4 ) + 4;
  
  D3DXMACRO aDefines[3];
  CHAR szMaxNumClusters[64];

  sprintf( szMaxNumClusters, "%d", dwNumClusters );
  szMaxNumClusters[63] = 0;
  CHAR szMaxNumPCA[64];
  sprintf( szMaxNumPCA, "%d", dwNumPCA );
  szMaxNumPCA[63] = 0;
  aDefines[0].Name = "NUM_CLUSTERS";
  aDefines[0].Definition = szMaxNumClusters;
  aDefines[1].Name = "NUM_PCA";
  aDefines[1].Definition = szMaxNumPCA;
  aDefines[2].Name = NULL;
  aDefines[2].Definition = NULL;
  
  WCHAR* effectName = L"shader/diffuse.fx";
  PD( LoadEffectFile(gd3dDevice, effectName, aDefines, D3DXSHADER_DEBUG, &mFX),
      Concat(L"load effect file ", effectName) );
    
  // Obtain handles.
  mhPerVertexLightingTechnique = mFX->GetTechniqueByName("PerVertexLighting");
  mhPerPixelLightingTechniqueWithTexture = mFX->GetTechniqueByName("PerPixelLightingWithTexture");
  mhPerPixelLightingTechniqueWithoutTexture = mFX->GetTechniqueByName("PerPixelLightingWithoutTexture");
  mhPRTLightingTechniqueWithTexture  = mFX->GetTechniqueByName("PRTLightingWithTexture");
  mhPRTLightingTechniqueWithoutTexture  = mFX->GetTechniqueByName("PRTLightingWithoutTexture");
  mhView                  = mFX->GetParameterByName(0, "gView");
  mhProjection            = mFX->GetParameterByName(0, "gProjection");
  mhLightVecW             = mFX->GetParameterByName(0, "gLightVecW");
  mhEyePosW               = mFX->GetParameterByName(0, "gEyePosW");
  mhLightColor            = mFX->GetParameterByName(0, "gLightColor");
  mhLightPositionW        = mFX->GetParameterByName(0, "gLightPosW");
  mhReflectivity          = mFX->GetParameterByName(0, "gReflectivity");

  PD(D3D_OK, L"done building FX");
}

void MeshPRT::buildProjMtx()
{
  float w = (float)md3dPP.BackBufferWidth;
  float h = (float)md3dPP.BackBufferHeight;
  D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
