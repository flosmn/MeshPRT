#include "d3dUtil.h"

class Light
{
public:
  Light(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXCOLOR color);
  Light();
  virtual ~Light();

  D3DXVECTOR3 GetLightDirection() { return mLightDirection; }
  D3DXVECTOR3 GetLightPosition() { return mLightPosition; }
  D3DXCOLOR GetLightColor() { return mLightColor; }
  HRESULT CalculateSHCoefficients();
  float* GetSHCoeffsRed(){ return mSHCoeffsRed; }
  float* GetSHCoeffsGreen(){ return mSHCoeffsGreen; }
  float* GetSHCoeffsBlue() { return mSHCoeffsBlue; }

protected:
  D3DXVECTOR3 mLightDirection;
  D3DXVECTOR3 mLightPosition;
  D3DXCOLOR mLightColor;

  float mSHCoeffsRed[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsGreen[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsBlue[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
};