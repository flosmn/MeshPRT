#include "PRTEngine.h"

PRTEngine::PRTEngine(IDirect3DDevice9* device) {
  mDevice = device;
  mOrder = 5;
}

PRTEngine::~PRTEngine() {
  
}

void PRTEngine::getCoefficientsForMesh(Mesh* mesh, Light* light) {
  ID3DXPRTBuffer* pDataTotal = NULL;
  ID3DXPRTBuffer* pBufferA = NULL;
  ID3DXPRTBuffer* pBufferB = NULL;
  ID3DXPRTCompBuffer *compPRTBuffer;
  ID3DXMesh* pMesh = mesh->getMesh();

  DWORD mNumBounces = 1;
  DWORD mNumRays = 1000;
  DWORD mNumChannels = 3;
  DWORD dwNumPCA = mNumChannels * mOrder * mOrder;
  if(dwNumPCA > 24) dwNumPCA = 24;
  DWORD dwNumMeshes = 0;
  pMesh->GetAttributeTable( NULL, &dwNumMeshes );
  float mLengthScale = 25.0f;

  DWORD* pdwAdj = new DWORD[pMesh->GetNumFaces() * 3];
  pMesh->GenerateAdjacency( 1e-6f, pdwAdj );

  PD(D3DXCreatePRTEngine(pMesh, pdwAdj, false, NULL, &mPRTEngine), L"create engine");
  DWORD dwNumSamples = mPRTEngine->GetNumVerts();

  PD(mPRTEngine->SetSamplingInfo( mNumRays, FALSE, TRUE, FALSE, 0.0f ), L"set sampling info");
  PD(mPRTEngine->SetMeshMaterials( getMeshMaterial(mesh), dwNumMeshes, mNumChannels, true, mLengthScale), L"set mesh materials" );

  PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder, mNumChannels, &pDataTotal ), L"create prt buffer");
  PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferA ), L"create prt buffer");
  PD(D3DXCreatePRTBuffer( dwNumSamples, mOrder * mOrder,  mNumChannels, &pBufferB ), L"create prt buffer");

  PD(mPRTEngine->ComputeDirectLightingSH( mOrder, pDataTotal ), L"compute direct lighting SH");

  pBufferA->AddBuffer( pDataTotal );
  PD(mPRTEngine->ComputeBounce( pBufferA, pBufferB, pDataTotal ), L"first boundce");
  PD(mPRTEngine->ComputeBounce( pBufferB, pBufferA, pDataTotal ), L"second bounce");

  PD(D3DXCreatePRTCompBuffer( D3DXSHCQUAL_SLOWHIGHQUALITY, 1, dwNumPCA, NULL, NULL, pDataTotal, &compPRTBuffer ), L"create compressed prt buffer");
  PD(D3DXSavePRTCompBufferToFile( L"compbuffer.buffer", compPRTBuffer ), L"save compressed prt buffer to file");
  PD(D3DXSavePRTBufferToFile( L"buffer.buffer", pDataTotal ), L"save prt buffer to file");

  mesh->setPRTCompBuffer(compPRTBuffer);

  PD( compPRTBuffer->NormalizeData() , L"normalize data of comp prt buffer");

  UINT dwNumCoeffs = compPRTBuffer->GetNumCoeffs();
  UINT dwNumClusters = compPRTBuffer->GetNumClusters();

  UINT* pClusterIDs = new UINT[ dwNumSamples ];
  if( pClusterIDs == NULL )
    return;
  PD( compPRTBuffer->ExtractClusterIDs( pClusterIDs ) , L"extract cluster ids of comp prt buffer");

  D3DVERTEXELEMENT9 declCur[MAX_FVF_DECL_SIZE];
  mesh->getMesh()->GetDeclaration( declCur );

  BYTE* pV = NULL;
  PD( mesh->getMesh()->LockVertexBuffer( 0, ( void** )&pV ), L"lock vertex buffer");
  UINT uStride = mesh->getMesh()->GetNumBytesPerVertex();
  BYTE* pClusterID = pV + 32; // 32 == D3DDECLUSAGE_BLENDWEIGHT[0] offset
  for( UINT uVert = 0; uVert < dwNumSamples; uVert++ )
  {
    float fArrayOffset = ( float )( pClusterIDs[uVert] * ( 1 + 3 * ( dwNumPCA / 4 ) ) );
    memcpy( pClusterID, &fArrayOffset, sizeof( float ) );
    pClusterID += uStride;
  }
  mesh->getMesh()->UnlockVertexBuffer();
  delete pClusterIDs;

  PD( compPRTBuffer->ExtractToMesh( dwNumPCA, D3DDECLUSAGE_BLENDWEIGHT, 1, mesh->getMesh() ), L"extract to mesh" );

  int nClusterBasisSize = ( dwNumPCA + 1 ) * dwNumCoeffs * mNumChannels;
  int nBufferSize = nClusterBasisSize * dwNumClusters;

  float* PRTClusterBases = new float[nBufferSize];
  float* PRTConstants = new float[dwNumClusters * ( 4 + mNumChannels * dwNumPCA )];

  for( DWORD iCluster = 0; iCluster < dwNumClusters; iCluster++ ) {
    PD( compPRTBuffer->ExtractBasis( iCluster, &PRTClusterBases[iCluster * nClusterBasisSize] ), L"extract basis" );
  }
  
  DWORD dwClusterStride = mNumChannels * dwNumPCA + 4;
  DWORD dwBasisStride = dwNumCoeffs * mNumChannels * ( dwNumPCA + 1 );
  
  for( DWORD iCluster = 0; iCluster < dwNumClusters; iCluster++ )  {
    PRTConstants[iCluster * dwClusterStride + 0] = D3DXSHDot( mOrder, &PRTClusterBases[iCluster * dwBasisStride + 0 * dwNumCoeffs], light->getSHCoeffsRed() );
    PRTConstants[iCluster * dwClusterStride + 1] = D3DXSHDot( mOrder, &PRTClusterBases[iCluster * dwBasisStride + 1 * dwNumCoeffs], light->getSHCoeffsGreen() );
    PRTConstants[iCluster * dwClusterStride + 2] = D3DXSHDot( mOrder, &PRTClusterBases[iCluster * dwBasisStride + 2 * dwNumCoeffs], light->getSHCoeffsBlue() );
    PRTConstants[iCluster * dwClusterStride + 3] = 0.0f;

    float* pPCAStart = &PRTConstants[iCluster * dwClusterStride + 4];
    for( DWORD iPCA = 0; iPCA < dwNumPCA; iPCA++ ) {
      int nOffset = iCluster * dwBasisStride + ( iPCA + 1 ) * dwNumCoeffs * mNumChannels;

      pPCAStart[0 * dwNumPCA + iPCA] = D3DXSHDot( mOrder, &PRTClusterBases[nOffset + 0 * dwNumCoeffs], light->getSHCoeffsRed() );
      pPCAStart[1 * dwNumPCA + iPCA] = D3DXSHDot( mOrder, &PRTClusterBases[nOffset + 1 * dwNumCoeffs], light->getSHCoeffsGreen() );
      pPCAStart[2 * dwNumPCA + iPCA] = D3DXSHDot( mOrder, &PRTClusterBases[nOffset + 2 * dwNumCoeffs], light->getSHCoeffsBlue() );
    }
  }

  mesh->setPRTConstants(PRTConstants);

  ReleaseCOM( mPRTEngine );
  ReleaseCOM( pBufferA );
  ReleaseCOM( pBufferB );
  ReleaseCOM( pDataTotal );
  delete[] pdwAdj;
}

const D3DXSHMATERIAL** PRTEngine::getMeshMaterial(Mesh *mesh) {
  D3DCOLORVALUE absorbtion, redScattering;  
  absorbtion.r = 0.0f; absorbtion.g = 0.0f; absorbtion.b = 0.0f; absorbtion.a = 0.0f;
  redScattering.r = 2.0f; redScattering.g = 2.0f; redScattering.b = 2.0f; redScattering.a = 1.0f;
  
  DWORD dwNumMeshes = 0;
  mesh->getMesh()->GetAttributeTable( NULL, &dwNumMeshes );

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
    pMatPtr[i].Diffuse = mesh->getDiffuseMaterial(i);
    
    pMatPtrArray[i] = &pMatPtr[i];
  }

  return (const D3DXSHMATERIAL**)pMatPtrArray;
}
