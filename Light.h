#ifndef LIGHT_H
#define LIGHT_H

#include "d3dUtil.h"
#include "LightSource.h"

class Light : public LightSource 
{
public:
  Light(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXCOLOR color);
  Light();
  virtual ~Light();

  D3DXVECTOR3 GetLightDirection() { return mLightDirection; }
  D3DXVECTOR3 GetLightPosition() { return mLightPosition; }
  D3DXCOLOR GetLightColor() { return mLightColor; }
  HRESULT CalculateSHCoefficients(DWORD order);
  
protected:
  D3DXVECTOR3 mLightDirection;
  D3DXVECTOR3 mLightPosition;
  D3DXCOLOR mLightColor;
};

#endif // LIGHT_H
