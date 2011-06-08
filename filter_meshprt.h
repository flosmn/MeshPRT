#ifndef FILTER_MESHPRT_H
#define FILTER_MESHPRT_H

#include <QObject>

#include <common/interfaces.h>

#include "DirectXMesh.h"
#include "Window.h"

class QScriptEngine;

class MyPlugin : public QObject, public MeshFilterInterface {
    Q_OBJECT
    Q_INTERFACES(MeshFilterInterface)

public:
    enum { FP_MOVE_VERTEX  } ;

    MyPlugin();
    ~MyPlugin();

    virtual QString pluginName(void) const { return "MyPlugin"; }

    QString filterName(FilterIDType filter) const;
    QString filterInfo(FilterIDType filter) const;

    void initParameterSet(QAction *,MeshModel &/*m*/, RichParameterSet &);

    bool applyFilter(QAction *filter, MeshDocument &md, RichParameterSet &,
                     vcg::CallBackPos * cb);

    FilterClass getClass(QAction *a);

    QString filterScriptFunctionName(FilterIDType filterID);

    int postCondition( QAction* ) const
    {
      return  MeshModel::MM_VERTCOORD | MeshModel::MM_FACENORMAL |
              MeshModel::MM_VERTNORMAL;
    }
private:
    Window* window; // window for directX

    ID3DXMesh *mMesh; // the mesh for directX which is
                        //created out of the meshlab mesh
};

#endif // FILTER_MESHPRT_H
