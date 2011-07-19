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
  ID3DXMesh* pMesh = mesh->GetMesh();
  DWORD dwNumMeshes = 0;
  pMesh->GetAttributeTable( NULL, &dwNumMeshes );
  
  D3DXSHMATERIAL* material = new D3DXSHMATERIAL[dwNumMeshes];
  D3DXSHMATERIAL** materials = new D3DXSHMATERIAL*[dwNumMeshes];
  InitMeshMaterial(mesh, dwNumMeshes, material, materials);

  ID3DXPRTEngine* engine = NULL;
  ID3DXPRTBuffer* pDataTotal = NULL;
  ID3DXPRTBuffer* pBufferA = NULL;
  ID3DXPRTBuffer* pBufferB = NULL;
  ID3DXPRTCompBuffer *compPRTBuffer;
      
  float mLengthScale = 25.0f;
  bool hasTextures = false; //mesh->HasTextures();
  
  DWORD* pdwAdj = new DWORD[pMesh->GetNumFaces() * 3];
  pMesh->GenerateAdjacency( 1e-6f, pdwAdj );

  PD(D3DXCreatePRTEngine(pMesh, pdwAdj, hasTextures, NULL, &engine), L"create engine");
  DWORD dwNumSamples = engine->GetNumVerts();

  if(hasTextures)
  {
    PD(engine->SetPerTexelAlbedo(mesh->GetTextures(), mNumChannels, NULL), L"set texture as per texel albedo");
  }

  PD(engine->SetSamplingInfo( mNumRays, FALSE, TRUE, FALSE, 0.0f ), L"set sampling info");
  PD(engine->SetMeshMaterials( (const D3DXSHMATERIAL **)materials, dwNumMeshes, mNumChannels, !hasTextures, mLengthScale), L"set mesh materials" );

  //check if compbuffer already exists
  WCHAR compbufferfile[120];
  WCHAR compbufferpathrel[120];
  WCHAR compbufferpath[120];
  WCHAR bufferfile[120];
  WCHAR bufferpathrel[120];
  Concat(compbufferfile, mesh->GetName(), L".compbuffer"); 
  Concat(compbufferpathrel, mesh->GetDirectory(), compbufferfile);
  AppendToRootDir(compbufferpath, compbufferpathrel);
  Concat(bufferfile, mesh->GetName(), L".buffer");
  Concat(bufferpathrel, mesh->GetDirectory(), bufferfile);
  WCHAR bufferpath[120];
  AppendToRootDir(bufferpath, bufferpathrel);

  hr = D3DXLoadPRTCompBufferFromFile(compbufferpath, &compPRTBuffer);

  if(FAILED(hr)) {
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder, mNumChannels, &pDataTotal ), L"create prt buffer");
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferA ), L"create prt buffer");
    PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferB ), L"create prt buffer");

    PD(engine->ComputeDirectLightingSH( mOrder, pDataTotal ), L"compute direct lighting SH");
    
    pBufferA->AddBuffer( pDataTotal );
  
    hr = engine->ComputeBounce( pBufferA, pBufferB, pDataTotal );
    PD(hr, L"compute first bounce");
    if(FAILED(hr)) return hr;

    ID3DXPRTBuffer* pPRTBufferTemp = NULL;
    pPRTBufferTemp = pBufferA;
    pBufferA = pBufferB;
    pBufferB = pPRTBufferTemp;
  
    hr = engine->ComputeBounce( pBufferA, pBufferB, pDataTotal );
    PD(hr, L"compute second bounce");
    if(FAILED(hr)) return hr;
   
    hr = D3DXCreatePRTCompBuffer( D3DXSHCQUAL_FASTLOWQUALITY, 1, mNumPCA, NULL,
                                NULL, pDataTotal, &compPRTBuffer );
    PD(hr, L"create compressed prt buffer");
    if(FAILED(hr)) return hr;
  
    hr = D3DXSavePRTCompBufferToFile( compbufferpath, compPRTBuffer );
    PD(hr, L"save compressed prt buffer to file");
    if(FAILED(hr)) return hr;

    hr = D3DXSavePRTBufferToFile( bufferpath, pDataTotal );
    PD(hr, L"save prt buffer to file");
    if(FAILED(hr)) return hr;
  }
  
  mesh->SetPRTCompBuffer(compPRTBuffer);

  PD( compPRTBuffer->NormalizeData() , L"normalize data of comp prt buffer");

  UINT dwNumCoeffs = compPRTBuffer->GetNumCoeffs();
  UINT dwNumClusters = compPRTBuffer->GetNumClusters();

  UINT* clusterIDs = new UINT[ dwNumSamples ];
  PD( compPRTBuffer->ExtractClusterIDs( clusterIDs ) , L"extract cluster id's of comp prt buffer");
  mesh->SetClusterIds(clusterIDs); 

  float* pcaWeights = new float[mesh->GetNumVertices() * mNumPCA];
  PD( compPRTBuffer->ExtractPCA(0, mNumPCA, pcaWeights), L"extract pca to array" );
  mesh->SetPcaWeights(pcaWeights);

  UINT numCoeffs = mesh->GetPRTCompBuffer()->GetNumCoeffs();
  UINT numClusters = mesh->GetPRTCompBuffer()->GetNumClusters();
  int nClusterBasisSize = ( mNumPCA + 1 ) * numCoeffs * mNumChannels;  // mean + pca-basis vectors of cluster
  int nBufferSize = nClusterBasisSize * numClusters;
  
  float* PRTClusterBases = new float[nBufferSize];
  for( DWORD cluster = 0; cluster < numClusters; cluster++ ) {
    hr = mesh->GetPRTCompBuffer()->ExtractBasis( cluster, &PRTClusterBases[cluster * nClusterBasisSize] );

    PD(hr, L"extract basis");
    if(FAILED(hr)) return hr;
  }

  mesh->SetPRTClusterBases(PRTClusterBases);

  ReleaseCOM( engine );
  ReleaseCOM( pBufferA );
  ReleaseCOM( pBufferB );
  ReleaseCOM( pDataTotal );
  
  delete[] pdwAdj;
  delete [] materials;
  delete [] material;
  
  return D3D_OK;
}

/*
After the call of this function the precomputed diffuse color will be stored
at the vertex data position blendweight1 (see FULL_VERTEX structure). 
Previously stored data on this position will be overwritten (PCAWeights). 
*/
HRESULT PRTEngine::CalculateDiffuseColor(Mesh* mesh, LightSource* light) {
  HRESULT hr;

  ID3DXPRTCompBuffer* compPRTBuffer = mesh->GetPRTCompBuffer();
  float* pcaWeights = mesh->GetPcaWeights();
  UINT* clusterIds = mesh->GetClusterIds();
       
  FULL_VERTEX* pVertexBuffer = NULL;
  hr = mesh->GetMesh()->LockVertexBuffer( 0, ( void** )&pVertexBuffer );
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  DWORD numCoeffs = compPRTBuffer->GetNumCoeffs();
  DWORD numChannels = compPRTBuffer->GetNumChannels();

  for( UINT i = 0; i < mesh->GetNumVertices(); i++ )
  {
    int clusterId = clusterIds[i];
    float* pPCAWeights = &(pcaWeights[i * mNumPCA]);
  
    D3DXCOLOR diffuseColor = GetPrecomputedDiffuseColor(clusterId, 
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

void PRTEngine::InitMeshMaterial(Mesh *mesh, DWORD numMeshes, D3DXSHMATERIAL* material, D3DXSHMATERIAL** materials) {
  D3DCOLORVALUE absorbtion, redScattering;  
  absorbtion.r = 0.0f; absorbtion.g = 0.0f; absorbtion.b = 0.0f; absorbtion.a = 0.0f;
  redScattering.r = 2.0f; redScattering.g = 2.0f; redScattering.b = 2.0f; redScattering.a = 1.0f;
  
  for( DWORD i = 0; i < numMeshes; ++i )
  {
    ZeroMemory( &material[i], sizeof( D3DXSHMATERIAL ) );
    material[i].bMirror = false;
    material[i].bSubSurf = false;
    material[i].RelativeIndexOfRefraction = 1.3f;
    material[i].Absorption = absorbtion;
    material[i].ReducedScattering = redScattering;
    material[i].Diffuse = mesh->GetDiffuseMaterial(i);
    
    materials[i] = &material[i];
  }  
}
