#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <Windows.h>

DWORD WINAPI NewWindow(LPVOID lpParameter);

HINSTANCE GlobalInstance();
HWND GlobalWindowHandle();
void SetGlobalInstance(HINSTANCE hInst);
void SetGlobalWindowHandle(HWND handle);


void RegClass();
void InitMainWindow();

LRESULT CALLBACK WndProc(HWND windowHandle, UINT msg, WPARAM wParam, LPARAM lParam);
int Run();

class Window
{
public:
  Window();
  ~Window();
  void OpenWindow();
};
  
