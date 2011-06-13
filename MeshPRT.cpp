#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Camera.h"
#include "PRTEngine.h"
#include "CubeMap.h"
#include "Light.h"

class MeshPRT : public D3DApp
{
public:
  MeshPRT(std::string winCaption, ID3DXMesh* mesh, D3DDEVTYPE devType,
          DWORD requestedVP);
  ~MeshPRT();

  bool IsInitialized();

  bool checkDeviceCaps();
  void onLostDevice();
  void onResetDevice();
  void updateScene(float dt);
  void drawScene();

  // Helper methods
  void buildFX(Mesh* mesh);
  void buildViewMtx();
  void buildProjMtx();

  void LoadEffectFile_InShader(Mesh* mesh);
  void LoadEffectFile_AllCpu(Mesh* mesh);
  
private:
  HRESULT Init(ID3DXMesh* mesh);
  HRESULT UpdateLighting();
  void SetTechnique();

  bool phongShading;
  bool environmentLighting;
  bool allCpu;

  bool initialized;

  float reflectivity;

  GfxStats* mGfxStats;

  ID3DXEffect* mFX;
  D3DXHANDLE   mhPerPixelLighting; 
  D3DXHANDLE   mhPRTLighting;  
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

bool StartDirectX(ID3DXMesh* mesh) {
  // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  MeshPRT app( "MeshPRT", mesh, D3DDEVTYPE_HAL,
               D3DCREATE_HARDWARE_VERTEXPROCESSING);
  gd3dApp = &app;

  if(!app.IsInitialized()) return false;

  DirectInput di( DISCL_NONEXCLUSIVE|DISCL_FOREGROUND,
                  DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
  gDInput = &di;

  return gd3dApp->run();
}

MeshPRT::MeshPRT( std::string winCaption, ID3DXMesh* mesh, D3DDEVTYPE devType, DWORD requestedVP )
  : D3DApp(winCaption, devType, requestedVP)
{
  if(!checkDeviceCaps())
  {
    MessageBox(0, L"checkDeviceCaps() Failed", 0, 0);
    PostQuitMessage(0);
  }
  initialized = false;

  HRESULT hr = Init(mesh);
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
  delete mMesh;
  delete mPRTEngine;
  delete mLight;
  delete mCamera;
  delete mCubeMap;
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

HRESULT MeshPRT::Init(ID3DXMesh* mesh) {
  HRESULT hr;

  phongShading = false;
  environmentLighting = true;
  allCpu = true;

  reflectivity = 0.0f;
  DWORD order = 6;
  
  mMesh = new Mesh(gd3dDevice);

  if(mesh == NULL){
    hr = mMesh->LoadMesh(L"models/", L"bigship1", L".x");
  }
  else{
    hr = mMesh->LoadMesh(mesh);
    /* mesh is now released*/
  }
  PD(hr, L"load mesh");
  if(FAILED(hr)) return hr;
  
  mCubeMap = new CubeMap(gd3dDevice);
  hr = mCubeMap->LoadCubeMap(L"cubemaps/", L"stpeters_cross", L".dds");
  PD(hr, L"load cube map");
  if(FAILED(hr)) return hr;

  hr = mCubeMap->CalculateSHCoefficients(order);
  PD(hr, L"calculate SHCoefficients of cube map light");
  if(FAILED(hr)) return hr;
  
  mLight = new Light( D3DXVECTOR3(0.0f, 5.0f, -5.0f),
                      D3DXVECTOR3(-1.0f, -1.0f, 1.0f),
                      D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f) );

  hr = mLight->CalculateSHCoefficients(order);
  PD(hr, L"calculate SHCoefficients of directional light");
  if(FAILED(hr)) return hr;

  mPRTEngine = new PRTEngine(gd3dDevice, order);

  hr = mPRTEngine->CalculateSHCoefficients(mMesh);
  PD(hr, L"calculate sh coefficients for mesh");
  if(FAILED(hr)) return hr;

  buildFX(mMesh);
  mMesh->LoadFX(mFX);

  UpdateLighting();

  mGfxStats = new GfxStats();

  D3DXMatrixIdentity(&mWorld);

  onResetDevice();

  mCamera = new Camera();

  return D3D_OK;
}

void MeshPRT::updateScene(float dt)
{
  mGfxStats->setVertexCount(mMesh->GetNumVertices());
  mGfxStats->setTriCount(mMesh->GetNumFaces());
  mGfxStats->update(dt);

  mCamera->update(dt);

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

  if( gDInput->keyDown(DIK_M) && environmentLighting ) {
    environmentLighting = false;
    UpdateLighting();
  }

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

  if(environmentLighting) {
    hr = mPRTEngine->ConvoluteSHCoefficients(mMesh, mCubeMap);
  }
  else{
    hr = mPRTEngine->ConvoluteSHCoefficients(mMesh, mLight);
  }

  PD(hr, L"convolute sh coefficients");
  if(FAILED(hr)) return hr;

  if(allCpu) {
    hr = mPRTEngine->CalculateDiffuseColor(mMesh);
    PD(hr, L"calculate diffuse color of mesh");
    if(FAILED(hr)) return hr;
  }
  else {
    hr = mMesh->SetPRTConstantsInEffect();
    PD(hr, L"set prt constants in effect");
    if(FAILED(hr)) return hr;
  }

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
    mFX->SetTexture( "EnvMap", mCubeMap->GetTexture() );
    mFX->SetFloat( "gReflectivity", reflectivity );
  }
  else {
    mFX->SetFloat( "gReflectivity", 0.0f );
  }
  
  mFX->SetBool("environmentLighting", environmentLighting);
  mFX->SetBool("useTextures", mMesh->HasTextures());
  
  SetTechnique();
    
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
    
  gd3dDevice->Present(0, 0, 0, 0);
}

void MeshPRT::SetTechnique() {
  HRESULT hr;

  if(phongShading) {
    hr = mFX->SetTechnique(mhPerPixelLighting);
    // PD(hr, L"set technique to per pixel lighting");
  }
  else {
    hr = mFX->SetTechnique(mhPRTLighting);
    // PD(hr, L"set technique to PRT lighting");
  }
}

void MeshPRT::buildFX(Mesh* mesh)
{
  if(allCpu) {
    LoadEffectFile_AllCpu(mesh);
  }
  else {
    LoadEffectFile_InShader(mesh);
  }

  mhPerPixelLighting      = mFX->GetTechniqueByName("PerPixelLighting");  
  mhPRTLighting           = mFX->GetTechniqueByName("PRTLighting");
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

void MeshPRT::LoadEffectFile_InShader(Mesh* mesh){
  UINT dwNumChannels = mesh->GetPRTCompBuffer()->GetNumChannels();
  UINT dwNumClusters = mesh->GetPRTCompBuffer()->GetNumClusters();
  UINT dwNumPCA = mesh->GetPRTCompBuffer()->GetNumPCA();

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
}

void MeshPRT::LoadEffectFile_AllCpu(Mesh* mesh){ 
  WCHAR* effectName = L"shader/diffuse_cpu.fx";
  PD( LoadEffectFile(gd3dDevice, effectName, 0, D3DXSHADER_DEBUG, &mFX),
      Concat(L"load effect file ", effectName) );
}

