#include "Light.h"

Light::Light( D3DXVECTOR3 position, D3DXVECTOR3 direction, D3DXCOLOR color) {
  mLightPosition = position;
  mLightDirection = direction;
  mLightColor = color;
}

Light::Light() {
  
}

Light::~Light() {
}

HRESULT Light::CalculateSHCoefficients(DWORD order) {
  HRESULT hr;
    
  D3DXVECTOR3 lightDirection = D3DXVECTOR3( -mLightDirection.x, 
                                            -mLightDirection.y,
                                            -mLightDirection.z);
  
  hr = D3DXSHEvalConeLight( order, &lightDirection, D3DX_PI/4,
                                   mLightColor.r,
                                   mLightColor.g,
                                   mLightColor.b,
                                   mSHCoeffsRed, mSHCoeffsGreen, mSHCoeffsBlue);
  
  return hr;
}

