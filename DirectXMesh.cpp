#include "DirectXMesh.h"

struct LOCAL_VERTEX
{
  D3DXVECTOR3 pos;
  D3DXVECTOR3 norm;
};

DirectXMesh::DirectXMesh() {
  CreateDevice();
}

DirectXMesh::~DirectXMesh() {
  SAFE_RELEASE(mMesh)
  SAFE_RELEASE(pd3d9Device)
  SAFE_RELEASE(pd3d9)
}

void DirectXMesh::CreateDevice() {
  mMesh = NULL;
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
  for(int i = 0; i < m.cm.vert.size(); i++)
  {
    Vertex vertex;
    vertex.x = m.cm.vert[i].P()[0];
    vertex.y = m.cm.vert[i].P()[1];
    vertex.z = m.cm.vert[i].P()[2];

    vertices.push_back(vertex);
    index[vertex] = indexCounter++;
  }

  for(int i = 0; i < m.cm.face.size(); i++){
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
  HRESULT hr;

  std::vector<Vertex> vertices;
  std::vector<Face> faces;

  ParseMesh(m, vertices, faces);
  WriteToFile(vertices, faces);

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
                      localVertDecl, pd3d9Device, &mMesh);

  PD(hr, L"create mesh");
  if(FAILED(hr)) return hr;

  LOCAL_VERTEX *pVertices = NULL;
  hr = mMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < numVertices; ++i )
  {
    pVertices[i].pos = D3DXVECTOR3(vertices[i].x,
                                   vertices[i].y,
                                   vertices[i].z);
  }

  hr = mMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  DWORD* pIndices = NULL;

  hr = mMesh->LockIndexBuffer(0, (void**)&pIndices);
  PD(hr, L"lock index buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < numFaces; i++ )
  {
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

HRESULT DirectXMesh::SaveMeshToFile(const WCHAR* filename) {
  HRESULT hr;

  DWORD dwFormat = D3DXF_FILEFORMAT_TEXT;
  DWORD numFaces = mMesh->GetNumFaces();
  PD(L"number of faces: ", numFaces);
  PD(L"number of vertices: ", mMesh->GetNumVertices());
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

  hr = D3DXSaveMeshToX(filename, mMesh, pAdjacency, materials, NULL,
                       dwNumMeshes, dwFormat);

  PD(hr, L"save mesh to file");
  PD(hr);

  delete [] pAdjacency;
  return hr;
}

HRESULT DirectXMesh::CloneMesh(ID3DXMesh** target) {
  HRESULT hr;

  D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
  hr = mMesh->GetDeclaration(decl);
  PD(hr, L"get vertex declaration");
  if(FAILED(hr)) return hr;

  hr = mMesh->CloneMesh( mMesh->GetOptions(), decl, pd3d9Device, target );
  PD(hr, L"clone mesh");

  return hr;
}

void DirectXMesh::WriteToFile(std::vector<Vertex> &vertices,
                              std::vector<Face> &faces)
{
  PD(D3D_OK, L"write to file");

  std::ofstream testfile;
  testfile.open("../meshlabplugins/filter_meshprt/export/testfile");
  testfile << "Test";
  testfile.close();

  std::ofstream outfile;
  outfile.open("../meshlabplugins/filter_meshprt/export/meshdump");

  DWORD nrOfVertices = vertices.size();
  DWORD nrOfFaces = faces.size();

  outfile << nrOfVertices << "\n";

  for(int i = 0; i < nrOfVertices; ++i) {
    Vertex vertex = vertices[i];
    float x = vertex.x;
    float y = vertex.y;
    float z = vertex.z;
    outfile << "vertex " << i << ": " << x << ", " << y << ", " << z << "\n";
  }

  outfile << "\n";
  outfile << nrOfFaces << "\n";

  for(int i = 0; i < nrOfFaces; ++i) {
    Face face = faces[i];
    DWORD i0 = face.vertices[0];
    DWORD i1 = face.vertices[1];
    DWORD i2 = face.vertices[2];
    outfile << "face " << i << ": " << i0 << ", " << i1 << ", " << i2 << "\n";
  }

  outfile.close();
}

