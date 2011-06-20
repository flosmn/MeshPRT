#ifndef PRTHIERARCHYMAPPING_H
#define PRTHIERARCHYMAPPING_H

#include "MeshDatastructures.h"
#include "d3dUtil.h"
#include "K3Tree.h"
#include <time.h>

class PRTHierarchyMapping {
public:
  PRTHierarchyMapping();
  ~PRTHierarchyMapping();
  
  void NearestNeighbourMappingNaive( std::vector<Vertex> approxMeshVertices,
                                     std::vector<Vertex> renderMeshVertices,
                                     std::vector<D3DXCOLOR> approxMeshVertexColors,
                                     std::vector<D3DXCOLOR> &renderMeshVertexColors);

  void NearestNeighbourMappingTree( std::vector<Vertex> approxMeshVertices,
                                    std::vector<Vertex> renderMeshVertices,
                                    std::vector<D3DXCOLOR> approxMeshVertexColors,
                                    std::vector<D3DXCOLOR> &renderMeshVertexColors);
private:
  DWORD GetNNNaive(Vertex v, std::vector<Vertex> vertices);

  DWORD GetNNTree(Vertex v);

  float GetDistance(Vertex v, Vertex u);

  K3Tree* mTree;
};

#endif // PRTHIERARCHYMAPPING_H