#ifndef PRTHIERARCHYMAPPING_H
#define PRTHIERARCHYMAPPING_H

#include "MeshDatastructures.h"
#include "d3dUtil.h"
#include "K3Tree.h"

class PRTHierarchyMapping {
public:
  PRTHierarchyMapping();
  ~PRTHierarchyMapping();
  
  void NearestNeighbourMappingNaive( std::vector<Vertex> approxMeshVertices,
                                     std::vector<Vertex> renderMeshVertices,
                                     std::vector<D3DXCOLOR> approxMeshVertexColors,
                                     std::vector<D3DXCOLOR> &renderMeshVertexColors);

  void NearestNeighbourMappingTree( int numberOfNearestNeighbours,
                                    std::vector<Vertex> approxMeshVertices,
                                    std::vector<Vertex> renderMeshVertices,
                                    std::vector<D3DXCOLOR> approxMeshVertexColors,
                                    std::vector<D3DXCOLOR> &renderMeshVertexColors);
private:
  DWORD GetNNNaive(Vertex v, std::vector<Vertex> vertices);

  DWORD GetNNTree(Vertex v);

  bool Equals(Vertex v, Vertex u);

  void InitArrays(int numberOfNearestNeighbours);
  void FreeArrays();

  D3DXCOLOR GetInterpolatedColor(int numNN,
                                 Vertex vertex,
                                 std::vector<Vertex> *approxMeshVertices,
                                 std::vector<D3DXCOLOR> *approxMeshVertexColors);

  float GetDistance(Vertex v, Vertex u);

  K3Tree* mTree;

  int* indices;
  float* weights;
  Vertex* nnVertices;

};

#endif // PRTHIERARCHYMAPPING_H