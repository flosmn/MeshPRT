#include "filter_meshprt.h"
#include <QtScript>
#include <QFile>
#include <QCoreApplication>
#include <QTextStream>

// Constructor usually performs only two simple tasks of filling the two lists 
//  - typeList: with all the possible id of the filtering actions
//  - actionList with the corresponding actions. If you want to add icons to
//    your filtering actions you can do here by construction the QActions
//    accordingly
MyPlugin::MyPlugin() {
  typeList << FP_MOVE_VERTEX;

  foreach(FilterIDType tt , types())
    actionList << new QAction(filterName(tt), this);

  window = new Window();
  mesh = new DirectXMesh();
}

MyPlugin::~MyPlugin() {
  delete window;
  delete mesh;
}

// ST() must return the very short string describing each filtering action 
// (this string is used also to define the menu entry)
QString MyPlugin::filterName(FilterIDType filterId) const {
  switch(filterId) {
    case FP_MOVE_VERTEX :  return QString("MeshPRT");
    default : assert(0);
  }
  return QString();
}

// Info() must return the longer string describing each filtering action 
// (this string is used in the About plugin dialog)
QString MyPlugin::filterInfo(FilterIDType filterId) const {
  switch(filterId) {
    case FP_MOVE_VERTEX :  return QString("Colorize vertices.");
    default : assert(0);
  }
  return QString("Unknown Filter");
}

// The FilterClass describes in which generic class of filters it fits. 
// This choice affect the submenu in which each filter will be placed 
// More than a single class can be choosen.
MyPlugin::FilterClass MyPlugin::getClass(QAction *a) {
  switch(ID(a)) {
    case FP_MOVE_VERTEX :  return MeshFilterInterface::VertexColoring;
    default : assert(0);
  }
  return MeshFilterInterface::Generic;
}

// This function defines the needed parameters for each filter. Return true if
// the filter has some parameters it is called every time, so you can set the
// default value of parameters according to the mesh
// For each parameter you need to define, 
// - the name of the parameter, 
// - the string shown in the dialog 
// - the default value
// - a possibly long string describing the meaning of that parameter
//   (shown as a popup help in the dialog)
void MyPlugin::initParameterSet(QAction *action,MeshModel &m,
                                RichParameterSet & parlst) {
  switch(ID(action)) {
    case FP_MOVE_VERTEX :
      parlst.addParam(new RichBool ("NormalColoring",
                                    true,
                                   "Color Vertices with normal values",
                                   "Color Vertices with normal values"));
      break;
    default : assert(0);
  }
}

// The Real Core Function doing the actual mesh processing.
bool MyPlugin::applyFilter(QAction */*filter*/, MeshDocument &md,
                           RichParameterSet & par, vcg::CallBackPos *cb) {

  QFile file("testfile");
  file.open(QIODevice::WriteOnly | QIODevice::Text);
  QTextStream out(&file);
  out << "hallo";
  file.close();

  //MeshModel &m=*(md.mm());

  //mesh->CreateDirectXMesh(m);

  //WCHAR* path = L"models/meshlab.x";
  //mesh->SaveMeshToFile(AppendToRootDir(path));

  //mesh->CloneMesh(&mMesh);
  //window->OpenWindow(mMesh);

  return true;
}

QString MyPlugin::filterScriptFunctionName( FilterIDType filterID ) {
  switch(filterID) {
    case FP_MOVE_VERTEX :  return QString("Precompute Radiance Transfer");
    default : assert(0);
  }
  return QString();
}

Q_EXPORT_PLUGIN(MyPlugin)
