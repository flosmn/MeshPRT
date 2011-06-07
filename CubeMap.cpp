#include "CubeMap.h"

struct CUBEMAP_VERTEX
{
  D3DXVECTOR4 pos;
  D3DXVECTOR3 tex;
};


D3DVERTEXELEMENT9 g_CubeMapDeclaration[] =
{
  { 0, 0,  D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
  D3DDECL_END()
};

CubeMap::CubeMap(IDirect3DDevice9* device) 
{
  mDevice = device;
}

CubeMap::CubeMap() 
{
}

CubeMap::~CubeMap() 
{
}

HRESULT CubeMap::LoadCubeMap(WCHAR* directory, WCHAR* name, WCHAR* extension){
  HRESULT hr;

  SetDirectory(directory);
  SetName(name);
  WCHAR* cubemapfile = Concat( name, extension );
  WCHAR* cubemappath = Concat( directory, cubemapfile );

  WCHAR* path = AppendToRootDir(cubemappath);
  PD(L"Path: ", path);

  hr = D3DXCreateCubeTextureFromFile( mDevice, path, &mCubeTexture );
  
  PD(hr, L"create cube texture from file");
  if(FAILED(hr)) return hr;
    
  WCHAR* effectName = L"shader/cubemap.fx";
  hr = LoadEffectFile(gd3dDevice, effectName, 0, D3DXSHADER_DEBUG, &mEffect);
  
  PD(hr, Concat(L"load effect file ", effectName));
  if(FAILED(hr)) return hr;

  hr = mDevice->CreateVertexDeclaration( g_CubeMapDeclaration, &mVertexDecl );

  PD(hr, L"create vertex delaration for cube map");
  if(FAILED(hr)) return hr;

  hr = FillVertexBuffer();

  PD(hr, L"fill vertex buffer");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}

HRESULT CubeMap::CalculateSHCoefficients(DWORD order) {
  HRESULT hr;

  hr = D3DXSHProjectCubeMap( order, 
                             mCubeTexture, 
                             mSHCoeffsRed, 
                             mSHCoeffsGreen, 
                             mSHCoeffsBlue );

  return hr;
}

HRESULT CubeMap::DrawCubeMap(D3DXMATRIX* pmWorldViewProj) {
  HRESULT hr;
  
  D3DXMATRIX mInvWorldViewProj;
  D3DXMatrixInverse( &mInvWorldViewProj, NULL, pmWorldViewProj );
  
  hr = mEffect->SetTechnique( "CubeMap" );
  hr = mEffect->SetMatrix( "g_InvWVP", &mInvWorldViewProj ); 
  hr = mEffect->SetTexture( "g_Texture", mCubeTexture );  

  mDevice->SetStreamSource( 0, mVertexBuffer, 0, sizeof( CUBEMAP_VERTEX ) );
  mDevice->SetVertexDeclaration( mVertexDecl );
  
  UINT uiPass, uiNumPasses;
  hr = mEffect->Begin( &uiNumPasses, 0 );
  for( uiPass = 0; uiPass < uiNumPasses; uiPass++ )
  {
    hr = mEffect->BeginPass( uiPass );
    mDevice->DrawPrimitive( D3DPT_TRIANGLESTRIP, 0, 2 );
    hr = mEffect->EndPass();
  }
  mEffect->End();

  return D3D_OK;
}

HRESULT CubeMap::FillVertexBuffer() {
  HRESULT hr;
  
  if( mEffect ) mEffect->OnResetDevice();

  hr =  mDevice->CreateVertexBuffer( 4 * sizeof( CUBEMAP_VERTEX ),
                                     D3DUSAGE_WRITEONLY, 0, D3DPOOL_DEFAULT,
                                     &mVertexBuffer, NULL );

  PD(hr, L"create vertex buffer");
  if(FAILED(hr)) return hr;

  CUBEMAP_VERTEX* vertex = NULL;
  hr = mVertexBuffer->Lock( 0, 0, ( void** )&vertex, 0 );

  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  // Map texels to pixels 
  float width = 800; // (float)pBackBufferSurfaceDesc->Width;
  float height = 600; // (float)pBackBufferSurfaceDesc->Height;
  float fHighW = -1.0f - ( 1.0f / width );
  float fHighH = -1.0f - ( 1.0f / height );
  float fLowW = 1.0f + ( 1.0f / width );
  float fLowH = 1.0f + ( 1.0f / height );

  vertex[0].pos = D3DXVECTOR4( fLowW, fLowH, 1.0f, 1.0f );
  vertex[1].pos = D3DXVECTOR4( fLowW, fHighH, 1.0f, 1.0f );
  vertex[2].pos = D3DXVECTOR4( fHighW, fLowH, 1.0f, 1.0f );
  vertex[3].pos = D3DXVECTOR4( fHighW, fHighH, 1.0f, 1.0f );

  hr = mVertexBuffer->Unlock();

  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}
