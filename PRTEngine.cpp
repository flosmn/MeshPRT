#include "PRTEngine.h"

PRTEngine::PRTEngine(IDirect3DDevice9* device, DWORD order) {
  mDevice = device;
  mOrder = order;

  mNumBounces = 0;
  mNumRays = 256;
  mNumChannels = 3;
  mNumPCA = mNumChannels * mOrder * mOrder;
  if(mNumPCA > 24) mNumPCA = 24;
}

PRTEngine::~PRTEngine() {
  
}

HRESULT PRTEngine::CalculateSHCoefficients(Mesh* mesh) {
  HRESULT hr;

  ID3DXPRTBuffer* pDataTotal = NULL;
  ID3DXPRTBuffer* pBufferA = NULL;
  ID3DXPRTBuffer* pBufferB = NULL;
  ID3DXPRTCompBuffer *compPRTBuffer;
  ID3DXMesh* pMesh = mesh->GetMesh();
      
  DWORD dwNumMeshes = 0;
  pMesh->GetAttributeTable( NULL, &dwNumMeshes );
  float mLengthScale = 25.0f;
  bool hasTextures = false; //mesh->HasTextures();
  
  DWORD* pdwAdj = new DWORD[pMesh->GetNumFaces() * 3];
  pMesh->GenerateAdjacency( 1e-6f, pdwAdj );

  PD(D3DXCreatePRTEngine(pMesh, pdwAdj, hasTextures, NULL, &mPRTEngine), L"create engine");
  DWORD dwNumSamples = mPRTEngine->GetNumVerts();

  if(hasTextures)
  {
    PD(mPRTEngine->SetPerTexelAlbedo(mesh->GetTextures(), mNumChannels, NULL), L"set texture as per texel albedo");
  }

  PD(mPRTEngine->SetSamplingInfo( mNumRays, FALSE, TRUE, FALSE, 0.0f ), L"set sampling info");
  PD(mPRTEngine->SetMeshMaterials( getMeshMaterial(mesh), dwNumMeshes, mNumChannels, !hasTextures, mLengthScale), L"set mesh materials" );

  //check if compbuffer already exists
  WCHAR* compbufferfile = Concat(mesh->GetName(), L".compbuffer");
  WCHAR* compbufferpath = Concat(mesh->GetDirectory(), compbufferfile);
  WCHAR* bufferfile = Concat(mesh->GetName(), L".buffer");
  WCHAR* bufferpath = Concat(mesh->GetDirectory(), bufferfile);


  hr = D3DXLoadPRTCompBufferFromFile(AppendToRootDir(compbufferpath), &compPRTBuffer);

  if(FAILED(hr)) {
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder, mNumChannels, &pDataTotal ), L"create prt buffer");
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferA ), L"create prt buffer");
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferB ), L"create prt buffer");

    PD(mPRTEngine->ComputeDirectLightingSH( mOrder, pDataTotal ), L"compute direct lighting SH");
    
    pBufferA->AddBuffer( pDataTotal );
  
    hr = mPRTEngine->ComputeBounce( pBufferA, pBufferB, pDataTotal );
    PD(hr, L"compute first bounce");
    if(FAILED(hr)) return hr;

    ID3DXPRTBuffer* pPRTBufferTemp = NULL;
    pPRTBufferTemp = pBufferA;
    pBufferA = pBufferB;
    pBufferB = pPRTBufferTemp;
  
    hr = mPRTEngine->ComputeBounce( pBufferA, pBufferB, pDataTotal );
    PD(hr, L"compute second bounce");
    if(FAILED(hr)) return hr;
   
    hr = D3DXCreatePRTCompBuffer( D3DXSHCQUAL_FASTLOWQUALITY, 1, mNumPCA, NULL,
                                NULL, pDataTotal, &compPRTBuffer );
    PD(hr, L"create compressed prt buffer");
    if(FAILED(hr)) return hr;
  
    hr = D3DXSavePRTCompBufferToFile( AppendToRootDir(compbufferpath), compPRTBuffer );
    PD(hr, L"save compressed prt buffer to file");
    if(FAILED(hr)) return hr;

    hr = D3DXSavePRTBufferToFile( AppendToRootDir(bufferpath), pDataTotal );
    PD(hr, L"save prt buffer to file");
    if(FAILED(hr)) return hr;
  }
  
  mesh->SetPRTCompBuffer(compPRTBuffer);

  PD( compPRTBuffer->NormalizeData() , L"normalize data of comp prt buffer");

  UINT dwNumCoeffs = compPRTBuffer->GetNumCoeffs();
  UINT dwNumClusters = compPRTBuffer->GetNumClusters();

  UINT* pClusterIDs = new UINT[ dwNumSamples ];
  
  PD( compPRTBuffer->ExtractClusterIDs( pClusterIDs ) , L"extract cluster id's of comp prt buffer");

  D3DVERTEXELEMENT9 declCur[MAX_FVF_DECL_SIZE];
  mesh->GetMesh()->GetDeclaration( declCur );

  BYTE* pV = NULL;
  PD( mesh->GetMesh()->LockVertexBuffer( 0, ( void** )&pV ), L"lock vertex buffer");
  UINT uStride = mesh->GetMesh()->GetNumBytesPerVertex();
  BYTE* pClusterID = pV + 32; // 32 == D3DDECLUSAGE_BLENDWEIGHT[0] offset
  for( UINT uVert = 0; uVert < dwNumSamples; uVert++ )
  {
    float fArrayOffset = ( float )( pClusterIDs[uVert] * ( 1 + 3 * ( mNumPCA / 4 ) ) );
    memcpy( pClusterID, &fArrayOffset, sizeof( float ) );
    pClusterID += uStride;
  }
  mesh->GetMesh()->UnlockVertexBuffer();
  delete pClusterIDs;

  PD( compPRTBuffer->ExtractToMesh( mNumPCA, D3DDECLUSAGE_BLENDWEIGHT, 1, mesh->GetMesh() ), L"extract to mesh" );
   
  UINT numCoeffs = mesh->GetPRTCompBuffer()->GetNumCoeffs();
  UINT numClusters = mesh->GetPRTCompBuffer()->GetNumClusters();
  int nClusterBasisSize = ( mNumPCA + 1 ) * numCoeffs * mNumChannels;  // mean + pca-basis vectors of cluster
  int nBufferSize = nClusterBasisSize * numClusters;
  
  float* PRTClusterBases = new float[nBufferSize];
  for( DWORD iCluster = 0; iCluster < numClusters; iCluster++ ) {
    hr = mesh->GetPRTCompBuffer()->ExtractBasis( iCluster, &PRTClusterBases[iCluster * nClusterBasisSize] );

    PD(hr, L"extract basis");
    if(FAILED(hr)) return hr;
  }

  mesh->SetPRTClusterBases(PRTClusterBases);

  ReleaseCOM( mPRTEngine );
  ReleaseCOM( pBufferA );
  ReleaseCOM( pBufferB );
  ReleaseCOM( pDataTotal );
  delete[] pdwAdj;

  return D3D_OK;
}

/*
After the call of this function the precomputed diffuse color will be stored
at the vertex data position blendweight1 (see FULL_VERTEX structure). 
Previously stored data on this position will be overwritten (PCAWeights). 
*/
HRESULT PRTEngine::CalculateDiffuseColor(Mesh* mesh, LightSource* light) {
  HRESULT hr;

  /* 
  we override the first blendweight color in the mesh with the diffuse
  color used for rendering. so when we calculate the diffuse color again we
  have to make sure that the PCAWeights are stored in the blendweight region
  of the mesh vertices again. therefore we extract the PCAWeights again.
  */
  ID3DXPRTCompBuffer* compPRTBuffer = mesh->GetPRTCompBuffer();
  hr = compPRTBuffer->ExtractToMesh( mNumPCA, D3DDECLUSAGE_BLENDWEIGHT, 
                                             1, mesh->GetMesh() );
  
  PD(hr, L"extract to mesh" );
  if(FAILED(hr)) return hr;
       
  FULL_VERTEX* pVertexBuffer = NULL;
  hr = mesh->GetMesh()->LockVertexBuffer( 0, ( void** )&pVertexBuffer );
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  DWORD numCoeffs = compPRTBuffer->GetNumCoeffs();
  DWORD numChannels = compPRTBuffer->GetNumChannels();

  for( UINT i = 0; i < mesh->GetNumVertices(); i++ )
  {
    int clusterID = pVertexBuffer[i].clusterID;
    float* pPCAWeights = pVertexBuffer[i].blendWeight1;
  
    D3DXCOLOR diffuseColor = GetPrecomputedDiffuseColor(clusterID, 
                                                        pPCAWeights,
                                                        mNumPCA,
                                                        numCoeffs,
                                                        numChannels,
                                                        mesh->GetPRTClusterBases(),
                                                        light->GetSHCoeffsRed(),
                                                        light->GetSHCoeffsGreen(),
                                                        light->GetSHCoeffsBlue());
    
    pVertexBuffer[i].blendWeight1 = diffuseColor;
  }
  hr = mesh->GetMesh()->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}

/*
calculates the diffuse color for a vertex according to the PCAWeights of that
vertex and the basis vectors of the cluster.
*/
D3DXCOLOR PRTEngine::GetPrecomputedDiffuseColor( int clusterID, 
                                                 float *vPCAWeights, 
                                                 DWORD numPCA,
                                                 int numCoeffs,
                                                 int numChannels,
                                                 float* prtClusterBases,
                                                 float* redLightCoeff,
                                                 float* greenLightCoeff,
                                                 float* blueLightCoeff)
{         
  DWORD dwClusterStride = numChannels * numPCA + 4;
  DWORD dwBasisStride = numCoeffs * numChannels * ( numPCA + 1 );
  
  float vAccumR = D3DXSHDot( mOrder, &prtClusterBases[clusterID * dwBasisStride + 0 * numCoeffs], redLightCoeff );
  float vAccumG = D3DXSHDot( mOrder, &prtClusterBases[clusterID * dwBasisStride + 1 * numCoeffs], greenLightCoeff);
  float vAccumB = D3DXSHDot( mOrder, &prtClusterBases[clusterID * dwBasisStride + 2 * numCoeffs], blueLightCoeff );
  
  for(int i = 0; i < mNumPCA; ++i) {
      int nOffset = clusterID * dwBasisStride + ( i + 1 ) * numCoeffs * mNumChannels;

      vAccumR += vPCAWeights[i] * D3DXSHDot( mOrder, &prtClusterBases[nOffset + 0 * numCoeffs], redLightCoeff );
      vAccumG += vPCAWeights[i] * D3DXSHDot( mOrder, &prtClusterBases[nOffset + 1 * numCoeffs], greenLightCoeff );
      vAccumB += vPCAWeights[i] * D3DXSHDot( mOrder, &prtClusterBases[nOffset + 2 * numCoeffs], blueLightCoeff );
  }
  
  D3DXCOLOR vDiffuse;
  vDiffuse.r = vAccumR;
  vDiffuse.g = vAccumG;
  vDiffuse.b = vAccumB;
  vDiffuse.a = 1.0f;
    
    
  return vDiffuse;
}

const D3DXSHMATERIAL** PRTEngine::getMeshMaterial(Mesh *mesh) {
  D3DCOLORVALUE absorbtion, redScattering;  
  absorbtion.r = 0.0f; absorbtion.g = 0.0f; absorbtion.b = 0.0f; absorbtion.a = 0.0f;
  redScattering.r = 2.0f; redScattering.g = 2.0f; redScattering.b = 2.0f; redScattering.a = 1.0f;
  
  DWORD dwNumMeshes = 0;
  mesh->GetMesh()->GetAttributeTable( NULL, &dwNumMeshes );

  D3DXSHMATERIAL* pMatPtr = new D3DXSHMATERIAL[dwNumMeshes];
  D3DXSHMATERIAL** pMatPtrArray = new D3DXSHMATERIAL*[dwNumMeshes]; 
  for( DWORD i = 0; i < dwNumMeshes; ++i )
  {
    ZeroMemory( &pMatPtr[i], sizeof( D3DXSHMATERIAL ) );
    pMatPtr[i].bMirror = false;
    pMatPtr[i].bSubSurf = false;
    pMatPtr[i].RelativeIndexOfRefraction = 1.3f;
    pMatPtr[i].Absorption = absorbtion;
    pMatPtr[i].ReducedScattering = redScattering;
    pMatPtr[i].Diffuse = mesh->GetDiffuseMaterial(i);
    
    pMatPtrArray[i] = &pMatPtr[i];
  }

  return (const D3DXSHMATERIAL**)pMatPtrArray;
}
