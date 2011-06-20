#include "Window.h"
#include "ObjFileParser.h"
#include "XFileCreator.h"

#include "K3Tree.h"

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

void CreateCube(std::vector<Vertex> &vertices) {
  Vertex vertex[8];
  
  vertex[0].pos.x = -1.0f; vertex[0].pos.y = -1.0f; vertex[0].pos.z = -1.0f;  
  vertex[1].pos.x =  1.0f; vertex[1].pos.y = -1.0f; vertex[1].pos.z = -1.0f;  
  vertex[2].pos.x =  1.0f; vertex[2].pos.y = -1.0f; vertex[2].pos.z =  1.0f;
  vertex[3].pos.x = -1.0f; vertex[3].pos.y = -1.0f; vertex[3].pos.z =  1.0f;
  vertex[4].pos.x = -1.0f; vertex[4].pos.y =  1.0f; vertex[4].pos.z = -1.0f;  
  vertex[5].pos.x =  1.0f; vertex[5].pos.y =  1.0f; vertex[5].pos.z = -1.0f;  
  vertex[6].pos.x =  1.0f; vertex[6].pos.y =  1.0f; vertex[6].pos.z =  1.0f;
  vertex[7].pos.x = -1.0f; vertex[7].pos.y =  1.0f; vertex[7].pos.z =  1.0f;

  vertex[0].normal.x = 0.0f; vertex[0].normal.y = 0.0f; vertex[0].normal.z = 0.0f;  
  vertex[1].normal.x = 0.0f; vertex[1].normal.y = 0.0f; vertex[1].normal.z = 0.0f;  
  vertex[2].normal.x = 0.0f; vertex[2].normal.y = 0.0f; vertex[2].normal.z = 0.0f;
  vertex[3].normal.x = 0.0f; vertex[3].normal.y = 0.0f; vertex[3].normal.z = 0.0f;
  vertex[4].normal.x = 0.0f; vertex[4].normal.y = 0.0f; vertex[4].normal.z = 0.0f;  
  vertex[5].normal.x = 0.0f; vertex[5].normal.y = 0.0f; vertex[5].normal.z = 0.0f;  
  vertex[6].normal.x = 0.0f; vertex[6].normal.y = 0.0f; vertex[6].normal.z = 0.0f;
  vertex[7].normal.x = 0.0f; vertex[7].normal.y = 0.0f; vertex[7].normal.z = 0.0f;

  for(int i = 0; i < 8; ++i){
    vertices.push_back(vertex[i]);
  }  
}

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE prevInstance,
                    PSTR cmdLine, int showCmd)
{
  
  Window* window = new Window();
  SetGlobalInstance(hInstance);
  InitMainWindow();
  StartDirectX(NULL);
  
  /*  
  std::vector<Vertex> vertices;
  std::vector<Face> faces;
  ObjFileParser *parser = new ObjFileParser();
  XFileCreator *creator = new XFileCreator();
  parser->ParseFile(AppendToRootDir(L"obj/bimba_d.obj"), vertices, faces);
  creator->CreateXFile(AppendToRootDir(L"obj/bimba_d_scaled_10.x"), vertices, faces);
  
  delete parser;
  delete creator;
  */
  /*
  std::vector<Vertex> vertices;
  CreateCube(vertices);

  K3Tree* k3Tree = new K3Tree(vertices);
  k3Tree->FillTreeWithData();
  
  Vertex v;
  v.pos.x = -1.0f; v.pos.y = 0.0f; v.pos.z = 0.0f;
  v.normal.x = 1.0f; v.normal.y = 0.0f; v.normal.z = 0.0f;
  int indices[4];
  indices[0] = -1;
  indices[1] = -1;
  indices[2] = -1;
  indices[3] = -1;

  k3Tree->GetNearestNeighbours(v, indices, 4);
  PD(L"index :", indices[0]);
  PD(L"index :", indices[1]);
  PD(L"index :", indices[2]);
  PD(L"index :", indices[3]);
  */
  return 0;
  
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
