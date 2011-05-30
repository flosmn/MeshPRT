#include <list>

#include "d3dApp.h"
#include "DirectInput.h"
#include "GfxStats.h"
#include "Camera.h"
#include "PRTEngine.h"

class DiffuseDemo : public D3DApp
{
public:
  DiffuseDemo(std::string winCaption, D3DDEVTYPE devType,
              DWORD requestedVP);
  ~DiffuseDemo();

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
  bool phongShading;

  GfxStats* mGfxStats;

  ID3DXEffect* mFX;
  D3DXHANDLE   mhPerVertexLightingTechnique;
  D3DXHANDLE   mhPerPixelLightingTechnique;
  D3DXHANDLE   mhPRTLightingTechnique;
  D3DXHANDLE   mhView;
  D3DXHANDLE   mhProjection;
  D3DXHANDLE   mhEyePosW;
  D3DXHANDLE   mhLightPositionW;
  D3DXHANDLE   mhLightVecW;
  D3DXHANDLE   mhLightColor;

  D3DXMATRIX mWorld;
  D3DXMATRIX mProj;

  Mesh *mMesh;
  Camera *mCamera;
  PRTEngine* mPRTEngine;
  Light* mLight;
};

bool StartDirectX() {
  // Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
  _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

  DiffuseDemo app("Diffuse Demo", D3DDEVTYPE_HAL, D3DCREATE_HARDWARE_VERTEXPROCESSING);
  gd3dApp = &app;

  DirectInput di(DISCL_NONEXCLUSIVE|DISCL_FOREGROUND, DISCL_NONEXCLUSIVE|DISCL_FOREGROUND);
  gDInput = &di;

  return gd3dApp->run();
}

DiffuseDemo::DiffuseDemo(std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP)
  : D3DApp(winCaption, devType, requestedVP)
{
  if(!checkDeviceCaps())
  {
    MessageBox(0, L"checkDeviceCaps() Failed", 0, 0);
    PostQuitMessage(0);
  }

  phongShading = false;

  mMesh = new Mesh(gd3dDevice);
  mMesh->LoadMesh();
  
  mPRTEngine = new PRTEngine(gd3dDevice);
  
  mLight = new Light(D3DXVECTOR3(0.0f, 5.0f, -5.0f), D3DXVECTOR3(0.0f, -1.0f, 1.0f), D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
  mLight->calculateSHCoefficients();

  mPRTEngine->getCoefficientsForMesh(mMesh, mLight);

  buildFX(mMesh);
  mMesh->loadFX(mFX);

  mMesh->setPRTConstantsInEffect();

  mGfxStats = new GfxStats();

  D3DXMatrixIdentity(&mWorld);
  
  onResetDevice();

  mCamera = new Camera();  
}

DiffuseDemo::~DiffuseDemo()
{
  delete mGfxStats;
  delete mMesh;
  delete mPRTEngine;
  delete mLight;
  delete mCamera;
}

bool DiffuseDemo::checkDeviceCaps()
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

void DiffuseDemo::onLostDevice()
{
  mGfxStats->onLostDevice();
  mFX->OnLostDevice();
}

void DiffuseDemo::onResetDevice()
{
  mGfxStats->onResetDevice();
  mFX->OnResetDevice();

  // The aspect ratio depends on the backbuffer dimensions, which can
  // possibly change after a reset.  So rebuild the projection matrix.
  buildProjMtx();
}

void DiffuseDemo::updateScene(float dt)
{
  mGfxStats->setVertexCount(mMesh->GetNumVertices());
  mGfxStats->setTriCount(mMesh->GetNumFaces());
  mGfxStats->update(dt);
  mCamera->update(dt);

  // Get snapshot of input devices.
  gDInput->poll();

  if( gDInput->keyDown(DIK_P) )	 
    phongShading = !phongShading;
}


void DiffuseDemo::drawScene()
{
  D3DXCOLOR color = D3DXCOLOR(0.3f, 0.3f, 0.3f, 0.0f);
  // Clear the backbuffer and depth buffer.
  gd3dDevice->Clear(0, 0, D3DCLEAR_ZBUFFER, 0xffeeeeee, 1.0f, 0);
  gd3dDevice->Clear(0, 0, D3DCLEAR_TARGET, color, 1.0f, 0);

  gd3dDevice->BeginScene();

  // Setup the rendering FX
  if(phongShading){
    mFX->SetTechnique(mhPerPixelLightingTechnique);
  }
  else {
    mFX->SetTechnique(mhPRTLightingTechnique);
  }

  mFX->SetMatrix(mhView, &(mCamera->view()));
  mFX->SetValue(mhEyePosW, &(mCamera->pos()), sizeof(D3DXVECTOR3));
  mFX->SetMatrix(mhProjection, &mProj);

  mFX->SetValue(mhLightVecW, &mLight->getLightDirection(), sizeof(D3DXVECTOR3));
  mFX->SetValue(mhLightPositionW, &mLight->getLightPosition(), sizeof(D3DXCOLOR));
  mFX->SetValue(mhLightColor, &mLight->getLightColor(), sizeof(D3DXCOLOR));

  // Begin passes.
  UINT numPasses = 0;
  mFX->Begin(&numPasses, 0);
  for(UINT i = 0; i < numPasses; ++i)
  {
    mFX->BeginPass(i);
    mMesh->drawMesh();
    mFX->EndPass();
  }
  mFX->End();


  mGfxStats->display();

  gd3dDevice->EndScene();

  // Present the backbuffer.
  gd3dDevice->Present(0, 0, 0, 0);
}

void DiffuseDemo::buildFX(Mesh* mesh)
{
  UINT dwNumChannels = 3; //mesh->getPRTCompBuffer()->GetNumChannels();
  UINT dwNumClusters = 4; //mesh->getPRTCompBuffer()->GetNumClusters();
  UINT dwNumPCA = 12; //mesh->getPRTCompBuffer()->GetNumPCA();

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

  // Create the FX from a .fx file.
  ID3DXBuffer* errors = 0;
  LPCWSTR effectName = L"../../shader/diffuse.fx";

  PD(D3D_OK, L"try create effect from file: ");
  PD(D3D_OK, effectName);
  HRESULT hr = D3DXCreateEffectFromFile(gd3dDevice, effectName,
                              aDefines, 0, D3DXSHADER_DEBUG, 0, &mFX, &errors);
  PD( hr, L"create effect from file" );

  if( errors ) {
    char* c_error = (char*)errors->GetBufferPointer();
    WCHAR* w_error;
    CharArrayToWCharArray(c_error, w_error);
    OutputDebugString(w_error);
    delete [] w_error;
    delete [] c_error;
  }

  // Obtain handles.
  mhPerVertexLightingTechnique = mFX->GetTechniqueByName("PerVertexLighting");
  mhPerPixelLightingTechnique = mFX->GetTechniqueByName("PerPixelLighting");
  mhPRTLightingTechnique  = mFX->GetTechniqueByName("PRTLighting");
  mhView                  = mFX->GetParameterByName(0, "gView");
  mhProjection            = mFX->GetParameterByName(0, "gProjection");
  mhLightVecW             = mFX->GetParameterByName(0, "gLightVecW");
  mhEyePosW               = mFX->GetParameterByName(0, "gEyePosW");
  mhLightColor            = mFX->GetParameterByName(0, "gLightColor");
  mhLightPositionW        = mFX->GetParameterByName(0, "gLightPosW");

  PD(D3D_OK, L"done building FX");
}

void DiffuseDemo::buildProjMtx()
{
  float w = (float)md3dPP.BackBufferWidth;
  float h = (float)md3dPP.BackBufferHeight;
  D3DXMatrixPerspectiveFovLH(&mProj, D3DX_PI * 0.25f, w/h, 1.0f, 5000.0f);
}
