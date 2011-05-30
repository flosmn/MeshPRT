#include "d3dUtil.h"
#include "DirectInput.h"

void PD(HRESULT result, const WCHAR* str)  {
  if(result == D3D_OK ) OutputDebugString(L"D3D_OK");
  else if(result == S_OK) OutputDebugString(L"S_OK");
  else if(result == D3DERR_INVALIDCALL) OutputDebugString(L"D3DERR_INVALIDCALL");
  else if(result == D3DXERR_INVALIDDATA) OutputDebugString(L"D3DXERR_INVALIDDATA");
  else if(result == E_OUTOFMEMORY) OutputDebugString(L"E_OUTOFMEMORY");
  else if(result == E_NOTIMPL) OutputDebugString(L"E_NOTIMPL");
  else if(result == DI_OK) OutputDebugString(L"DI_OK");
  else if(result == DIERR_BETADIRECTINPUTVERSION) OutputDebugString(L"DIERR_BETADIRECTINPUTVERSION");
  else if(result == DIERR_INVALIDPARAM) OutputDebugString(L"DIERR_INVALIDPARAM");
  else if(result == DIERR_OLDDIRECTINPUTVERSION) OutputDebugString(L"DIERR_OLDDIRECTINPUTVERSION");
  else if(result == DIERR_OUTOFMEMORY) OutputDebugString(L"DIERR_OUTOFMEMORY");
  else if(result == DIERR_NOTINITIALIZED) OutputDebugString(L"DIERR_NOTINITIALIZED");
  else if(result == DIERR_OTHERAPPHASPRIO) OutputDebugString(L"DIERR_OTHERAPPHASPRIO");
  else OutputDebugString(L"unknown HRESULT");
  OutputDebugString(L"   (");
  OutputDebugString(str);
  OutputDebugString(L")\n");
}

void PD(HRESULT result, const char* str) {
  WCHAR* wchar = 0;
  CharArrayToWCharArray(str, wchar);
  PD(result, wchar);
  delete [] wchar;
}

void CharArrayToWCharArray(const char* in, WCHAR* out) {
  DWORD dwNum = MultiByteToWideChar (CP_ACP, 0, in, -1, NULL, 0);
  WCHAR *w_error = 0;
  out = new WCHAR[dwNum];
  if(!w_error)
  {
    delete [] w_error;
  }
  MultiByteToWideChar (CP_ACP, 0, in, -1, out, dwNum );
}

WCHAR* GetRootDir() {
#ifdef MESHLAB
  return L"../meshlabplugins/filter_meshprt/";
#else
  return L"../";
#endif
}

WCHAR* Concat(const WCHAR *a, const WCHAR *b){
  WCHAR* out = new WCHAR[120];
  wcscpy(out, a);
  wcscat(out, b);
  return out;
}

WCHAR* AppendToRootDir(const WCHAR *b){
  WCHAR* a = GetRootDir();
  return Concat(a, b);
}

void DbgOutInt(std::string label, int value ) {
 std::stringstream strs;
 strs << value;
 label.append(strs.str()) ;
 const char *c_str = label.c_str();
 wchar_t *w_str = 0;
 CharArrayToWCharArray(c_str, w_str);
 OutputDebugString( w_str ) ;
 //delete [] w_str;
}
