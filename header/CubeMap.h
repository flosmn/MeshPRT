#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "d3dUtil.h"
#include "LightSource.h"
#include "Mesh.h"

class CubeMap : public LightSource
{
public:
  CubeMap(IDirect3DDevice9*);
  CubeMap();
  ~CubeMap();

  HRESULT LoadCubeMap(WCHAR* directory, WCHAR* name, WCHAR* extension);
  HRESULT DrawCubeMap(D3DXMATRIX* pmWorldViewProj);
  HRESULT CalculateSHCoefficients(DWORD order);
  HRESULT FillVertexBuffer();

	void GetSHCoeffsRed(Mesh* mesh, float* red);
	void GetSHCoeffsGreen(Mesh* mesh, float* green);
	void GetSHCoeffsBlue(Mesh* mesh, float* blue);

  IDirect3DCubeTexture9* GetTexture() { return mCubeTexture; }

  void SetDirectory(WCHAR* dir) { directory = dir; } 
  WCHAR* GetDirectory() { return directory; }

  void SetName(WCHAR* _name) { name = _name; } 
  WCHAR* GetName() { return name; }

protected:
	void GetTransformedCoeffs(Mesh* mesh, float* target, float* coeffs);

  IDirect3DDevice9* mDevice;
  IDirect3DCubeTexture9* mCubeTexture;
  ID3DXEffect* mEffect;
  IDirect3DVertexBuffer9* mVertexBuffer;
  IDirect3DVertexDeclaration9* mVertexDecl;

  WCHAR* directory;
  WCHAR* name;

	DWORD mOrder;
};

#endif // CUBEMAP_H
