#include "ObjFileParser.h"
#include <iterator>

using namespace std;

void ObjFileParser::ParseFile(WCHAR* pathToFile, 
                              std::vector<Vertex> &vertices,  
                              std::vector<Face> &faces)
{
  string line;
  ifstream file(pathToFile);
  
  if (!file.is_open()) {
    PD(L"error: file not found");
    return;
  }

  while (file.good()) {
    getline(file, line);
    istringstream ss(line);
    vector<string> tokens;
    
    copy( istream_iterator<string>(ss),
          istream_iterator<string>(),
          back_inserter<vector<string>>(tokens));
    
    if(tokens.size() != 4) continue;

    if(tokens[0] == "v") {
      ParseVertex(tokens, vertices);
    }
    if(tokens[0] == "f") {
      ParseFace(tokens, faces);
    }
  }

  PD(L"number of vertices parsed: ", (int)vertices.size());
  PD(L"number of faces parsed: ", (int)faces.size());

  file.close();

  PD(L"file parsed sucessfully");
}

void ObjFileParser::ParseVertex(std::vector<std::string> tokens, 
                                std::vector<Vertex> &vertices)
{
  if(tokens.size() != 4 || tokens[0] != "v") {
    PD(D3DERR_INVALIDCALL, L"token represents no vertex");
    return;
  }
    
  Vertex vertex;
  stringstream ss;
  float coords[3];

  for(int i = 1; i < 4; ++i){
    ss << tokens[i];
    ss >> coords[i-1];
    ss.clear();
  }

  vertex.pos.x = coords[0];
  vertex.pos.y = coords[1];
  vertex.pos.z = coords[2];
  
  vertices.push_back(vertex);

  return;
}

void ObjFileParser::ParseFace(std::vector<std::string> tokens, 
                              std::vector<Face> &faces)
{
  if(tokens.size() != 4 || tokens[0] != "f") {
    PD(D3DERR_INVALIDCALL, L"token represents no face");
    return;
  }
    
  Face face;
  stringstream ss;
  
  for(int i = 1; i < 4; ++i){
    ss << tokens[i];
    ss >> face.vertices[i-1];
    ss.clear();

    // indices on obj. format range from 1 to #vertices. We start at 0.
    face.vertices[i-1]--;
  }
    
  faces.push_back(face);

  return;
}