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

HRESULT Light::CalculateSHCoefficients() {
  HRESULT hr;
  DWORD order = 5;
  
  D3DXVECTOR3 lightDirection = D3DXVECTOR3( -mLightDirection.x, 
                                            -mLightDirection.y,
                                            -mLightDirection.z);
  
  hr = D3DXSHEvalDirectionalLight( order, &lightDirection, 
                                   mLightColor.r, mLightColor.g, mLightColor.b,
                                   mSHCoeffsRed, mSHCoeffsGreen, mSHCoeffsBlue );
  
  PD( hr, L"eval directional light" );
  return hr;
}

