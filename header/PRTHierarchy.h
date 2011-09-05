#ifndef PRTHIERARCHY_H
#define PRTHIERARCHY_H

#include "d3dUtil.h"
#include "Mesh.h"
#include "PRTEngine.h"
#include "PRTHierarchyMapping.h"
#include "LightSource.h"
#include "Timer.h"
#include "NNMapping.h"
#include "Camera.h"


class PRTHierarchy{
public:
  PRTHierarchy(IDirect3DDevice9 *device);
  ~PRTHierarchy();

  HRESULT LoadMeshHierarchy(WCHAR* renderMeshFile, WCHAR* approxMeshFile, 
                            WCHAR* directory, WCHAR* extension, DWORD order);

  HRESULT CalculateSHCoefficients();

  HRESULT CalculateMapping();

	HRESULT UpdateExactSHLighting(LightSource* lightSource);

	HRESULT InterpolateSHCoefficients();

  HRESULT TransferSHDataToGPU();

  HRESULT UpdateLighting(LightSource* lightSource);

  HRESULT ScaleMeshes();

  HRESULT CheckColor(LightSource* lightSource);

	void UpdateState(bool visualizeError, bool interpolate);

  bool HasTextures();
  void DrawMesh();
  void LoadEffect(ID3DXEffect* mEffect);
  int GetNumVertices();
  int GetNumFaces();

  Mesh* GetRenderMesh() { return mRenderMesh; }
  Mesh* GetApproxMesh() { return mApproxMesh; }

	void RotateX(float dw);
	void RotateY(float dw);
	void RotateZ(float dw);

	void Rotate(float dx, float dy, Camera* camera);

private:  
	void UpdateTransformationMatrices();
	float CheckAngleRange(float dw);

  IDirect3DDevice9 *mDevice;
  ID3DXEffect* mEffect;
  Mesh* mRenderMesh;
  Mesh* mApproxMesh;
  PRTEngine* mPRTEngine;
  PRTHierarchyMapping* mPRTHierarchyMapping;

  Timer* mTimer;
      
  DWORD mOrder;

	float mBoudingSphereRadius;
  
	float mRotationX;
	float mRotationY;
	float mRotationZ;
  D3DXMATRIX mWorldTransform;
	D3DXMATRIX mScaleMatrix;
};

#endif // PRTHIERARCHY_H