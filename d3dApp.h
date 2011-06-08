#ifndef D3DAPP_H
#define D3DAPP_H

#include "d3dUtil.h"
#include "Window.h"

bool StartDirectX(ID3DXMesh* mesh);

class D3DApp
{
public:
    D3DApp(std::string winCaption, D3DDEVTYPE devType, DWORD requestedVP);
    virtual ~D3DApp();

    // Framework methods.  Derived client class overrides these methods to
    // implement specific application requirements.
    virtual bool checkDeviceCaps()     { return true; }
    virtual void onLostDevice()        {}
    virtual void onResetDevice()       {}
    virtual void updateScene(float dt) {}
    virtual void drawScene()           {}

    // Override these methods only if you do not like the default window creation,
    // direct3D device creation, window procedure, or message loop.  In general,
    // for the sample programs of this book, we will not need to modify these.
    virtual void initDirect3D();
    virtual int run();
    virtual LRESULT msgProc(UINT msg, WPARAM wParam, LPARAM lParam);

    void enableFullScreenMode(bool enable);
    bool isDeviceLost();

protected:
    // Derived client class can modify these data members in the constructor to
    // customize the application.
    std::string mMainWndCaption;
    D3DDEVTYPE  mDevType;
    DWORD       mRequestedVP;

    // Application, Windows, and Direct3D data members.
    IDirect3D9*           md3dObject;
    bool                  mAppPaused;
    D3DPRESENT_PARAMETERS md3dPP;
};

// Globals for convenient access.
extern D3DApp* gd3dApp;
extern IDirect3DDevice9* gd3dDevice;

#endif // D3DAPP_H
