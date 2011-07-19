#ifndef WINDOW_H
#define WINDOW_H

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <Windows.h>

#include "d3dApp.h"
//#include "Mesh.h"

DWORD WINAPI NewWindow(LPVOID lpParameter);

HINSTANCE GlobalInstance();
HWND GlobalWindowHandle();
void SetGlobalInstance(HINSTANCE hInst);
void SetGlobalWindowHandle(HWND handle);
void RegClass();
void InitMainWindow();

HRESULT CALLBACK WndProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);
int Run();

class Window
{
public:
  Window();
  ~Window();
  void OpenWindow(ID3DXMesh*);
};
  
#endif // WINDOW_H
