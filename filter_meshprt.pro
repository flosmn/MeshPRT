include (../../shared.pri)

INCLUDEPATH += C:\Users\flosmn\Dropbox\Studienarbeit\meshlab-sources\meshlab\src\external\directX

win32:LIBS += -L"C:\\Users\\flosmn\\Dropbox\\Studienarbeit\\meshlab-sources\\meshlab\\src\\external\\directX" \
              -ld3d9 -ld3dx9 -ld3dx9d -ldinput8 -ldxguid

HEADERS += d3d9.h \
    Window.h \
    PRTEngine.h \
    MeshDatastructures.h \
    Mesh.h \
    Light.h \
    GfxStats.h \
    filter_meshprt.h \
    DirectXMesh.h \
    DirectInput.h \
    d3dUtil.h \
    d3dApp.h \
    Camera.h \
    localConfig.h \
    LightSource.h \
    CubeMap.h
HEADERS += d3dx9.h
HEADERS += d3dx9d.h
HEADERS += dinput.h
HEADERS += Windows.h
		
TARGET = filter_meshprt

SOURCES += \
    Window.cpp \
    PRTEngine.cpp \
    Mesh.cpp \
    Light.cpp \
    GfxStats.cpp \
    filter_meshprt.cpp \
    DirectXMesh.cpp \
    DirectInput.cpp \
    d3dUtil.cpp \
    d3dApp.cpp \
    Camera.cpp \
    MeshPRT.cpp \
    CubeMap.cpp
