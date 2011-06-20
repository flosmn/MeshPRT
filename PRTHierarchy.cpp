#include "PRTHierarchy.h"
#include <time.h>

PRTHierarchy::PRTHierarchy(IDirect3DDevice9 *device) {
  mDevice = device;

  srand ( time(NULL) );

  mVisualizeMapping = false;

  mRenderMesh = 0;
  mApproxMesh = 0;
  mPRTEngine = 0;
  mPRTHierarchyMapping = 0;
  mOrder = 0;
  D3DXMatrixIdentity(&mWorldTransform);

  mPRTHierarchyMapping = new PRTHierarchyMapping();
}

PRTHierarchy::~PRTHierarchy() {
  delete mRenderMesh;
  delete mApproxMesh;
  delete mPRTEngine;
  delete mPRTHierarchyMapping;
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

HRESULT PRTHierarchy::ScaleMeshes() {
  HRESULT hr;

  ID3DXMesh* d3dMesh = mRenderMesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;
   
  D3DXVECTOR3 center;
  float radius;

  hr = D3DXComputeBoundingSphere((D3DXVECTOR3*)pVertices, 
                                 d3dMesh->GetNumVertices(),
                                 d3dMesh->GetNumBytesPerVertex(), 
                                 &center, &radius);

  PD(hr, L"compute bounding sphere");
  if(FAILED(hr)) return hr;

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  PD(L"radius: ", radius);

  float scale = 10.0f/radius;
  D3DXMatrixScaling(&mWorldTransform, scale, scale, scale); 

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
  
  if(mVisualizeMapping) {
    FillWithRandomColors(mApproxMeshVertexColors);
    PD(L"fill approx mesh with random vertex colors");
  }

  mPRTHierarchyMapping->NearestNeighbourMappingTree( mApproxMeshVertices,
                                                     mRenderMeshVertices,
                                                     mApproxMeshVertexColors,
                                                     mRenderMeshVertexColors);

  hr = SetRenderMeshVertexColors();
  PD(hr, L"set vertex colors for render mesh");
  if(FAILED(hr)) return hr;
                                                
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
    Vertex vertex;
    vertex.pos.x = pVertices[i].position.x;
    vertex.pos.y = pVertices[i].position.y;
    vertex.pos.z = pVertices[i].position.z;
    vertex.normal.x = pVertices[i].normal.x;
    vertex.normal.y = pVertices[i].normal.y;
    vertex.normal.z = pVertices[i].normal.z;
    vec.push_back(vertex);
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

HRESULT PRTHierarchy::SetRenderMeshVertexColors()
{
  HRESULT hr;
  
  ID3DXMesh* d3dMesh = mRenderMesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < d3dMesh->GetNumVertices(); ++i ) {
    pVertices[i].blendWeight1 = mRenderMeshVertexColors[i];
  }

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;
  
  return D3D_OK;
}

void PRTHierarchy::FillWithRandomColors(std::vector<D3DXCOLOR> &colors){
  for(int i = 0; i < colors.size(); ++i) {
    colors[i] = GenerateRandomColor();
  }
}

D3DXCOLOR PRTHierarchy::GenerateRandomColor() {
  D3DXCOLOR color;
  color.r = std::rand() / (float)RAND_MAX;
  color.g = std::rand() / (float)RAND_MAX; 
  color.b = std::rand() / (float)RAND_MAX; 
  color.a = 1.0f;

  return color;
}

void PRTHierarchy::LoadEffect(ID3DXEffect* mEffect){
  mRenderMesh->LoadFX(mEffect);
  mApproxMesh->LoadFX(mEffect);
}

bool PRTHierarchy::HasTextures() {
  return mRenderMesh->HasTextures();
}

void PRTHierarchy::DrawMesh() {
  mRenderMesh->SetWorldTransform(&mWorldTransform);
  mRenderMesh->DrawMesh();
}

int PRTHierarchy::GetNumVertices() {
  return mRenderMesh->GetNumVertices();
}

int PRTHierarchy::GetNumFaces() {
  return mRenderMesh->GetNumFaces();
}