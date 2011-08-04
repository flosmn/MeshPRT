#ifndef K3TREE_H
#define K3TREE_H

#include "kdtree.h"
#include "Interpolator.h"
#include "Mesh.h"
#include "d3dUtil.h"
#include "Structs.h"
#include <map>
#include <math.h>
#include <assert.h>
#include <limits>

class K3Tree {
public:
  K3Tree(std::vector<Vertex> &vertices);
  ~K3Tree();

  void GetNearestNeighbours(Vertex v,
                            int* indices,
							float* weights,
                            int numberOfNearestNeighbours);

  void FillTreeWithData();

	void SetDebug(bool b) { mDebug = b; }

private:
	void ComputeInitialQueryRadiusAndEpsilon();
  
  void GetNearestNeighboursFromResultSet(Vertex v, 
                                         int* indices,
																				 float* weights,
                                         int numberOfNearestNeighbours,
                                         kdres* resultSet);

  float GetDistance(Vertex v, float x, float y, float z);
  float GetDistance(Vertex v, Vertex u);
  float DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u);
	
	bool PerfectMatch(Vertex u, Vertex v);

  float GetValue(Vertex v, Vertex u);

	D3DXVECTOR3 K3Tree::Normalize(D3DXVECTOR3 v);

	bool EqualPosition(Vertex u, Vertex v);

  void Sort(NNCandidate* candidates, int size);
  
  float mBoundingBoxVolume;
  float mBBDiagonalLength;
  float mQueryRadius;
  float mQueryEpsilon;
  int mNumVertices;

	float mDistanceScaling;
  
  std::vector<Vertex> mVertices;
  std::map<Vertex, int, Vertex> mIndex;
  
  kdtree *mTree;
	Interpolator* mInterpolator;

	bool mDebug;
};

#endif // K3TREE_H