#ifndef D3DUTIL_H
#define D3DUTIL_H

#if defined(DEBUG) | defined(_DEBUG)
#ifndef D3D_DEBUG_INFO
#define D3D_DEBUG_INFO
#endif
#endif

#define UNICODE

#include <d3d9.h>
#include <d3dx9.h>

#include <string>
#include <stdio.h>
#include <sstream>
#include <vector>
#include <stdio.h>
#include <iostream>
#include <fstream>

#include "LocalConfig.h"

//===============================================================
// Globals for convenient access.
class D3DApp;
extern D3DApp* gd3dApp;
extern IDirect3DDevice9* gd3dDevice;

void Concat(WCHAR* result, const WCHAR *a, const WCHAR *b);
void AppendToRootDir(WCHAR* result, const WCHAR *b);

WCHAR* GetRootDir();

HRESULT LoadEffectFile( IDirect3DDevice9* device, WCHAR* file, 
                        const D3DXMACRO *defines, DWORD flags, 
                        LPD3DXEFFECT *effect );

//===============================================================
// Clean up

#define ReleaseCOM(x) { if(x){ x->Release();x = 0; } }

#ifndef SAFE_DELETE
#define SAFE_DELETE(p)       { if (p) { delete (p);     (p)=NULL; } }
#endif
#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p) { if (p) { delete[] (p);   (p)=NULL; } }
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if (p) { (p)->Release(); (p)=NULL; } }
#endif

// print debug
void PD(HRESULT, const WCHAR*);
void PD(HRESULT, const char*);
void PD(const WCHAR*);
void PD(const WCHAR*, const WCHAR*);
void PD(const WCHAR*, DWORD);
void PD(const WCHAR*, int);
void PD(const WCHAR*, float);
void PD(const WCHAR*, double);
void PD(HRESULT);
void CharArrayToWCharArray(const char* in, WCHAR* out);
void DbgOutInt(std::string label, int value );

#endif // D3DUTIL_H
