#include "PRTHierarchy.h"
#include <time.h>

PRTHierarchy::PRTHierarchy(IDirect3DDevice9 *device) {
  mDevice = device;

  mTimer = new Timer();

  mRenderMesh = 0;
  mApproxMesh = 0;
  mPRTEngine = 0;
  mPRTHierarchyMapping = 0;
  mOrder = 0;
  D3DXMatrixIdentity(&mWorldTransform);

  mPRTHierarchyMapping = new PRTHierarchyMapping();
}

PRTHierarchy::~PRTHierarchy() {
  delete mRenderMesh;
  delete mApproxMesh;
  delete mPRTEngine;
  delete mPRTHierarchyMapping;
  delete mTimer;
}

HRESULT PRTHierarchy::LoadMeshHierarchy(WCHAR* renderMeshFile, 
                                        WCHAR* approxMeshFile, 
                                        WCHAR* directory,
                                        WCHAR* extension,
                                        DWORD order)
{
  HRESULT hr;

  mOrder = order;

  mPRTEngine = new PRTEngine(mDevice, mOrder);
  mRenderMesh = new Mesh(mDevice);
  mApproxMesh = new Mesh(mDevice);
  
  hr = mRenderMesh->LoadMesh(directory, renderMeshFile, extension);
  PD(hr, L"load render mesh file");
  if(FAILED(hr)) return hr;

  hr = mApproxMesh->LoadMesh(directory, approxMeshFile, extension);
  PD(hr, L"load approx mesh file");
  if(FAILED(hr)) return hr;

  hr = FillVertexVectors();
  PD(hr, L"fill vertex vectors");
  if(FAILED(hr)) return hr;
   
  return D3D_OK;
}

HRESULT PRTHierarchy::ScaleMeshes() {
  HRESULT hr;

  ID3DXMesh* d3dMesh = mRenderMesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;
   
  D3DXVECTOR3 center;
  float radius;

  hr = D3DXComputeBoundingSphere((D3DXVECTOR3*)pVertices, 
                                 d3dMesh->GetNumVertices(),
                                 d3dMesh->GetNumBytesPerVertex(), 
                                 &center, &radius);

  PD(hr, L"compute bounding sphere");
  if(FAILED(hr)) return hr;

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  PD(L"radius: ", radius);

  float scale = 10.0f/radius;
  D3DXMatrixScaling(&mWorldTransform, scale, scale, scale); 

  return D3D_OK;
}

HRESULT PRTHierarchy::CalculateSHCoefficients() {
  HRESULT hr;

  mTimer->Start();
  hr = mPRTEngine->CalculateSHCoefficients(mApproxMesh);
  PD(hr, L"calculate coefficients for approx mesh");
  if(FAILED(hr)) return  hr;
  mTimer->Stop(L"calculate coefficients for approx mesh");
    
  mTimer->Start();
  hr = mPRTEngine->CalculateSHCoefficients(mRenderMesh);
  PD(hr, L"calculate coefficients for approx mesh");
  if(FAILED(hr)) return  hr;
  mTimer->Stop(L"calculate coefficients for render mesh");
    
  return D3D_OK;
}

HRESULT PRTHierarchy::CalculateNNMapping(DWORD numNN) {
	HRESULT hr;

	mRenderMesh->InitialiseMappingDatastructures(numNN);
	int* mappingIndices = mRenderMesh->GetMappingIndices();
	float* mappingWeights = mRenderMesh->GetMappingWeights();

	mPRTHierarchyMapping->NearestNeighbourMappingTree(numNN,
                                                     mApproxMeshVertices,
                                                     mRenderMeshVertices,
                                                     mappingIndices,
                                                     mappingWeights);

	return D3D_OK;
}

HRESULT PRTHierarchy::InterpolateSHCoefficients(DWORD numNN) {
	HRESULT hr;
	ID3DXPRTCompBuffer* prtCompBuffer = mRenderMesh->GetPRTCompBuffer();
	UINT numChannels = prtCompBuffer->GetNumChannels();
	UINT numCoeffs = prtCompBuffer->GetNumCoeffs();
	
	int* mappingIndices = mRenderMesh->GetMappingIndices();
	float* mappingWeights = mRenderMesh->GetMappingWeights();
		
	float* approxSHCoeff = mApproxMesh->GetSHCoefficients();
	float* exactSHCoeff = mRenderMesh->GetSHCoefficients();
	float* interpolSHCoeff = mRenderMesh->GetInterpolatedSHCoefficients();

	UINT numApproxMeshVertices = mApproxMesh->GetNumVertices();
	UINT numRenderMeshVertices = mRenderMesh->GetNumVertices();
	
	bool debug = false;
		
	for(int i = 0; i < numRenderMeshVertices; ++i) {
		UINT stride = numCoeffs * numChannels;
		for(int j = 0; j < numCoeffs; ++j) {
			float redCoeff = 0.0f;
			float greenCoeff = 0.0f;
			float blueCoeff = 0.0f;
			
			for(int k = 0; k < numNN; ++k) {
				UINT index = mappingIndices[i*numNN + k];
				if(index < 0 || index >= numApproxMeshVertices) {
					PD(L"error nn index out of bounds");
					return -1;
				}
				
				float redApprox   = approxSHCoeff[index*stride + 0*numCoeffs + j];
				float greenApprox = approxSHCoeff[index*stride + 1*numCoeffs + j];
				float blueApprox  = approxSHCoeff[index*stride + 2*numCoeffs + j];
				float weight = mappingWeights[i*numNN + k];
								
				redCoeff   += weight * redApprox;
				greenCoeff += weight * greenApprox;
				blueCoeff  += weight * blueApprox;

				if(debug) PD(L"redApprox: ", redApprox);
				if(debug) PD(L"greenApprox: ", greenApprox);
				if(debug) PD(L"blueApprox: ", blueApprox);
				if(debug) PD(L"weight: ", weight);
				if(debug) PD(L"partial redCoeff: ", redCoeff);
				if(debug) PD(L"partial greenCoeff: ", greenCoeff);
				if(debug) PD(L"partial blueCoeff: ", blueCoeff);		
			}
			
			if(debug) PD(L"final redCoeff: ", redCoeff);
			if(debug) PD(L"final redCoeff: ", greenCoeff);
			if(debug) PD(L"final redCoeff: ", blueCoeff);

			interpolSHCoeff[i*stride + 0*numCoeffs + j] = redCoeff;
			interpolSHCoeff[i*stride + 1*numCoeffs + j] = greenCoeff;
			interpolSHCoeff[i*stride + 2*numCoeffs + j] = blueCoeff;
		}
	}

	return D3D_OK;
}

HRESULT PRTHierarchy::TransferSHDataToGPU(DWORD numNN, bool interpolate) {
	HRESULT hr;

	mRenderMesh->FillVertexBufferWithSHCoefficients(numNN, interpolate);

	return D3D_OK;
}

HRESULT PRTHierarchy::CheckColor(LightSource* lightSource) {
	mPRTEngine->CheckCalculatedSHCoefficients(mRenderMesh, lightSource);

	return D3D_OK;
}

HRESULT PRTHierarchy::UpdateLighting(LightSource* lightSource) {
  HRESULT hr;

  UINT numCoeffs = mOrder * mOrder;

  lightSource->CalculateSHCoefficients(mOrder);

  mEffect->SetFloatArray("redSHCoeffsLight", lightSource->GetSHCoeffsRed(), numCoeffs);
  mEffect->SetFloatArray("greenSHCoeffsLight", lightSource->GetSHCoeffsGreen(), numCoeffs);
  mEffect->SetFloatArray("blueSHCoeffsLight", lightSource->GetSHCoeffsBlue(), numCoeffs);
  mEffect->CommitChanges();

  return D3D_OK;
}

HRESULT PRTHierarchy::FillVertexVectors() {
  HRESULT hr;

  hr = FillVertexVector(mRenderMeshVertices, mRenderMesh);
  PD(hr, L"fill render mesh vertex vector");
  if(FAILED(hr)) return hr;

  hr = FillVertexVector(mApproxMeshVertices, mApproxMesh);
  PD(hr, L"fill approx mesh vertex vector");
  if(FAILED(hr)) return hr;

  PD(L"size of render mesh vertex vector: ", (int)mRenderMeshVertices.size());
  PD(L"size of approx mesh vertex vector: ", (int)mApproxMeshVertices.size());

  return D3D_OK;
}

HRESULT PRTHierarchy::FillVertexVector(std::vector<Vertex> &vec, Mesh* mesh) {
  HRESULT hr;

  ID3DXMesh* d3dMesh = mesh->GetMesh();

  FULL_VERTEX *pVertices = NULL;
  hr = d3dMesh->LockVertexBuffer(0, (void**)&pVertices);
  PD(hr, L"lock vertex buffer");
  if(FAILED(hr)) return hr;

  for ( DWORD i = 0; i < d3dMesh->GetNumVertices(); ++i ) {
    Vertex vertex;
    vertex.pos.x = pVertices[i].position.x;
    vertex.pos.y = pVertices[i].position.y;
    vertex.pos.z = pVertices[i].position.z;
    vertex.normal.x = pVertices[i].normal.x;
    vertex.normal.y = pVertices[i].normal.y;
    vertex.normal.z = pVertices[i].normal.z;
    vec.push_back(vertex);
  }

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  return D3D_OK;
}

void PRTHierarchy::LoadEffect(ID3DXEffect* effect){
  mEffect = effect;
  mRenderMesh->LoadFX(mEffect);
  mApproxMesh->LoadFX(mEffect);
}

bool PRTHierarchy::HasTextures() {
  return mRenderMesh->HasTextures();
}

void PRTHierarchy::DrawMesh() {
  mRenderMesh->SetWorldTransform(&mWorldTransform);
  mRenderMesh->DrawMesh();
}

int PRTHierarchy::GetNumVertices() {
  return mRenderMesh->GetNumVertices();
}

int PRTHierarchy::GetNumFaces() {
  return mRenderMesh->GetNumFaces();
}

void PRTHierarchy::UpdateState(bool renderError, bool interpolate, DWORD numNN) {
	PD(L"update state");
	PD(L"interpolate: ", interpolate);
	PD(L"renderError: ", renderError);

	mRenderMesh->FillVertexBufferWithSHCoefficients(numNN, interpolate);
	
	mEffect->SetBool("renderError", renderError);
	mEffect->CommitChanges();
}