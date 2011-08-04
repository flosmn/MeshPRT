#ifndef PRTHIERARCHY_H
#define PRTHIERARCHY_H

#include "d3dUtil.h"
#include "Mesh.h"
#include "PRTEngine.h"
#include "PRTHierarchyMapping.h"
#include "LightSource.h"
#include "Timer.h"
#include "NNMapping.h"


class PRTHierarchy{
public:
  PRTHierarchy(IDirect3DDevice9 *device);
  ~PRTHierarchy();

  HRESULT LoadMeshHierarchy(WCHAR* renderMeshFile, WCHAR* approxMeshFile, 
                            WCHAR* directory, WCHAR* extension, DWORD order);

  HRESULT CalculateSHCoefficients();

  HRESULT CalculateNNMapping(DWORD numberOfNearestNeightbours);

	HRESULT InterpolateSHCoefficients(DWORD numberOfNearestNeightbours);

  HRESULT TransferSHDataToGPU(DWORD numberOfNearestNeightbours, bool interpolate);

  HRESULT UpdateLighting(LightSource* lightSource);

  HRESULT ScaleMeshes();

  HRESULT CheckColor(LightSource* lightSource);

	void UpdateState(bool visualizeError, bool interpolate, DWORD numNN);

  bool HasTextures();
  void DrawMesh();
  void LoadEffect(ID3DXEffect* mEffect);
  int GetNumVertices();
  int GetNumFaces();

  Mesh* GetRenderMesh() { return mRenderMesh; }
  Mesh* GetApproxMesh() { return mApproxMesh; }

private:
  HRESULT FillVertexVectors();  
  HRESULT FillVertexVector(std::vector<Vertex> &vec, Mesh* mesh);
    
  IDirect3DDevice9 *mDevice;
  ID3DXEffect* mEffect;
  Mesh* mRenderMesh;
  Mesh* mApproxMesh;
  PRTEngine* mPRTEngine;
  PRTHierarchyMapping* mPRTHierarchyMapping;

  Timer* mTimer;
  
  std::vector<Vertex> mRenderMeshVertices;
  std::vector<Vertex> mApproxMeshVertices;
  std::vector<D3DXVECTOR3> mRenderMeshVertexNormals;
  std::vector<D3DXVECTOR3> mApproxMeshVertexNormals;
    
  DWORD mOrder;
  
  D3DXMATRIX mWorldTransform;
};

#endif // PRTHIERARCHY_H