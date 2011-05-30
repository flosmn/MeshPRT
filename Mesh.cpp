#include "Mesh.h"

bool DoesMeshHaveUsage( ID3DXMesh* pMesh, BYTE Usage );

Mesh::Mesh() {
}

Mesh::Mesh(IDirect3DDevice9 *device) {
  mMesh = 0;  
  mRotationX = 0;
  mRotationY = 0;
  mRotationZ = 0;  
  
  mDevice = device;
  D3DXMatrixIdentity(&mWorld);

  mDiffuseMtrl[0].r = 1.0f; mDiffuseMtrl[0].g = 0.0f; mDiffuseMtrl[0].b = 0.0f; mDiffuseMtrl[0].a = 1.0f;
  mDiffuseMtrl[1].r = 1.0f; mDiffuseMtrl[1].g = 1.0f; mDiffuseMtrl[1].b = 1.0f; mDiffuseMtrl[1].a = 1.0f;
  mDiffuseMtrl[2].r = 0.0f; mDiffuseMtrl[2].g = 0.0f; mDiffuseMtrl[2].b = 1.0f; mDiffuseMtrl[2].a = 1.0f;

  mSpecularMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
  mSpecularPower = 8.0f;
}

D3DXCOLOR Mesh::getDiffuseMaterial(int i) {
  return mDiffuseMtrl[i%3];
}

void Mesh::loadFX(ID3DXEffect *effect) {
  mEffect = effect;

  mhDiffuseMtrl           = mEffect->GetParameterByName(0, "gDiffuseMtrl");
  mhSpecularMtrl          = mEffect->GetParameterByName(0, "gSpecularMtrl");
  mhSpecularPower         = mEffect->GetParameterByName(0, "gSpecularPower");
  mhWorld                 = mEffect->GetParameterByName(0, "gWorld");
  mhWorldInverseTranspose = mEffect->GetParameterByName(0, "gWorldInverseTranspose");
}

Mesh::~Mesh() {
  if(mMesh) ReleaseCOM(mMesh);
  if(mPRTCompBuffer) ReleaseCOM(mPRTCompBuffer);
}

ID3DXMesh* Mesh::getMesh() {
  return mMesh;
}

void Mesh::drawMesh() {
  D3DXMATRIX rotX, rotY, rotZ, translation;
  D3DXMatrixRotationX(&rotX, mRotationX);
  D3DXMatrixRotationY(&rotY, mRotationY);
  D3DXMatrixRotationZ(&rotZ, mRotationZ);
  D3DXMatrixTranslation(&translation, 0.0f, 0.0f, 5.0f);
  mWorld = translation * rotX * rotY * rotZ;

  D3DXMATRIX worldInverseTranspose;
  D3DXMatrixInverse(&worldInverseTranspose, 0, &mWorld);
  D3DXMatrixTranspose(&worldInverseTranspose, &worldInverseTranspose);

  mEffect->SetMatrix(mhWorldInverseTranspose, &worldInverseTranspose);
  mEffect->SetValue(mhWorld, &mWorld, sizeof(D3DXMATRIX));
  mEffect->SetValue(mhSpecularMtrl, &mSpecularMtrl, sizeof(D3DXCOLOR));
  mEffect->SetValue(mhSpecularPower, &mSpecularPower, sizeof(D3DXCOLOR));
  mEffect->CommitChanges();

  DWORD dwNumMeshes = 0;
  getMesh()->GetAttributeTable( NULL, &dwNumMeshes );
  for( UINT i = 0; i < dwNumMeshes; i++ ) {
    mEffect->SetValue(mhDiffuseMtrl, &getDiffuseMaterial(i), sizeof(D3DXCOLOR));
    mEffect->CommitChanges();
    mMesh->DrawSubset( i );
  }
}

DWORD Mesh::GetNumFaces() {
  return mMesh->GetNumFaces();
}

DWORD Mesh::GetNumVertices() {
  return mMesh->GetNumVertices();
}

HRESULT Mesh::AdjustMeshDecl( IDirect3DDevice9* pd3dDevice, ID3DXMesh** ppMesh )
{
  LPD3DXMESH pInMesh = *ppMesh;
  LPD3DXMESH pOutMesh = NULL;

  D3DVERTEXELEMENT9 vertDecl[MAX_FVF_DECL_SIZE] =
  {
    {0,  0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    {0,  24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},

    {0,  32, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
    {0,  36, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 1},
    {0,  52, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 2},
    {0,  68, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 3},
    {0,  84, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 4},
    {0, 100, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 5},
    {0, 116, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 6},
    D3DDECL_END()
  };

  PD( pInMesh->CloneMesh( pInMesh->GetOptions(), vertDecl, pd3dDevice, &pOutMesh ) , L"clone mesh");

  if( !DoesMeshHaveUsage( pInMesh, D3DDECLUSAGE_NORMAL ) )
    PD( D3DXComputeNormals( pOutMesh, NULL ), L"compute normals");

  ReleaseCOM( pInMesh )

  *ppMesh = pOutMesh;

  return S_OK;
}

bool DoesMeshHaveUsage( ID3DXMesh* pMesh, BYTE Usage )
{
  D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
  pMesh->GetDeclaration( decl );

  for( int di = 0; di < MAX_FVF_DECL_SIZE; di++ )
  {
    if( decl[di].Usage == Usage )
      return true;
    if( decl[di].Stream == 255 )
      return false;
  }

  return false;
}

HRESULT AttribSortMesh( ID3DXMesh** ppInOutMesh )
{
  ID3DXMesh* pInputMesh = *ppInOutMesh;

  ID3DXMesh* pTempMesh = NULL;
  DWORD* rgdwAdjacency = new DWORD[pInputMesh->GetNumFaces() * 3];
  DWORD* rgdwAdjacencyOut = new DWORD[pInputMesh->GetNumFaces() * 3];

  PD( pInputMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency ),
      L"generate adjacency" );

  PD( pInputMesh->Optimize( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT ,
                            rgdwAdjacency, rgdwAdjacencyOut, NULL, NULL,
                            &pTempMesh ),
      L"optimize" );

  ReleaseCOM(pInputMesh);
  pInputMesh = pTempMesh;
  delete []rgdwAdjacency;
  delete []rgdwAdjacencyOut;
  *ppInOutMesh = pTempMesh;

  return S_OK;
}

void Mesh::setPRTConstantsInEffect() {
  DWORD mNumChannels = 3;
  DWORD mOrder = 5;
  DWORD dwNumPCA = mNumChannels * mOrder * mOrder;
  if(dwNumPCA > 24) dwNumPCA = 24;
  UINT dwNumClusters = mPRTCompBuffer->GetNumClusters();
  
  PD( mEffect->SetFloatArray( "aPRTConstants", mPRTConstants,
                              dwNumClusters * ( 4 + mNumChannels * dwNumPCA ) ),
      L"set constants in effect" );
}

HRESULT Mesh::LoadMesh()
{
  ReleaseCOM( mMesh )
  
  WCHAR* model = L"models/bigship1.x";
  D3DXLoadMeshFromX(AppendToRootDir(model) , D3DXMESH_MANAGED | D3DXMESH_32BIT,
                    mDevice, NULL, NULL, NULL, NULL, &mMesh );
  
  AdjustMeshDecl( mDevice, &mMesh );
  AttribSortMesh( &mMesh );

  return S_OK;
}

HRESULT Mesh::CreateMeshFrom(std::vector<Vertex> vertices, std::vector<Face> faces){
  DWORD numFaces = faces.size();
  DWORD numVertices = vertices.size();

  D3DVERTEXELEMENT9 localVertDecl[MAX_FVF_DECL_SIZE] =
  {
    {0,  0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0,  12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0},
    D3DDECL_END()
  };

  struct LocalVertex {
    D3DXVECTOR3 pos;
    D3DXVECTOR3 normal;
  };
  
  PD(D3DXCreateMesh(numFaces, numVertices, D3DXMESH_MANAGED | D3DXMESH_32BIT,
                    localVertDecl, mDevice, &mMesh),
     L"create mesh");

  LocalVertex* pVertices = NULL;
  mMesh->LockVertexBuffer(0, (void**)&pVertices);
  for ( DWORD i = 0; i < numVertices; ++i )
  {
    pVertices[i].pos = D3DXVECTOR3(vertices[i].x, vertices[i].y, vertices[i].z);
  }
  mMesh->UnlockVertexBuffer();

  DWORD* pIndices = NULL;
  mMesh->LockIndexBuffer(0, (void**)&pIndices);
  for ( DWORD i = 0; i < numFaces; i++ )
  {
    pIndices[3 * i]     = faces[i].vertices[0];
    pIndices[3 * i + 1] = faces[i].vertices[1];
    pIndices[3 * i + 2] = faces[i].vertices[2];
  }
  mMesh->UnlockIndexBuffer();

  DWORD* pAdjacency = new DWORD[numFaces * 3];
  mMesh->GenerateAdjacency(1e-6f, pAdjacency);

  PD(D3DXComputeNormals(mMesh, pAdjacency),
     L"compute normals");

  delete [] pAdjacency;

  AdjustMeshDecl( mDevice, &mMesh );
  AttribSortMesh( &mMesh );

  return S_OK;
}
