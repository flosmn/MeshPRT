#ifndef K3TREE_H
#define K3TREE_H

#include "kdtree\kdtree.h"
#include "Mesh.h"
#include "d3dUtil.h"
#include <map>
#include <math.h>
#include <assert.h>
#include <limits>

#pragma comment(lib, "kdtree.lib")

class K3Tree {
public:
  K3Tree(std::vector<Vertex> vertices);
  ~K3Tree();
  
  void Free();

  void GetNearestNeighbours(Vertex v,
                            int* indices,
                            int numberOfNearestNeighbours);

  void FillTreeWithData();

private:
  struct NNCandidate{
    Vertex vertex;
    float distance;
    int index;
  };
  
  void ComputeInitialQueryRadiusAndEpsilon();
  
  void GetNearestNeighboursFromResultSet(Vertex v, 
                                         int* indices,
                                         int numberOfNearestNeighbours,
                                         struct kdres* resultSet);

  float GetDistance(Vertex v, float x, float y, float z);
  float GetDistance(Vertex v, Vertex u);
  float DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u);

  bool BetterNN(Vertex queryVertex, Vertex current, Vertex candidate);

  void Sort(NNCandidate* candidates, int size);
  
  float mBoundingBoxVolume;
  float mBBDiagonalLength;
  float mQueryRadius;
  float mQueryEpsilon;
  int mNumVertices;
  
  std::vector<Vertex> mVertices;
  std::map<Vertex, int, Vertex> mIndex;
  
  kdtree *mTree;
};

#endif // K3TREE_H