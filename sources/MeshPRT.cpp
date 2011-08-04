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
  void buildEffectFiles();
  void buildViewMtx();
  void buildProjMtx();
  
private:
  HRESULT Init();
  HRESULT UpdateLighting();
  void SetTechnique();
    
  bool initialized;

	bool mVisualizeError;
	bool mInterpolate;

	DWORD mNumNN;

  float reflectivity;

  D3DXMATRIX mWorld;
  D3DXMATRIX mProj;

  GfxStats* mGfxStats;
  ID3DXEffect* mFX;
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
  mCamera = 0;
  mCubeMap = 0;
  mPRTHierarchy = 0;

  if(!checkDeviceCaps())
  {
    MessageBox(0, L"checkDeviceCaps() Failed", 0, 0);
    PostQuitMessage(0);
  }

  mVisualizeError = false;
	mInterpolate = true;
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
    
  reflectivity = 0.0f;

  // order is max 4
  DWORD order = 3;
  mNumNN = 3;
    
  mCubeMap = new CubeMap(gd3dDevice);
  hr = mCubeMap->LoadCubeMap(L"cubemaps/", L"stpeters_cross", L".dds");
  PD(hr, L"load cube map");
  if(FAILED(hr)) return hr;

  mPRTHierarchy = new PRTHierarchy(gd3dDevice);
  mPRTHierarchy->LoadMeshHierarchy(L"bimba_d",
                                   L"bimba_e",
                                   L"models/",
                                   L".x",
                                   order);

  mPRTHierarchy->ScaleMeshes();    
  hr = mPRTHierarchy->CalculateSHCoefficients();
  PD(hr, L"ScaleMeshes");
	if(FAILED(hr)) return hr;

  // has to happen after SH calculations
  buildEffectFiles();
  
	mPRTHierarchy->CalculateNNMapping(mNumNN);
	PD(hr, L"CalculateNNMapping");
	if(FAILED(hr)) return hr;
  
	mPRTHierarchy->InterpolateSHCoefficients(mNumNN);
	PD(hr, L"InterpolateSHCoefficients");
	if(FAILED(hr)) return hr;

	mPRTHierarchy->TransferSHDataToGPU(mNumNN, mInterpolate);
  PD(hr, L"TransferSHDataToGPU");
	if(FAILED(hr)) return hr;

	mPRTHierarchy->UpdateLighting(mCubeMap);

  mPRTHierarchy->CheckColor(mCubeMap);
  
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
	
	if( gDInput->keyDown(DIK_E) ) {
    mVisualizeError = !mVisualizeError;
		mPRTHierarchy->UpdateState(mVisualizeError, mInterpolate, mNumNN);
  }

	if( gDInput->keyDown(DIK_I) ) {
		if(!mInterpolate){ 
			mInterpolate = true;	
			mPRTHierarchy->UpdateState(mVisualizeError, mInterpolate, mNumNN);
		}
  }

	if( gDInput->keyDown(DIK_O) ) {
		if(mInterpolate){ 
			mInterpolate = false;	
			mPRTHierarchy->UpdateState(mVisualizeError, mInterpolate, mNumNN);
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

  ID3DXEffect* effect = mFX;
  D3DXHANDLE handle;
  D3DXMATRIX mWVP = mCamera->view() * mProj;
  mCubeMap->DrawCubeMap(&mWVP);
  effect->SetTexture( "EnvMap", mCubeMap->GetTexture() );
    
  effect->SetBool("useTextures", mPRTHierarchy->HasTextures());
  
  handle = effect->GetTechniqueByName("PRTLighting");
  effect->SetFloat( "gReflectivity", reflectivity );
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

void MeshPRT::buildEffectFiles()
{
	UINT NUM_COEFFICIENTS = mPRTHierarchy->GetRenderMesh()->GetPRTCompBuffer()->GetNumCoeffs();
	D3DXMACRO aDefines[2];

	CHAR szMaxNumCoefficients[64];
	sprintf_s( szMaxNumCoefficients, 64, "%d", NUM_COEFFICIENTS );
	szMaxNumCoefficients[63] = 0;
	
	aDefines[0].Name = "NUM_COEFFICIENTS";
	aDefines[0].Definition = szMaxNumCoefficients;
	aDefines[1].Name = 0;
	aDefines[1].Definition = 0;		

	WCHAR* effectName = L"shader/diffuse.fx";
	LoadEffectFile( gd3dDevice, effectName, aDefines, 
					D3DXSHADER_DEBUG | D3DXSHADER_FORCE_PS_SOFTWARE_NOOPT, &mFX);
	
	PD(D3D_OK, L"done building FX");
		
	mPRTHierarchy->LoadEffect(mFX);
}

void MeshPRT::buildProjMtx()
{
  float w = (float)md3dPP.BackBufferWidth;
  float h = (float)md3dPP.BackBufferHeight;
  D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
