#include "DirectXMesh.h"

DirectXMesh::DirectXMesh() {
  CreateDevice();
}

DirectXMesh::~DirectXMesh() {
  SAFE_RELEASE(pMesh)
  SAFE_RELEASE(pd3d9Device)
  SAFE_RELEASE(pd3d9)
}

void DirectXMesh::CreateDevice() {
  pMesh = NULL;
  pd3d9 = NULL;
  pd3d9Device = NULL;

  HRESULT hr;

  pd3d9 = Direct3DCreate9(D3D_SDK_VERSION);

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

  hr = pd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
                           D3DCREATE_SOFTWARE_VERTEXPROCESSING, &fakeParams,
                           &pd3d9Device);
  if(FAILED(hr))
  {
    hr = pd3d9->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, NULL,
                             D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                             &fakeParams, &pd3d9Device);
    if(FAILED(hr))
    {
      SAFE_RELEASE( pd3d9 )
    }
  }
}

void DirectXMesh::ParseMesh(const MeshModel &m, std::vector<Vertex>& vertices,
                            std::vector<Face> &faces){
  std::map<Vertex, DWORD, Vertex> index;

  DWORD indexCounter = 0;
  for(unsigned int i = 0; i < m.cm.vert.size(); i++)
  {
    Vertex vertex;
    vertex.x = m.cm.vert[i].P()[0];
    vertex.y = m.cm.vert[i].P()[1];
    vertex.z = m.cm.vert[i].P()[2];

    vertices.push_back(vertex);
    index[vertex] = indexCounter++;
  }

  for(unsigned int i = 0; i < m.cm.face.size(); i++){
    Vertex vertices[3];
    for(int j = 0; j < 3; j++) {
      vertices[j].x = m.cm.face[i].V(j)->P()[0];
      vertices[j].y = m.cm.face[i].V(j)->P()[1];
      vertices[j].z = m.cm.face[i].V(j)->P()[2];
    }

    Face face;
    face.vertices[0] = index[vertices[0]];
    face.vertices[1] = index[vertices[1]];
    face.vertices[2] = index[vertices[2]];
    faces.push_back(face);
  }
}

HRESULT DirectXMesh::CreateDirectXMesh(const MeshModel &m) {
  OutputDebugStr(L"create directX mesh from meshDocument");

  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  ParseMesh(m, vertices, faces);

  DWORD numFaces = faces.size();
  DWORD numVertices = vertices.size();

  D3DVERTEXELEMENT9 localVertDecl[MAX_FVF_DECL_SIZE] =
  {
    {0,  0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    D3DDECL_END()
  };

  struct LOCAL_VERTEX
  {
    D3DXVECTOR3 pos;
    D3DXVECTOR3 norm;
  };

  PD(D3DXCreateMesh(numFaces, numVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT,
                    localVertDecl, pd3d9Device, &pMesh),
     L"create mesh");

  LOCAL_VERTEX *pVertices = NULL;
  PD(pMesh->LockVertexBuffer(0, (void**)&pVertices), L"lock vertex buffer");
  for ( DWORD i = 0; i < numVertices; ++i )
  {
    pVertices[i].pos = D3DXVECTOR3(vertices[i].x, vertices[i].y, vertices[i].z);
  }
  PD(pMesh->UnlockVertexBuffer(), L"unlock vertex buffer");

  DWORD* pIndices = NULL;
  PD(pMesh->LockIndexBuffer(0, (void**)&pIndices), L"lock index buffer");
  for ( DWORD i = 0; i < numFaces; i++ )
  {
    pIndices[3 * i]     = faces[i].vertices[0];
    pIndices[3 * i + 1] = faces[i].vertices[1];
    pIndices[3 * i + 2] = faces[i].vertices[2];
  }
  PD(pMesh->UnlockIndexBuffer(), L"unlock index buffer");

  DWORD* pAdjacency = new DWORD[numFaces * 3];
  PD(pMesh->GenerateAdjacency(1e-6f, pAdjacency), L"generate adjacency");

  PD(D3DXComputeNormals(pMesh, pAdjacency), L"compute normals");

  delete [] pAdjacency;

  return D3D_OK;
}

HRESULT DirectXMesh::SaveMeshToFile(const WCHAR* filename) {
  DWORD dwFormat = D3DXF_FILEFORMAT_TEXT;
  DWORD numFaces = pMesh->GetNumFaces();
  DWORD* pAdjacency = new DWORD[numFaces * 3];
  PD(pMesh->GenerateAdjacency(1e-6f, pAdjacency), L"generate adjacency");

  DWORD dwNumMeshes = 0;
  PD( pMesh->GetAttributeTable( NULL, &dwNumMeshes ), L"get number of meshes");
  D3DXMATERIAL* materials = new D3DXMATERIAL[dwNumMeshes];

  for(int i = 0; i < dwNumMeshes; ++i) {
    materials[i].MatD3D.Ambient = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Specular = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Emissive = D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.0f);
    materials[i].MatD3D.Diffuse = D3DXCOLOR(1.0f, 0.5f, 0.0f, 0.0f);
    materials[i].pTextureFilename = NULL;
  }

  PD(D3DXSaveMeshToX(filename, pMesh, pAdjacency, materials, NULL,
                     dwNumMeshes, dwFormat),
     L"save mesh to file");

  delete [] pAdjacency;
  return D3D_OK;
}

