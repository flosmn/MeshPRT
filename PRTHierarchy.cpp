#include "PRTHierarchy.h"

PRTHierarchy::PRTHierarchy(IDirect3DDevice9 *device) {
  mDevice = device;

  mRenderMesh = 0;
  mApproxMesh = 0;
  mPRTEngine = 0;

  mOrder = 0;
}

PRTHierarchy::~PRTHierarchy() {
  delete mRenderMesh;
  delete mApproxMesh;
  delete mPRTEngine;
}

HRESULT PRTHierarchy::LoadMeshHierarchy(WCHAR* renderMeshFile, 
                                        WCHAR* approxMeshFile, 
                                        WCHAR* directory,
                                        WCHAR* extension,
                                        DWORD order)
{
  HRESULT hr;

  mOrder = order;

  mPRTEngine = new PRTEngine(mDevice, mOrder);
  mRenderMesh = new Mesh(mDevice);
  mApproxMesh = new Mesh(mDevice);
  
  hr = mRenderMesh->LoadMesh(directory, renderMeshFile, extension);
  PD(hr, L"load render mesh file");
  if(FAILED(hr)) return hr;

  hr = mApproxMesh->LoadMesh(directory, approxMeshFile, extension);
  PD(hr, L"load approx mesh file");
  if(FAILED(hr)) return hr;

  hr = FillVertexVectors();
  PD(hr, L"fill vertex vectors");
  if(FAILED(hr)) return hr;
  
  return D3D_OK;
}

HRESULT PRTHierarchy::CalculateSHCoefficients(LightSource* lightSource) {
  HRESULT hr;
  
  hr = mPRTEngine->CalculateSHCoefficients(mApproxMesh);
  PD(hr, L"calculate coefficients for approx mesh");
  if(FAILED(hr)) return  hr;

  hr = lightSource->CalculateSHCoefficients(mOrder);
  PD(hr, L"calculate coefficients for lightsource");
  if(FAILED(hr)) return hr;

  hr = mPRTEngine->ConvoluteSHCoefficients(mApproxMesh, lightSource);
  PD(hr, L"convolute coefficients of approx mesh and lightsource");
  if(FAILED(hr)) return hr;
    
  return D3D_OK;
}

HRESULT PRTHierarchy::CalculateDiffuseColor() {
  HRESULT hr;

  hr = mPRTEngine->CalculateDiffuseColor(mApproxMesh);
  PD(hr, L"calculate diffuse color for approx mesh");
  if(FAILED(hr)) return hr;

  hr = FillColorVector(mApproxMeshVertexColors, mApproxMesh);
  PD(hr, L"fill approx mesh vertex color vector");
  if(FAILED(hr)) return hr;

  PD(L"size of approx mesh color vector: ", (int)mApproxMeshVertexColors.size());

  return D3D_OK;
}

HRESULT PRTHierarchy::UpdateLighting(LightSource* lightSource) {
  HRESULT hr;
  
  hr = lightSource->CalculateSHCoefficients(mOrder);
  PD(hr, L"calculate coefficients for lightsource");
  if(FAILED(hr)) return hr;

  hr = mPRTEngine->ConvoluteSHCoefficients(mApproxMesh, lightSource);
  PD(hr, L"convolute coefficients of approx mesh and lightsource");
  if(FAILED(hr)) return hr;

  hr = mPRTEngine->CalculateDiffuseColor(mApproxMesh);
  PD(hr, L"calculate diffuse color for approx mesh");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}

HRESULT PRTHierarchy::FillVertexVectors() {
  HRESULT hr;

  hr = FillVertexVector(mRenderMeshVertices, mRenderMesh);
  PD(hr, L"fill render mesh vertex vector");
  if(FAILED(hr)) return hr;

  hr = FillVertexVector(mApproxMeshVertices, mApproxMesh);
  PD(hr, L"fill approx mesh vertex vector");
  if(FAILED(hr)) return hr;

  PD(L"size of render mesh vertex vector: ", (int)mRenderMeshVertices.size());
  PD(L"size of approx mesh vertex vector: ", (int)mApproxMeshVertices.size());

  return D3D_OK;
}

HRESULT PRTHierarchy::FillVertexVector(std::vector<Vertex> &vec, Mesh* mesh) {
  HRESULT hr;

  ID3DXMesh* d3dMesh = mesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < d3dMesh->GetNumVertices(); ++i ) {
    Vertex pos;
    pos.x = pVertices[i].position.x;
    pos.y = pVertices[i].position.y;
    pos.z = pVertices[i].position.z;
    vec.push_back(pos);
  }

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}

HRESULT PRTHierarchy::FillColorVector(std::vector<D3DXCOLOR> &colors,
                                      Mesh* mesh)
{
  HRESULT hr;
  
  ID3DXMesh* d3dMesh = mesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < d3dMesh->GetNumVertices(); ++i ) {
    D3DXCOLOR color = pVertices[i].blendWeight1;
    colors.push_back(color);
  }

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;
  
  return D3D_OK;
}

void PRTHierarchy::LoadEffect(ID3DXEffect* mEffect){
  mRenderMesh->LoadFX(mEffect);
  mApproxMesh->LoadFX(mEffect);
}

bool PRTHierarchy::HasTextures() {
  return mRenderMesh->HasTextures();
}

void PRTHierarchy::DrawMesh() {
  mApproxMesh->DrawMesh();
}

int PRTHierarchy::GetNumVertices() {
  return mRenderMesh->GetNumVertices();
}

int PRTHierarchy::GetNumFaces() {
  return mRenderMesh->GetNumFaces();
}