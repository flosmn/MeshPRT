#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(IDirect3DDevice9 *device) 
{
  mMaterialBuffer = 0;
  mNumMaterials = 0;
  hasTextures = false;
  mMesh = 0;  
  mRotationX = 0;
  mRotationY = 0;
  mRotationZ = 0;  
  
  mDevice = device;
  D3DXMatrixIdentity(&mWorld);

  mDiffuseMtrl[0].r = 1.0f; mDiffuseMtrl[0].g = 1.0f; mDiffuseMtrl[0].b = 1.0f;
  mDiffuseMtrl[0].a = 1.0f;
  
  mDiffuseMtrl[1].r = 1.0f; mDiffuseMtrl[1].g = 1.0f; mDiffuseMtrl[1].b = 1.0f; 
  mDiffuseMtrl[1].a = 1.0f;
  
  mDiffuseMtrl[2].r = 1.0f; mDiffuseMtrl[2].g = 1.0f; mDiffuseMtrl[2].b = 1.0f; 
  mDiffuseMtrl[2].a = 1.0f;

  mSpecularMtrl = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
  mSpecularPower = 8.0f;
}

D3DXCOLOR Mesh::GetDiffuseMaterial(int i) 
{
  return mDiffuseMtrl[0];
}

void Mesh::LoadFX(ID3DXEffect *effect) 
{
  mEffect = effect;

  mhDiffuseMtrl           = mEffect->GetParameterByName(0, "gDiffuseMtrl");
  mhSpecularMtrl          = mEffect->GetParameterByName(0, "gSpecularMtrl");
  mhSpecularPower         = mEffect->GetParameterByName(0, "gSpecularPower");
  mhWorld                 = mEffect->GetParameterByName(0, "gWorld");
  mhWorldInverseTranspose = mEffect->GetParameterByName(0, "gWorldInverseTranspose");
}

Mesh::~Mesh() 
{
  CleanUpMesh();
}

ID3DXMesh* Mesh::GetMesh() 
{
  return mMesh;
}

void Mesh::DrawMesh() 
{
  HRESULT hr;
  D3DXMatrixIdentity(&mWorld);

  D3DXMATRIX worldInverseTranspose;
  D3DXMatrixInverse( &worldInverseTranspose, 0, &mWorld );
  D3DXMatrixTranspose( &worldInverseTranspose, &worldInverseTranspose );

  mEffect->SetMatrix( mhWorldInverseTranspose, &worldInverseTranspose );
  mEffect->SetValue( mhWorld, &mWorld, sizeof(D3DXMATRIX) );
  mEffect->SetValue( mhSpecularMtrl, &mSpecularMtrl, sizeof(D3DXCOLOR) );
  mEffect->SetValue( mhSpecularPower, &mSpecularPower, sizeof(D3DXCOLOR) );
  mEffect->CommitChanges();

  DWORD dwNumMeshes = 0;
  GetMesh()->GetAttributeTable( NULL, &dwNumMeshes );
  
  for( UINT i = 0; i < dwNumMeshes; i++ ) {
    D3DXCOLOR material;

    if(mTextures[i] != NULL)
    {
      hr = mEffect->SetTexture( "AlbedoTex", mTextures[i] );
      material = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
    }    
    else 
    {
      material = D3DXCOLOR( 1.0f, 1.0f, 1.0f, 1.0f );
    }
    
    hr = mEffect->SetValue( mhDiffuseMtrl, &material, sizeof(D3DXCOLOR) );
    
    mEffect->CommitChanges();
    mMesh->DrawSubset( i );
  }
}

DWORD Mesh::GetNumFaces() 
{
  return mMesh->GetNumFaces();
}

DWORD Mesh::GetNumVertices() 
{
  return mMesh->GetNumVertices();
}

HRESULT Mesh::AdjustMeshDecl()
{
  HRESULT hr;
  
  LPD3DXMESH pInMesh = mMesh;
  LPD3DXMESH pOutMesh = NULL;

  D3DVERTEXELEMENT9 vertDecl[ MAX_FVF_DECL_SIZE ] =
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

  hr = pInMesh->CloneMesh( pInMesh->GetOptions(), vertDecl, mDevice, &pOutMesh );
  PD( hr, L"clone mesh" );
  if( FAILED(hr) ) return hr;

  if( !DoesMeshHaveUsage( D3DDECLUSAGE_NORMAL ) )
  {
    hr = D3DXComputeNormals( pOutMesh, NULL );
    PD( hr, L"compute normals" );
    if( FAILED(hr) ) return hr;
  }

  ReleaseCOM( pInMesh )

  mMesh = pOutMesh;

  return D3D_OK;
}

bool Mesh::DoesMeshHaveUsage( BYTE Usage )
{
  D3DVERTEXELEMENT9 decl[ MAX_FVF_DECL_SIZE ];
  mMesh->GetDeclaration( decl );

  for( int di = 0; di < MAX_FVF_DECL_SIZE; di++ )
  {
    if( decl[di].Usage == Usage )
      return true;
    if( decl[di].Stream == 255 )
      return false;
  }

  return false;
}

HRESULT Mesh::AttribSortMesh()
{
  HRESULT hr;
  ID3DXMesh* pTempMesh = NULL;
  DWORD* rgdwAdjacency = new DWORD[ mMesh->GetNumFaces() * 3 ];
  DWORD* rgdwAdjacencyOut = new DWORD[ mMesh->GetNumFaces() * 3 ];

  hr = mMesh->GenerateAdjacency( 1e-6f, rgdwAdjacency );
  PD( hr, L"generate adjacency" );
  
  if( FAILED(hr) ) return hr;

  hr = mMesh->Optimize( D3DXMESHOPT_ATTRSORT | D3DXMESHOPT_COMPACT ,
                        rgdwAdjacency, rgdwAdjacencyOut, NULL, NULL,
                        &pTempMesh );
  PD( hr, L"optimize" );

  if( FAILED(hr) ) return hr;

  ReleaseCOM( mMesh )
  mMesh = pTempMesh;
  delete []rgdwAdjacency;
  delete []rgdwAdjacencyOut;
  
  return D3D_OK;
}

HRESULT Mesh::SetPRTConstantsInEffect() 
{
  HRESULT hr;

  DWORD mNumChannels = 3;
  DWORD mOrder = 5;
  DWORD dwNumPCA = mNumChannels * mOrder * mOrder;
  if(dwNumPCA > 24) dwNumPCA = 24;
  UINT dwNumClusters = mPRTCompBuffer->GetNumClusters();
  
  UINT size = dwNumClusters * ( 4 + mNumChannels * dwNumPCA );
  hr =  mEffect->SetFloatArray( "aPRTConstants", mPRTConstants, size );
  
  PD( hr, L"set float array" );
  return hr;
}

HRESULT Mesh::LoadMesh(WCHAR* directory, WCHAR* name, WCHAR* extension)
{
  CleanUpMesh();
    
  SetDirectory( directory );
  SetName( name );
  WCHAR* meshfile = Concat( name, extension );
  WCHAR* meshpath = Concat( directory, meshfile );
  
  D3DXLoadMeshFromX( AppendToRootDir(meshpath) , 
                    D3DXMESH_MANAGED | D3DXMESH_32BIT, mDevice, NULL, 
                    &mMaterialBuffer, NULL, &mNumMaterials, &mMesh );
  
  mMaterials = (D3DXMATERIAL*)mMaterialBuffer->GetBufferPointer();
    
  PD( AdjustMeshDecl(), L"adjust mesh delaration" );
  PD( AttribSortMesh(), "attribute sort mesh" );
  PD( LoadTextures(), L"load textures" );

  return S_OK;
}

HRESULT Mesh::LoadTextures() 
{
  HRESULT hr;

  for( UINT i = 0; i < mNumMaterials; i++ )
  {
    if ( mMaterials[i].pTextureFilename ) 
    {
      CHAR* filename_c = mMaterials[i].pTextureFilename;
      
      DWORD length = MultiByteToWideChar( CP_ACP, 0, filename_c, -1, NULL, 0 );
      WCHAR* filename_w = new WCHAR[length];
      MultiByteToWideChar( CP_ACP, 0, filename_c, -1, filename_w, length );
      WCHAR* filepath = Concat( GetDirectory(), filename_w );

      OutputDebugString( L"texture specified for material:\n" );
      OutputDebugString( filepath );
      OutputDebugString( L"\n" );

      IDirect3DTexture9* texture;
      hr = D3DXCreateTextureFromFileEx( mDevice, AppendToRootDir(filepath),
                                        D3DX_DEFAULT, D3DX_DEFAULT, 
                                        D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, 
                                        D3DPOOL_MANAGED, D3DX_DEFAULT, 
                                        D3DX_DEFAULT, 0, NULL, 
                                        NULL, &texture );
      PD( hr, L"create texture from file" );
      if( FAILED(hr) ) return hr;

      mTextures.push_back( texture );
      hasTextures = true;

      delete [] filename_w;
    } 
    
    else 
    {
      OutputDebugString( L"no texture specified for material\n" );
      mTextures.push_back( NULL );
    }
  }

  return D3D_OK;
}

void Mesh::CleanUpMesh() 
{
  ReleaseCOM( mMesh )
  ReleaseCOM( mMaterialBuffer );
  
  for( int i = 0; i < mTextures.size(); i++ )
  {
    ReleaseCOM( mTextures[i] )
  }
  
  mTextures.clear();
}