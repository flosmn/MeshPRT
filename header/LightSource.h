#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

#include "Mesh.h"

class LightSource {
public:
	virtual void GetSHCoeffsRed(Mesh* mesh, float* red) = 0;
	virtual void GetSHCoeffsGreen(Mesh* mesh, float* green) = 0;
	virtual void GetSHCoeffsBlue(Mesh* mesh, float* blue) = 0;

 virtual HRESULT CalculateSHCoefficients(DWORD order) = 0;

protected:
  float mSHCoeffsRed[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsGreen[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsBlue[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
};

#endif // LIGHTSOURCE_H
