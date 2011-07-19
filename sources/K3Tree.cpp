#include "K3Tree.h"

K3Tree::K3Tree(std::vector<Vertex> &vertices) {
  mQueryRadius = 0.0f;
  mQueryEpsilon = 0.0f;
  mBoundingBoxVolume = 0.0f;
  
  mVertices = vertices;
  mTree = 0;      
  mTree = kd_create(3);
}

K3Tree::~K3Tree() {
  kd_free(mTree);
}

void K3Tree::FillTreeWithData() {
  mNumVertices = mVertices.size();

  for ( int i = 0; i < mVertices.size(); ++i ) {
    mIndex[mVertices[i]] = -1;
  }

  Vertex smallestPoint, biggestPoint;
  smallestPoint.pos.x = smallestPoint.pos.y = smallestPoint.pos.z = FLT_MAX;
  biggestPoint.pos.x = biggestPoint.pos.y = biggestPoint.pos.z = FLT_MIN;
  
  for ( int i = 0; i < mVertices.size(); ++i ) {
    if(mIndex[mVertices[i]] == -1) {
      mIndex[mVertices[i]] = i;
    }

    float x = mVertices[i].pos.x;
    float y = mVertices[i].pos.y;
    float z = mVertices[i].pos.z;
    
    if(x < smallestPoint.pos.x) smallestPoint.pos.x = x;
    if(y < smallestPoint.pos.y) smallestPoint.pos.y = y;
    if(z < smallestPoint.pos.z) smallestPoint.pos.z = z;

    if(x > biggestPoint.pos.x) biggestPoint.pos.x = x;
    if(y > biggestPoint.pos.y) biggestPoint.pos.y = y;
    if(z > biggestPoint.pos.z) biggestPoint.pos.z = z;
    
    // debug
    if(false) {
      PD(L"vertex ", i);
      PD(L"index: ", mIndex[mVertices[i]]);
      PD(L"pos x: ", x);
      PD(L"pos y: ", y);
      PD(L"pos z: ", z);
      PD(L"norm x: ", mVertices[i].normal.x);
      PD(L"norm y: ", mVertices[i].normal.y);
      PD(L"norm z: ", mVertices[i].normal.z);
    }

    kd_insert3f(mTree, x, y, z, &mVertices[i]);   
  }

  float dx = abs(biggestPoint.pos.x - smallestPoint.pos.x);
  float dy = abs(biggestPoint.pos.y - smallestPoint.pos.y);
  float dz = abs(biggestPoint.pos.z - smallestPoint.pos.z);
  mBoundingBoxVolume = dx*dy*dz;
  mBBDiagonalLength = sqrt(dx*dx+dy*dy+dz*dz);

  ComputeInitialQueryRadiusAndEpsilon();
}

void K3Tree::ComputeInitialQueryRadiusAndEpsilon() {
  float density = mBoundingBoxVolume / mNumVertices;
  float q = 3.0f/(4.0f*D3DX_PI) * density;
  mQueryRadius = pow(q, 1.0f/3.0f);
  mQueryEpsilon = mQueryRadius;

  PD(L"size: ", mNumVertices);
  PD(L"boudning box volume: ", mBoundingBoxVolume);
  PD(L"query radius: ", mQueryRadius);
  PD(L"query epsilon: ", mQueryEpsilon);
}

void K3Tree::GetNearestNeighbours(Vertex v, 
                                  int* indices,
                                  int numberOfNearestNeigbours)
{
  int K = numberOfNearestNeigbours;

  kdres *resultSet = kd_nearest_range3f(mTree,
                                        v.pos.x,
                                        v.pos.y,
                                        v.pos.z,
                                        mQueryRadius);
  
  if(kd_res_size(resultSet) < K) {
    mQueryRadius += 0.5f * mQueryEpsilon;
    GetNearestNeighbours(v, indices, K);
  }
  else{
    GetNearestNeighboursFromResultSet(v, indices, K, resultSet);
    if(kd_res_size(resultSet) > 5*K && mQueryRadius-0.1f * mQueryEpsilon > 0) {
      mQueryRadius -= 0.1f * mQueryEpsilon;
    }
  }
  
  kd_res_free(resultSet);
}

void K3Tree::GetNearestNeighboursFromResultSet(Vertex v, 
                                               int* indices,
                                               int numberOfNearestNeighbours,
                                               kdres* resultSet) 
{
  int K = numberOfNearestNeighbours;
  NNCandidate* candidates = new NNCandidate[K];
  
  for(int i = 0; i < K; ++i) {
    candidates[i].vertex.pos.x = FLT_MAX;
    candidates[i].vertex.pos.y = FLT_MAX;
    candidates[i].vertex.pos.z = FLT_MAX;
    candidates[i].vertex.normal.x = 0.0f;
    candidates[i].vertex.normal.y = 0.0f;
    candidates[i].vertex.normal.z = 0.0f;
    candidates[i].distance = FLT_MAX;
    candidates[i].index = -1;
  }

  for(int i = 0; i < kd_res_size(resultSet); ++i) {
    int index = -1;

    float position[3];
    Vertex* pVertex = (Vertex*)kd_res_itemf(resultSet, position);
    Vertex nn = *pVertex;
    index = mIndex[nn];
    
    if(BetterNN(v, candidates[K-1].vertex, nn)) {
      candidates[K-1].vertex = nn;
      candidates[K-1].index = index;
      candidates[K-1].distance = GetDistance(v, nn);
      Sort(candidates, K);
    }

    kd_res_next(resultSet);
  }

  for(int i = 0; i < K; ++i) {
    indices[i] = candidates[i].index;
  }
  
  delete [] candidates;
}

bool K3Tree::BetterNN(Vertex queryVertex, Vertex current, Vertex candidate){                                             
  float currentDistance = GetDistance(queryVertex, current);
  float candidateDistance = GetDistance(queryVertex, candidate);

  float normalizedCurrDist = currentDistance / mBBDiagonalLength;
  float normalizedCandDist = candidateDistance / mBBDiagonalLength;
  
  float currentDotP = DotProd(queryVertex.normal, current.normal);
  float candidateDotP = DotProd(queryVertex.normal, candidate.normal);

  float weightDistance = 0.5f;
  float weightNormals = 0.5f;
  
  float valueCurrent = weightDistance * normalizedCurrDist
                       + (weightNormals * -currentDotP);
  float valueCandidate =  weightDistance * normalizedCandDist
                          + (weightNormals * -candidateDotP);

  if(valueCandidate < valueCurrent) {
    return true;
  }

  return false;
}
    
float K3Tree::GetDistance(Vertex v, float x, float y, float z) {
  float d = pow(v.pos.x - x, 2) + pow(v.pos.y - y, 2) + pow(v.pos.z - z, 2);
  return sqrt(d);
}

float K3Tree::GetDistance(Vertex v, Vertex u) {
  float d =   pow(v.pos.x - u.pos.x, 2)
            + pow(v.pos.y - u.pos.y, 2)
            + pow(v.pos.z - u.pos.z, 2);
  return sqrt(d);
}

float K3Tree::DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u) {
  return v.x * u.x + v.y * u.y + v.z * u.z;
}

void K3Tree::Sort(NNCandidate* candidates, int size){
  // precondition: candidates is always sorted 
  // after distance (incrementing with index)
  // except for the last item
  
  for(int i = size-1; i > 0; --i){
    if(candidates[i].distance < candidates[i-1].distance){
      //swap candidates[i] and candidates[i-1]
      NNCandidate temp = candidates[i];
      candidates[i] = candidates[i-1];
      candidates[i-1] = temp;
    }
  }  
}