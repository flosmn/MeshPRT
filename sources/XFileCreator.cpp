#include "XFileCreator.h"

struct LOCAL_VERTEX
{
  D3DXVECTOR3 pos;
  D3DXVECTOR3 norm;
};

XFileCreator::XFileCreator() {
  mMesh = NULL;
  mDirect3D9 = NULL;
  mDevice = NULL;  
}

XFileCreator::~XFileCreator() {
  SAFE_RELEASE(mMesh)
  SAFE_RELEASE(mDirect3D9)
  SAFE_RELEASE(mDevice)
}

HRESULT XFileCreator::CreateXFile(WCHAR* targetFile, 
                                  std::vector<Vertex> vertices,
                                  std::vector<Face> faces)
{
  HRESULT hr;

  hr = CreateDevice();
  PD(hr, L"create device");
  if(FAILED(hr)) return hr;
    
  hr = CreateMesh(vertices, faces);
  PD(hr, L"create mesh");
  if(FAILED(hr)) return hr;

  hr = SaveMeshToFile(targetFile);
  PD(hr, L"save mesh to file");
  if(FAILED(hr)) return hr;
  
  return D3D_OK;
}

HRESULT XFileCreator::CreateDevice() {
  HRESULT hr;

  mDirect3D9 = Direct3DCreate9(D3D_SDK_VERSION);

  D3DPRESENT_PARAMETERS fakeParams;
  fakeParams.BackBufferWidth = 320;
  fakeParams.BackBufferHeight = 240;
  fakeParams.BackBufferFormat = D3DFMT_X8R8G8B8;
  fakeParams.BackBufferCount = 1;
  fakeParams.MultiSampleType = D3DMULTISAMPLE_NONE;
  fakeParams.MultiSampleQuality = 0;
  fakeParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
  fakeParams.hDeviceWindow = GetShellWindow();
  fakeParams.Windowed = true;
  fakeParams.Flags = 0;
  fakeParams.FullScreen_RefreshRateInHz = 0;
  fakeParams.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
  fakeParams.EnableAutoDepthStencil = false;

  hr = mDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
                                D3DCREATE_SOFTWARE_VERTEXPROCESSING, 
                                &fakeParams, &mDevice);
  if(FAILED(hr))
  {
    hr = mDirect3D9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, NULL,
                                  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                  &fakeParams, &mDevice);
    if(FAILED(hr))
    {
      SAFE_RELEASE(mDirect3D9)
    }
  }

  return hr;
}


HRESULT XFileCreator::CreateMesh(std::vector<Vertex> vertices, 
                                 std::vector<Face> faces) 
{
  HRESULT hr;

  DWORD numFaces = faces.size();
  DWORD numVertices = vertices.size();

  PD(L"number of vertices: ", numVertices);
  PD(L"number of faces: ", numFaces);

  D3DVERTEXELEMENT9 localVertDecl[MAX_FVF_DECL_SIZE] =
  {
    {0,  0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    D3DDECL_END()
  };

  hr = D3DXCreateMesh(numFaces, numVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT,
                      localVertDecl, mDevice, &mMesh);

  PD(hr, L"create mesh");
  if(FAILED(hr)) return hr;

  LOCAL_VERTEX *pVertices = NULL;
  hr = mMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < numVertices; ++i ) {
    pVertices[i].pos = D3DXVECTOR3(vertices[i].pos.x,
                                   vertices[i].pos.y,
                                   vertices[i].pos.z);
  }

  hr = mMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  DWORD* pIndices = NULL;

  hr = mMesh->LockIndexBuffer(0, (void**)&pIndices);
  PD(hr, L"lock index buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < numFaces; i++ ) {
    pIndices[3 * i]     = faces[i].vertices[0];
    pIndices[3 * i + 1] = faces[i].vertices[1];
    pIndices[3 * i + 2] = faces[i].vertices[2];
  }

  hr = mMesh->UnlockIndexBuffer();
  PD(hr, L"unlock index buffer");
  if(FAILED(hr)) return hr;

  DWORD* pAdjacency = new DWORD[numFaces * 3];
  hr = mMesh->GenerateAdjacency(1e-6f, pAdjacency);
  PD(hr, L"generate adjacency");
  if(FAILED(hr)) return hr;

  hr = D3DXComputeNormals(mMesh, pAdjacency);
  PD(hr, L"compute normals");
  if(FAILED(hr)) return hr;
  
  delete [] pAdjacency;
  
  return D3D_OK;
}

HRESULT XFileCreator::SaveMeshToFile(WCHAR* targetFile) {
  HRESULT hr;

  DWORD dwFormat = D3DXF_FILEFORMAT_TEXT;
  DWORD numFaces = mMesh->GetNumFaces();
  DWORD* pAdjacency = new DWORD[numFaces * 3];
  hr = mMesh->GenerateAdjacency(1e-6f, pAdjacency);
  PD(hr, L"generate adjacency");
  if(FAILED(hr)) return hr;

  DWORD dwNumMeshes = 0;
  hr = mMesh->GetAttributeTable( NULL, &dwNumMeshes );
  PD(hr, L"get number of meshes");
  if(FAILED(hr)) return hr;
  PD(L"number of meshes: ", dwNumMeshes);

  D3DXMATERIAL* materials = new D3DXMATERIAL[dwNumMeshes];
  for(int i = 0; i < dwNumMeshes; ++i) {
    materials[i].MatD3D.Ambient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Diffuse = D3DXCOLOR(1.0f, 1.0f, 1.0f, 0.0f);
    materials[i].pTextureFilename = NULL;
  }

  PD(L"save mesh with #vertices: ", mMesh->GetNumVertices());
  PD(L"save mesh with #faces: ", mMesh->GetNumFaces());

  hr = D3DXSaveMeshToX(targetFile, mMesh, pAdjacency, materials, NULL,
                       dwNumMeshes, dwFormat);

  PD(hr, L"save mesh to file");
  
  delete [] pAdjacency;

  return hr;
}