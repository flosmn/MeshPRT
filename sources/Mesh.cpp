#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::Mesh(IDirect3DDevice9 *device) 
{
  mMaterialBuffer = 0;
  mNumMaterials = 0;
  mPRTClusterBases = 0;
  mPRTCompBuffer = 0;
  mPCAWeights = 0;
  mSHCoefficients = 0;
	mInterpolatedSHCoefficients = 0;
  mClusterIds = 0;
  mMappingIndices = 0;
  mMappingWeights = 0;

  hasTextures = false;
  mMesh = 0;  
  mRotationX = 0;
  mRotationY = 0;
  mRotationZ = 0;  
  
  mDevice = device;
  D3DXMatrixIdentity(&mWorld);

  mDiffuseMtrl[0].r = 1.0f;
  mDiffuseMtrl[0].g = 1.0f;
  mDiffuseMtrl[0].b = 1.0f;
  mDiffuseMtrl[0].a = 1.0f;
  
  mDiffuseMtrl[1].r = 1.0f;
  mDiffuseMtrl[1].g = 1.0f;
  mDiffuseMtrl[1].b = 1.0f; 
  mDiffuseMtrl[1].a = 1.0f;
  
  mDiffuseMtrl[2].r = 1.0f;
  mDiffuseMtrl[2].g = 1.0f;
  mDiffuseMtrl[2].b = 1.0f; 
  mDiffuseMtrl[2].a = 1.0f;

}

void Mesh::InitialiseMappingDatastructures(DWORD numNN) {
	mMappingIndices = new int[GetNumVertices() * numNN];
	mMappingWeights = new float[GetNumVertices() * numNN];
}

void Mesh::InitialiseSHDataStructures() {
	UINT numCoeffs = GetPRTCompBuffer()->GetNumCoeffs();
	UINT numClusters = GetPRTCompBuffer()->GetNumClusters();
	UINT numChannels = GetPRTCompBuffer()->GetNumChannels();
	UINT numPCA = GetPRTCompBuffer()->GetNumPCA();
	int nClusterBasisSize = ( numPCA + 1 ) * numCoeffs * numChannels;  // mean + pca-basis vectors of cluster
	int nBufferSize = nClusterBasisSize * numClusters;
  
	mPRTClusterBases = new float[nBufferSize];
	mPCAWeights = new float[GetNumVertices() * numPCA];

	UINT shCoefficientsSize = GetNumVertices() * numChannels * numCoeffs;
	mSHCoefficients = new float[shCoefficientsSize];
	mInterpolatedSHCoefficients = new float[shCoefficientsSize];
	mClusterIds = new int[GetNumVertices()];
}

D3DXCOLOR Mesh::GetDiffuseMaterial(int i) 
{
	if(HasTextures()) return D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);

	return mDiffuseMtrl[i % 3];
}

void Mesh::LoadFX(ID3DXEffect *effect) 
{
  mEffect = effect;
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
  
  D3DXMATRIX worldInverseTranspose;
  D3DXMatrixInverse( &worldInverseTranspose, 0, &mWorld );
  D3DXMatrixTranspose( &worldInverseTranspose, &worldInverseTranspose );

  mEffect->SetMatrix( mhWorldInverseTranspose, &worldInverseTranspose );
  mEffect->SetValue( mhWorld, &mWorld, sizeof(D3DXMATRIX) );
  mEffect->CommitChanges();

  DWORD dwNumMeshes = 0;
  GetMesh()->GetAttributeTable( NULL, &dwNumMeshes );
  
  for( UINT i = 0; i < dwNumMeshes; i++ ) {
    if(i < mTextures.size() && mTextures[i] != NULL)
	{
      hr = mEffect->SetTexture( "AlbedoTex", mTextures[i] );
	}   
    mEffect->CommitChanges();
    mMesh->DrawSubset( i );
  }
}

void Mesh::SetPRTClusterBases(float* prtClusterBases) { 
  SAFE_DELETE_ARRAY(mPRTClusterBases)
  mPRTClusterBases = prtClusterBases;
}

void Mesh::SetPcaWeights(float* pcaWeights) { 
  SAFE_DELETE_ARRAY(mPCAWeights)
  mPCAWeights = pcaWeights;
}

void Mesh::SetClusterIds(int* clusterIds) { 
  SAFE_DELETE_ARRAY(mClusterIds)
  mClusterIds = clusterIds;
}

void Mesh::SetSHCoefficients(float* shCoefficients) { 
  SAFE_DELETE_ARRAY(mSHCoefficients)
  mSHCoefficients = shCoefficients;
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

	// exact SH-coefficients
  {0, 32, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0},
	{0, 48, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 1},
  {0, 64, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 2},
  {0, 80, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 3},
  {0, 96, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 4},
  {0, 112, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 5},
  {0, 128, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 6},
	{0, 144, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 7},
	{0, 160, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 8},
	  
	{0, 176, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2},
	{0, 188, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 3},
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

HRESULT Mesh::LoadMesh(WCHAR* directory, WCHAR* name, WCHAR* extension)
{
  HRESULT hr;
  
  CleanUpMesh();
    
  SetDirectory( directory );
  SetName( name );
  WCHAR meshfile[120];
  WCHAR meshpathrel[120];
  WCHAR meshpath[120];
  Concat(meshfile, name, extension );
  Concat(meshpathrel, directory, meshfile );
  AppendToRootDir(meshpath, meshpathrel);

  hr = D3DXLoadMeshFromX( meshpath, 
                          D3DXMESH_MANAGED | D3DXMESH_32BIT, mDevice, NULL, 
                          &mMaterialBuffer, NULL, &mNumMaterials, &mMesh );
  
  PD(hr, L"load mesh from file");
  if(FAILED(hr)) return hr;

  mMaterials = (D3DXMATERIAL*)mMaterialBuffer->GetBufferPointer();
    
  PD( AdjustMeshDecl(), L"adjust mesh delaration" );
  PD( AttribSortMesh(), L"attribute sort mesh" );
  PD( LoadTextures(), L"load textures" );
  
  return D3D_OK;
}

HRESULT Mesh::LoadMesh(ID3DXMesh* mesh){
  HRESULT hr;

  CleanUpMesh();

  D3DVERTEXELEMENT9 decl[MAX_FVF_DECL_SIZE];
  hr = mesh->GetDeclaration(decl);
  hr = mesh->CloneMesh( mesh->GetOptions(), decl, mDevice, &mMesh );
  PD(hr, L"clone mesh");

  ReleaseCOM(mesh)

  SetDirectory( L"models/" );
  SetName( L"meshlab" );

  PD( AdjustMeshDecl(), L"adjust mesh delaration" );
  PD( AttribSortMesh(), L"attribute sort mesh" );

  return D3D_OK;
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
      WCHAR filepathrel[120];
      WCHAR filepath[120];
      Concat( filepathrel, GetDirectory(), filename_w );
      AppendToRootDir(filepath, filepathrel);

      OutputDebugString( L"texture specified for material:\n" );
      OutputDebugString( filepath );
      OutputDebugString( L"\n" );

      IDirect3DTexture9* texture;
      hr = D3DXCreateTextureFromFileEx( mDevice, filepath,
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

HRESULT Mesh::FillVertexBufferWithSHCoefficients(DWORD numNN, bool interpolate) {
	HRESULT hr;
	
	UINT numPCA = mPRTCompBuffer->GetNumPCA();
	UINT numChannels = mPRTCompBuffer->GetNumChannels();
	UINT numCoeffs = mPRTCompBuffer->GetNumCoeffs();
	UINT numClusters = mPRTCompBuffer->GetNumClusters();
	UINT numVertices = mPRTCompBuffer->GetNumSamples();

	float* shCoefficients = mInterpolatedSHCoefficients;
	if(!interpolate) shCoefficients = mSHCoefficients;
		
	FULL_VERTEX* pVertexBuffer = NULL;
	hr = mMesh->LockVertexBuffer( 0, ( void** )&pVertexBuffer );
	PD(hr, L"lock vertex buffer");
	if(FAILED(hr)) return hr;

	for( UINT j = 0; j < GetNumVertices(); j++ )
	{
		int coeffOffset = 32;
		for(int i = 0; i < numCoeffs; ++i) {
			float red    = shCoefficients[j * numCoeffs * numChannels + 0 * numCoeffs + i];
			float green  = shCoefficients[j * numCoeffs * numChannels + 1 * numCoeffs + i];
			float blue   = shCoefficients[j * numCoeffs * numChannels + 2 * numCoeffs + i];
			
			if(i == 0) pVertexBuffer[j].shCoeff1 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 1) pVertexBuffer[j].shCoeff2 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 2) pVertexBuffer[j].shCoeff3 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 3) pVertexBuffer[j].shCoeff4 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 4) pVertexBuffer[j].shCoeff5 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 5) pVertexBuffer[j].shCoeff6 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 6) pVertexBuffer[j].shCoeff7 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 7) pVertexBuffer[j].shCoeff8 = D3DXCOLOR(red, green, blue, 1.0f);
			if(i == 8) pVertexBuffer[j].shCoeff9 = D3DXCOLOR(red, green, blue, 1.0f);

			pVertexBuffer[j].index = D3DXVECTOR3(j, 0.0f, 0.0f);
		}
	}
	hr = mMesh->UnlockVertexBuffer();
	PD(hr, L"unlock vertex buffer");
	if(FAILED(hr)) return hr;

	return D3D_OK;
}

void Mesh::CleanUpMesh() 
{
  SAFE_RELEASE(mMesh)
  SAFE_RELEASE(mMaterialBuffer)
  SAFE_RELEASE(mPRTCompBuffer)

  SAFE_DELETE_ARRAY(mPRTClusterBases);  
  SAFE_DELETE_ARRAY(mPCAWeights);
  SAFE_DELETE_ARRAY(mSHCoefficients);
	SAFE_DELETE_ARRAY(mInterpolatedSHCoefficients);
  SAFE_DELETE_ARRAY(mClusterIds);
  SAFE_DELETE_ARRAY(mMappingIndices);
  SAFE_DELETE_ARRAY(mMappingWeights);

  for( int i = 0; i < mTextures.size(); i++ )
  {
    SAFE_RELEASE(mTextures[i])
  }
  
  mTextures.clear();
}
