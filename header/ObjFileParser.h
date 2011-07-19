#ifndef OBJFILEPARSER_H
#define OBJFILEPARSER_H

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#include "d3dUtil.h"
#include "MeshDatastructures.h"

class ObjFileParser {

public:
  void ParseFile( WCHAR* pathToFile, 
                  std::vector<Vertex> &vertices, 
                  std::vector<Face> &faces);

private:
  void ParseVertex( std::vector<std::string> tokens, 
                    std::vector<Vertex> &vertices);
  
  void ParseFace( std::vector<std::string> tokens, 
                  std::vector<Face> &faces);

};

#endif // OBJFILEPARSER_H