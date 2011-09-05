#ifndef LIGHT_H
#define LIGHT_H

#include "d3dUtil.h"
#include "LightSource.h"
#include "Mesh.h"

class Light : public LightSource 
{
public:
  Light(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXCOLOR color);
  Light();
  virtual ~Light();

  HRESULT CalculateSHCoefficients(DWORD order);
	
	void GetSHCoeffsRed(Mesh* mesh, float* red) { red = mSHCoeffsRed; }
	void GetSHCoeffsGreen(Mesh* mesh, float* green) { green = mSHCoeffsGreen; }
	void GetSHCoeffsBlue(Mesh* mesh, float* blue) { blue = mSHCoeffsBlue; }

  D3DXVECTOR3 GetLightDirection() { return mLightDirection; }
  D3DXVECTOR3 GetLightPosition() { return mLightPosition; }
  D3DXCOLOR GetLightColor() { return mLightColor; }
   
protected:
  D3DXVECTOR3 mLightDirection;
  D3DXVECTOR3 mLightPosition;
  D3DXCOLOR mLightColor;
};

#endif // LIGHT_H
