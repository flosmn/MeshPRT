#include "PRTHierarchyMapping.h"

PRTHierarchyMapping::PRTHierarchyMapping() {
  mTree = 0;
}

PRTHierarchyMapping::~PRTHierarchyMapping() {
  delete mTree;
}

void PRTHierarchyMapping::GetMapping(
                    Mesh* renderMesh, Mesh* approxMesh,
										int* mappingIndices, float* mappingWeights)
{
	mTree = new K3Tree(renderMesh, approxMesh);
  mTree->FillTreeWithData();
  bool debug = false;
  InitArrays();
	for(int i = 0; i < renderMesh->GetNumVertices(); ++i) {
    Vertex* vertices = renderMesh->GetVertices();
		Vertex vertex = vertices[i];
		
		if(i == 12314 || i == 12272 || i == 12266) {
			PD(L"mapping info for vertex ", i);
			PD(L"Blue, Point[{", vertex.pos.x);
			PD(L",", vertex.pos.y);
			PD(L",", vertex.pos.z);
			PD(L"}]");
			mTree->SetDebug(true);
			debug = true;
		}
		else{
			mTree->SetDebug(false);
			debug = false;
		}
				
		mTree->GetMapping(vertex, indices, weights);
  
		float sumOfWeights = 0;
		for(int k = 0; k < 3; ++k) {
			if(debug) {
				PD(L"nn: ", k);
				PD(L"index :", indices[k]);
				PD(L"weights :", weights[k]);
			}
			
			mappingIndices[(i*3) + k] = indices[k];
			mappingWeights[(i*3) + k] = weights[k];
			sumOfWeights += weights[k];
		}
		
		if(sumOfWeights > 1.001f || sumOfWeights < 0.999f) {
			PD(L"sum of weights not 1");
		}
  }
  FreeArrays();
}

void PRTHierarchyMapping::InitArrays() {
  indices = new int[3];
  weights = new float[3];
  nnVertices = new Vertex[3];
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