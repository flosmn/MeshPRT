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
                     int numNN,
                     std::vector<Vertex> approxMeshVertices,
                     std::vector<Vertex> renderMeshVertices,
                     std::vector<D3DXCOLOR> approxMeshVertexColors,
                     std::vector<D3DXCOLOR> &renderMeshVertexColors)
{
  mTree = new K3Tree(approxMeshVertices);
  mTree->FillTreeWithData();
  
  InitArrays(numNN);
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    D3DXCOLOR color = GetInterpolatedColor( numNN,
                                            renderMeshVertices[i],
                                            &approxMeshVertices,
                                            &approxMeshVertexColors); 
    
    renderMeshVertexColors.push_back(color);
  }
  FreeArrays();
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

void PRTHierarchyMapping::InitArrays(int numNN) {
  indices = new int[numNN];
  weights = new float[numNN];
  nnVertices = new Vertex[numNN];
}

void PRTHierarchyMapping::FreeArrays() {
  delete [] indices;
  delete [] weights;
  delete [] nnVertices;
}

D3DXCOLOR PRTHierarchyMapping::GetInterpolatedColor(
    int numNN,
    Vertex vertex,
    std::vector<Vertex> *pApproxMeshVertices,
    std::vector<D3DXCOLOR> *pApproxMeshVertexColors)
{
  D3DXCOLOR color(0.0f, 0.0f, 0.0f, 0.0f);
  mTree->GetNearestNeighbours(vertex, indices, numNN);
  
  float totalWeight = 0;
  for(int i = 0; i < numNN; ++i) {
    nnVertices[i] = (*pApproxMeshVertices)[indices[i]];
    float distance = GetDistance(vertex, nnVertices[i]);
    
    if(distance == 0) {
      return (*pApproxMeshVertexColors)[indices[i]];
    }
    else {
      weights[i] = 1.0f / distance;
    }

    totalWeight += weights[i];
  }
  
  for(int i = 0; i < numNN; ++i) {
    weights[i] = weights[i] / totalWeight;
  }  
  
  for(int i = 0; i < numNN; ++i) {
    color += weights[i] * (*pApproxMeshVertexColors)[indices[i]];
  }
  
  return color;
}

bool PRTHierarchyMapping::Equals(Vertex v, Vertex u) {
  bool samePos = false;
  bool sameNorm = false;

  if(    v.pos.x == u.pos.x 
      && v.pos.y == u.pos.y 
      && v.pos.z == u.pos.z)
  {
    samePos = true;
  }

  if(    v.normal.x == u.normal.x 
      && v.normal.y == u.normal.y
      && v.normal.z == u.normal.z)
  {
    sameNorm = true;
  }

  return samePos && sameNorm;
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