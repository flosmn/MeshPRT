#include "PRTHierarchyMapping.h"

PRTHierarchyMapping::PRTHierarchyMapping() {
  mTree = 0;
}

PRTHierarchyMapping::~PRTHierarchyMapping() {
  SAFE_DELETE(mTree);
}

void PRTHierarchyMapping::NearestNeighbourMappingNaive( 
                     std::vector<Vertex> approxMeshVertices,
                     std::vector<Vertex> renderMeshVertices,
                     std::vector<D3DXCOLOR> approxMeshVertexColors,
                     std::vector<D3DXCOLOR> &renderMeshVertexColors)
{
  DWORD* mapping = new DWORD[renderMeshVertices.size()];
  
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    DWORD index = GetNNNaive(renderMeshVertices[i], approxMeshVertices);
    mapping[i] = index;
  }
  
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    renderMeshVertexColors.push_back(approxMeshVertexColors[mapping[i]]);
  }

  delete [] mapping;
}

void PRTHierarchyMapping::NearestNeighbourMappingTree( 
                     std::vector<Vertex> approxMeshVertices,
                     std::vector<Vertex> renderMeshVertices,
                     std::vector<D3DXCOLOR> approxMeshVertexColors,
                     std::vector<D3DXCOLOR> &renderMeshVertexColors)
{
  double start, end, time;
  start = clock();
  
  mTree = new K3Tree(approxMeshVertices);
  mTree->FillTreeWithData();

  DWORD* mapping = new DWORD[renderMeshVertices.size()];
  
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    DWORD index = GetNNTree(renderMeshVertices[i]);
    mapping[i] = index;
  }
  
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    renderMeshVertexColors.push_back(approxMeshVertexColors[mapping[i]]);
  }

  delete [] mapping;

  end = clock();
  time = (double) (end-start)/CLOCKS_PER_SEC;
  PD(L"NN mapping took: ", time);
}

DWORD PRTHierarchyMapping::GetNNNaive(Vertex v, std::vector<Vertex> vertices){
  DWORD indexNN = 0;
  float distance = 10000000.0f;

  for(int i = 0; i < vertices.size(); ++i) {
    float tempDist = GetDistance(v, vertices[i]);
    if(tempDist < distance) {
      distance = tempDist;
      indexNN = i;
    }
  }

  return indexNN;
}

DWORD PRTHierarchyMapping::GetNNTree(Vertex v){
  int indices[1];

  mTree->GetNearestNeighbours(v, indices, 1);
  return indices[0];
}

float PRTHierarchyMapping::GetDistance(Vertex v, Vertex u) {
  float x =   (v.pos.x-u.pos.x) * (v.pos.x-u.pos.x) 
            + (v.pos.y-u.pos.y) * (v.pos.y-u.pos.y)
            + (v.pos.z-u.pos.z) * (v.pos.z-u.pos.z);
  return sqrt(x);
}