#include "Window.h"

HINSTANCE globalInstance;
HWND globalWindowHandle;

LPWSTR className = L"D3DWndClassName";

Window::Window() {
  globalInstance = 0;
  globalWindowHandle = 0;
  globalInstance = GetModuleHandle( NULL );

  RegClass();
}

Window::~Window() {

}

void Window::OpenWindow(ID3DXMesh* mesh) {
 CreateThread(NULL, 0, NewWindow, mesh, 0, NULL);
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE prevInstance,
                    PSTR cmdLine, int showCmd)
{
  Window* window = new Window();
  SetGlobalInstance(hInstance);
  InitMainWindow();
  StartDirectX(NULL);
}

DWORD WINAPI NewWindow(LPVOID lpParameter) {
  
  if(gd3dApp == 0) {
    InitMainWindow();
    StartDirectX((ID3DXMesh*) lpParameter);
  }
  else {
    // update
  }
  return 0;
}



HINSTANCE GlobalInstance() {
  return globalInstance;
}

void SetGlobalInstance(HINSTANCE hInst) {
  globalInstance = hInst;
}

HWND GlobalWindowHandle() {
  return globalWindowHandle;
}

void SetGlobalWindowHandle(HWND handle) {
  globalWindowHandle = handle;
}

void RegClass(){
  WNDCLASS wc;
  wc.style         = CS_HREDRAW | CS_VREDRAW | CS_GLOBALCLASS;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = GlobalInstance();
  wc.hIcon         = LoadIcon(0, IDI_APPLICATION);
  wc.hCursor       = LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = 0;
  wc.lpszClassName = className;

  WNDCLASS wndclass;
  if(GetClassInfo(GlobalInstance(), className, &wndclass) == 0) {
    OutputDebugString(L"no class info yet. try to register\n");
    if( !RegisterClass(&wc) )
    {
      MessageBox(0, L"RegisterClass FAILED", 0, 0);
      PostQuitMessage(0);
    }
    OutputDebugString(L"register class succeeded\n");
  }
  else {
    OutputDebugString(L"class info already exists. skip register\n");
  }
}

void InitMainWindow() {
  RECT R = {0, 0, 800, 600};
  AdjustWindowRect(&R, WS_OVERLAPPEDWINDOW, false);

  HWND handle = CreateWindow(className, L"Window", WS_OVERLAPPEDWINDOW,
                             100, 100, R.right, R.bottom, 0, 0,
                             GlobalInstance(), 0);

  SetGlobalWindowHandle(handle);

  if( !GlobalWindowHandle() )
  {
    MessageBox(0, L"CreateWindow FAILED", 0, 0);
    PostQuitMessage(0);
  }
  OutputDebugString(L"window create succeded");

  ShowWindow(GlobalWindowHandle(), SW_SHOW);
  UpdateWindow(GlobalWindowHandle());
}

LRESULT CALLBACK WndProc(HWND windowHandle, UINT msg, WPARAM wParam,
                         LPARAM lParam)
{
  if(gd3dApp != 0) {
    return gd3dApp->msgProc(msg, wParam, lParam);
  }
  else{
    switch( msg )
    {
      case WM_KEYDOWN:
        if( wParam == VK_ESCAPE )
          ::DestroyWindow(GlobalWindowHandle());
        return 0;

      case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProc(windowHandle, msg, wParam, lParam);
  }
}
