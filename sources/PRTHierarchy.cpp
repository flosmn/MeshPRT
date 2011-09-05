#include "PRTHierarchy.h"
#include <time.h>

PRTHierarchy::PRTHierarchy(IDirect3DDevice9 *device) {
  mDevice = device;

  mTimer = new Timer();

	mRotationX = 0.0f;
	mRotationY = 0.0f;
	mRotationZ = 0.0f;
	mBoudingSphereRadius = 10.0f;

	D3DXMatrixIdentity(&mWorldTransform);

  mRenderMesh = 0;
  mApproxMesh = 0;
  mPRTEngine = 0;
  mPRTHierarchyMapping = 0;
  mOrder = 0;
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

  hr = D3DXComputeBoundingSphere((D3DXVECTOR3*)pVertices, 
                                 d3dMesh->GetNumVertices(),
                                 d3dMesh->GetNumBytesPerVertex(), 
                                 &center, &mBoudingSphereRadius);

  PD(hr, L"compute bounding sphere");
  if(FAILED(hr)) return hr;

  hr = d3dMesh->UnlockVertexBuffer();
  PD(hr, L"unlock vertex buffer");
  if(FAILED(hr)) return hr;

  PD(L"radius: ", mBoudingSphereRadius);

  float scale = 10.0f/mBoudingSphereRadius;
  D3DXMatrixScaling(&mScaleMatrix, scale, scale, scale);
	UpdateTransformationMatrices();

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

HRESULT PRTHierarchy::CalculateMapping() {
	HRESULT hr;

	mRenderMesh->InitialiseMappingDatastructures();
	int* mappingIndices = mRenderMesh->GetMappingIndices();
	float* mappingWeights = mRenderMesh->GetMappingWeights();

	mPRTHierarchyMapping->GetMapping(
			mRenderMesh,	mApproxMesh, mappingIndices, mappingWeights);

	return D3D_OK;
}

HRESULT PRTHierarchy::InterpolateSHCoefficients() {
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
		if(i==12207) debug=true;
		else debug = false;
		
		UINT stride = numCoeffs * numChannels;
		for(int j = 0; j < numCoeffs; ++j) {
			float redCoeff = 0.0f;
			float greenCoeff = 0.0f;
			float blueCoeff = 0.0f;
			
			for(int k = 0; k < 3; ++k) {
				float weight = mappingWeights[i*3 + k];
				UINT index = mappingIndices[i*3 + k];
				if(index < 0 || index >= numApproxMeshVertices) {
					PD(L"error nn index out of bounds");
					return -1;
				}
				
				float redApprox   = approxSHCoeff[index*stride + 0*numCoeffs + j];
				float greenApprox = approxSHCoeff[index*stride + 1*numCoeffs + j];
				float blueApprox  = approxSHCoeff[index*stride + 2*numCoeffs + j];
				
								
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
			if(debug) PD(L"final greenCoeff: ", greenCoeff);
			if(debug) PD(L"final blueCoeff: ", blueCoeff);

			interpolSHCoeff[i*stride + 0*numCoeffs + j] = redCoeff;
			interpolSHCoeff[i*stride + 1*numCoeffs + j] = greenCoeff;
			interpolSHCoeff[i*stride + 2*numCoeffs + j] = blueCoeff;
		}
	}

	return D3D_OK;
}

HRESULT PRTHierarchy::UpdateExactSHLighting(LightSource* lightSource) {
	mPRTEngine->UpdateExactSHLighting(mRenderMesh, lightSource);
	return D3D_OK;
}

HRESULT PRTHierarchy::TransferSHDataToGPU() {
	HRESULT hr;

	mRenderMesh->FillVertexBufferWithSHCoefficients();

	return D3D_OK;
}

HRESULT PRTHierarchy::CheckColor(LightSource* lightSource) {
	mPRTEngine->CheckCalculatedSHCoefficients(mApproxMesh, lightSource);
	mPRTEngine->CheckCalculatedSHCoefficients(mRenderMesh, lightSource);

	return D3D_OK;
}

HRESULT PRTHierarchy::UpdateLighting(LightSource* lightSource) {
  HRESULT hr;

  UINT numCoeffs = mOrder * mOrder;

  lightSource->CalculateSHCoefficients(mOrder);
 
	float red[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
	float green[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
	float blue[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
	lightSource->GetSHCoeffsRed(mRenderMesh, red);
	lightSource->GetSHCoeffsGreen(mRenderMesh, green);
	lightSource->GetSHCoeffsBlue(mRenderMesh, blue);
	mEffect->SetFloatArray("redSHCoeffsLight", red, numCoeffs);
  mEffect->SetFloatArray("greenSHCoeffsLight", green, numCoeffs);
  mEffect->SetFloatArray("blueSHCoeffsLight", blue, numCoeffs);
  mEffect->CommitChanges();

	/*
	float* red = lightSource->GetSHCoeffsRed(mRenderMesh);
	float* green = lightSource->GetSHCoeffsGreen(mRenderMesh);
	float* blue = lightSource->GetSHCoeffsBlue(mRenderMesh);
	mEffect->SetFloatArray("redSHCoeffsLight", red, numCoeffs);
  mEffect->SetFloatArray("greenSHCoeffsLight", green, numCoeffs);
  mEffect->SetFloatArray("blueSHCoeffsLight", blue, numCoeffs);
	*/
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
  mRenderMesh->SetWorldTransformation(mWorldTransform);
  mRenderMesh->DrawMesh();
}

int PRTHierarchy::GetNumVertices() {
  return mRenderMesh->GetNumVertices();
}

int PRTHierarchy::GetNumFaces() {
  return mRenderMesh->GetNumFaces();
}

void PRTHierarchy::UpdateState(bool renderError, bool interpolate) {
	PD(L"update state");
	PD(L"interpolate: ", interpolate);
	PD(L"renderError: ", renderError);

	mRenderMesh->FillVertexBufferWithSHCoefficients();
	
	mEffect->SetBool("renderError", renderError);
	mEffect->SetBool("renderExact", !interpolate);
	mEffect->CommitChanges();
}

void PRTHierarchy::RotateX(float dw) {
	mRotationX += dw;
	mRotationX = CheckAngleRange(mRotationX);
	UpdateTransformationMatrices();
}

void PRTHierarchy::RotateY(float dw) {
	mRotationY += dw;
	mRotationY = CheckAngleRange(mRotationY);
	UpdateTransformationMatrices();
}

void PRTHierarchy::RotateZ(float dw) {
	mRotationZ += dw;
	mRotationZ = CheckAngleRange(mRotationZ);
	UpdateTransformationMatrices();
}

void PRTHierarchy::Rotate(float dx, float dy, Camera* camera) {
	D3DXVECTOR3 cameraPos = camera->pos();
	D3DXVECTOR3 zAxis = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
	
	D3DXMATRIX rotY;
	D3DXMatrixRotationY(&rotY, mRotationY);
	
	D3DXVECTOR4 tempAxis;
	D3DXVec3Transform(&tempAxis, &zAxis, &rotY);
	D3DXVECTOR3 rotatedZAxis = D3DXVECTOR3(tempAxis.x, tempAxis.y, tempAxis.z);

	mRotationY = mRotationY +  dy;
	
	int sign = D3DXVec3Dot(&cameraPos, &rotatedZAxis) >= 0 ? 1 : -1;
	mRotationX = mRotationX + sign * dx;

	UpdateTransformationMatrices();
}

float PRTHierarchy::CheckAngleRange(float dw) {
	if(dw >= 2.0f * D3DX_PI) {
		dw -= 2.0f * D3DX_PI;
	}
	if(dw <= 2.0f * D3DX_PI) {
		dw += 2.0f * D3DX_PI;
	}
	return dw;
}

void PRTHierarchy::UpdateTransformationMatrices() {
	D3DXMATRIX rotX, rotY, rotYInverse;
	D3DXMatrixRotationY(&rotY, mRotationY);
	D3DXMatrixRotationX(&rotX, mRotationX);
	
	D3DXMatrixIdentity(&mWorldTransform);

	D3DXMATRIX rot = rotX * rotY;
	mWorldTransform = mScaleMatrix * rot;
	mRenderMesh->SetWorldTransformation(mWorldTransform);
	mApproxMesh->SetWorldTransformation(mWorldTransform);
	mRenderMesh->SetRotationMatrix(rot);
	mApproxMesh->SetRotationMatrix(rot);
}