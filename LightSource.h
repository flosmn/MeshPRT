

class LightSource {
public:
 float* GetSHCoeffsRed(){ return mSHCoeffsRed; }
 float* GetSHCoeffsGreen(){ return mSHCoeffsGreen; }
 float* GetSHCoeffsBlue() { return mSHCoeffsBlue; }

protected:
  float mSHCoeffsRed[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsGreen[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
  float mSHCoeffsBlue[D3DXSH_MAXORDER*D3DXSH_MAXORDER];
};