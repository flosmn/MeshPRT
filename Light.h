#include "d3dUtil.h"

class Light
{
public:
  Light(D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXCOLOR color);
  Light();
  virtual ~Light();

  D3DXVECTOR3 getLightDirection() { return mLightDirection; }
  D3DXVECTOR3 getLightPosition() { return mLightPosition; }
  D3DXCOLOR getLightColor() { return mLightColor; }
  void calculateSHCoefficients();
  float* getSHCoeffsRed(){ return mSHCoeffsRed; }
  float* getSHCoeffsGreen(){ return mSHCoeffsGreen; }
  float* getSHCoeffsBlue() { return mSHCoeffsBlue; }

protected:
  D3DXVECTOR3 mLightDirection;
  D3DXVECTOR3 mLightPosition;
  D3DXCOLOR mLightColor;

  float mSHCoeffsRed[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsGreen[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsBlue[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
};