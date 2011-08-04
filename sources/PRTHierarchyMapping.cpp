#include "PRTHierarchyMapping.h"

PRTHierarchyMapping::PRTHierarchyMapping() {
  mTree = 0;
}

PRTHierarchyMapping::~PRTHierarchyMapping() {
  delete mTree;
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
                     std::vector<Vertex> &approxMeshVertices,
                     std::vector<Vertex> &renderMeshVertices,
                     int* mappingIndices,
										 float* mappingWeights)
{
  mTree = new K3Tree(approxMeshVertices);
  mTree->FillTreeWithData();
  
  InitArrays(numNN);
  for(int i = 0; i < renderMeshVertices.size(); ++i) {
    
		if(i == 12207) { // || i == 12217 || i == 12220 || i == 12208){
			PD(L"debug");
			mTree->SetDebug(true);
		}
		else{
			mTree->SetDebug(false);
		}
		
		Vertex vertex = renderMeshVertices[i];
		mTree->GetNearestNeighbours(vertex, indices, weights, numNN);
  
		bool debug = false;
		if(debug) PD(L"nn mapping info for vertex ", i);

		float sumOfWeights = 0;
		for(int k = 0; k < numNN; ++k) {
			if(debug) {
				PD(L"nn: ", k);
				PD(L"index :", indices[k]);
				PD(L"weights :", weights[k]);
			}

			if(abs(weights[k]) > 5.0f) {
				PD(L"big weight for vertex with index ", i);
				PD(L"weight: ", weights[k]);
			}

			mappingIndices[(i*numNN) + k] = indices[k];
			mappingWeights[(i*numNN) + k] = weights[k];
			sumOfWeights += weights[k];
		}
		
		if(sumOfWeights > 1.001f || sumOfWeights < 0.999f) {
			PD(L"sum of weights not 1");
		}
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

float PRTHierarchyMapping::GetDistance(Vertex v, Vertex u) {
  float x =   (v.pos.x-u.pos.x) * (v.pos.x-u.pos.x) 
            + (v.pos.y-u.pos.y) * (v.pos.y-u.pos.y)
            + (v.pos.z-u.pos.z) * (v.pos.z-u.pos.z);
  return sqrt(x);
}