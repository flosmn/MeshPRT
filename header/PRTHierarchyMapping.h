#ifndef PRTHIERARCHYMAPPING_H
#define PRTHIERARCHYMAPPING_H

#include "Structs.h"
#include "d3dUtil.h"
#include "K3Tree.h"

class PRTHierarchyMapping {
public:
  PRTHierarchyMapping();
  ~PRTHierarchyMapping();
  
  void GetMapping( Mesh* renderMesh, Mesh* approxMesh, 
									 int* mappingIndices, float* mappingWeights);

private:
  bool Equals(Vertex v, Vertex u);

  void InitArrays();
  void FreeArrays();

  float GetDistance(Vertex v, Vertex u);

  K3Tree* mTree;

  int* indices;
  float* weights;
  Vertex* nnVertices;

};

#endif // PRTHIERARCHYMAPPING_H