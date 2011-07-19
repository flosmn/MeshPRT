#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Camera.h"
#include "PRTEngine.h"
#include "CubeMap.h"
#include "Light.h"
#include "PRTHierarchy.h"

class MeshPRT : public D3DApp
{
public:
  MeshPRT(std::string winCaption, D3DDEVTYPE devType,
          DWORD requestedVP);
  ~MeshPRT();

  bool IsInitialized();

  bool checkDeviceCaps();
  void onLostDevice();
  void onResetDevice();
  void updateScene(float dt);
  void drawScene();

  // Helper methods
  void buildFX();
  void buildViewMtx();
  void buildProjMtx();
  
private:
  HRESULT Init();
  HRESULT UpdateLighting();
  void SetTechnique();
    
  bool initialized;

  bool visualizeError;

  float reflectivity;

  D3DXMATRIX mWorld;
  D3DXMATRIX mProj;

  GfxStats* mGfxStats;
  ID3DXEffect* mFX;
  ID3DXEffect* mErrorVisFX;
  Camera *mCamera;
  CubeMap *mCubeMap;
  PRTHierarchy *mPRTHierarchy;
};

bool StartDirectX() {

  MeshPRT app( "MeshPRT", D3DDEVTYPE_HAL,
               D3DCREATE_HARDWARE_VERTEXPROCESSING);
  gd3dApp = &app;

  if(!app.IsInitialized()) return false;

  DirectInput di( DISCL_NONEXCLUSIVE|DISCL_FOREGROUND,
                  DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
  gDInput = &di;
  
  return gd3dApp->run();
}

MeshPRT::MeshPRT( std::string winCaption, D3DDEVTYPE devType,
                  DWORD requestedVP )
  : D3DApp(winCaption, devType, requestedVP)
{
  mGfxStats = 0;
  mFX = 0;
  mErrorVisFX = 0;
  mCamera = 0;
  mCubeMap = 0;
  mPRTHierarchy = 0;

  if(!checkDeviceCaps())
  {
    MessageBox(0, L"checkDeviceCaps() Failed", 0, 0);
    PostQuitMessage(0);
  }

  visualizeError = false;
  initialized = false;

  HRESULT hr = Init();

  PD(hr, L"initialize");
  if(FAILED(hr)){
    initialized = false;
  }
  else{
    initialized = true;
  }
}

MeshPRT::~MeshPRT()
{
  delete mGfxStats;
  delete mCamera;
  delete mCubeMap;
  delete mPRTHierarchy;
  SAFE_RELEASE(mFX)
  SAFE_RELEASE(mErrorVisFX)
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

bool MeshPRT::IsInitialized() {
  return initialized;
}

HRESULT MeshPRT::Init() {
  HRESULT hr;
    
  reflectivity = 0.3f;
  DWORD order = 6;
    
  buildFX();

  mCubeMap = new CubeMap(gd3dDevice);
  hr = mCubeMap->LoadCubeMap(L"cubemaps/", L"stpeters_cross", L".dds");
  PD(hr, L"load cube map");
  if(FAILED(hr)) return hr;

  mPRTHierarchy = new PRTHierarchy(gd3dDevice);
  mPRTHierarchy->LoadMeshHierarchy(L"bigship1",
                                   L"bigship1",
                                   L"models/",
                                   L".x",
                                   order);

  mPRTHierarchy->ScaleMeshes();    
  mPRTHierarchy->CalculateSHCoefficients(mCubeMap);
  mPRTHierarchy->CalculateDiffuseColor();
  if(visualizeError){
    mPRTHierarchy->LoadEffect(mErrorVisFX);
  }
  else{
    mPRTHierarchy->LoadEffect(mFX);
  }
  
  mGfxStats = new GfxStats();

  D3DXMatrixIdentity(&mWorld);

  onResetDevice();

  mCamera = new Camera();

  return D3D_OK;
}

void MeshPRT::updateScene(float dt)
{
  mGfxStats->setVertexCount(mPRTHierarchy->GetNumVertices());
  mGfxStats->setTriCount(mPRTHierarchy->GetNumFaces());
  mGfxStats->update(dt);

  mCamera->update(dt);

  gDInput->poll();
  
  if( gDInput->keyDown(DIK_Z) ) {
    reflectivity = reflectivity - 0.0001f;

    if(reflectivity < 0){
      reflectivity = 0;
    }
  }

  if( gDInput->keyDown(DIK_X) ) {
    reflectivity = reflectivity + 0.0001f;

    if(reflectivity > 1){
      reflectivity = 1;
    }
  }
}

HRESULT MeshPRT::UpdateLighting(){
  HRESULT hr;
    
  return D3D_OK;
}

void MeshPRT::drawScene()
{
  D3DXCOLOR color = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.0f);
  gd3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0);
  gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, color, 1.0f, 0);

  gd3dDevice->BeginScene();

  ID3DXEffect* effect;
  if(visualizeError) {
    effect = mErrorVisFX;
  } else {
    effect = mFX;
  }
  D3DXHANDLE handle;
    
  D3DXMATRIX mWVP = mCamera->view() * mProj;
  mCubeMap->DrawCubeMap(&mWVP);
  effect->SetTexture( "EnvMap", mCubeMap->GetTexture() );
    
  effect->SetBool("useTextures", mPRTHierarchy->HasTextures());
  
  if(visualizeError){
    handle = effect->GetTechniqueByName("ErrorVisualization");
    effect->SetFloat( "gReflectivity", 0.0f );
  }
  else{
    handle = effect->GetTechniqueByName("PRTLighting");
    effect->SetFloat( "gReflectivity", reflectivity );
  }
  effect->SetTechnique(handle);
    
  handle = effect->GetParameterByName(0, "gEyePosW");
  effect->SetValue(handle, &(mCamera->pos()), sizeof(D3DXVECTOR3));
  handle = effect->GetParameterByName(0, "gView");
  effect->SetMatrix(handle, &(mCamera->view()));  
  handle = effect->GetParameterByName(0, "gProjection");
  effect->SetMatrix(handle, &mProj);

  // Begin passes.
  UINT numPasses = 0;
  effect->Begin(&numPasses, 0);
  for(UINT i = 0; i < numPasses; ++i)
  {
    effect->BeginPass(i);
    mPRTHierarchy->DrawMesh();
    effect->EndPass();
  }
  effect->End();
  
  mGfxStats->display();

  gd3dDevice->EndScene();
    
  gd3dDevice->Present(0, 0, 0, 0);
}

void MeshPRT::buildFX()
{
  WCHAR* effectName = L"shader/diffuse.fx";
  LoadEffectFile(gd3dDevice, effectName, 0, D3DXSHADER_DEBUG, &mFX);

  effectName = L"shader/errorVisualization.fx";
  LoadEffectFile(gd3dDevice, effectName, 0, D3DXSHADER_DEBUG, &mErrorVisFX);
  
  PD(D3D_OK, L"done building FX");
}

void MeshPRT::buildProjMtx()
{
  float w = (float)md3dPP.BackBufferWidth;
  float h = (float)md3dPP.BackBufferHeight;
  D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
