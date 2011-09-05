#include "K3Tree.h"

K3Tree::K3Tree(Mesh* renderMesh,	Mesh* approxMesh) {
  mQueryRadius = 0.0f;
  mQueryEpsilon = 0.0f;
  mBoundingBoxVolume = 0.0f;
	mDistanceScaling = 0.0f;
	//mInterpolator = new InterpolatorTopology(approxMesh, renderMesh);
	//mInterpolator = new InterpolatorKNN();
	//mInterpolator = new InterpolatorSimple();
	mInterpolator = new InterpolatorDistance2();
  mDebug = false;
	mRenderMesh = renderMesh;
	mApproxMesh = approxMesh;
  mTree = 0;      
  mTree = kd_create(3);
}

K3Tree::~K3Tree() {
  kd_free(mTree);
	delete mInterpolator;
}

void K3Tree::FillTreeWithData() {
	mNumVertices = mApproxMesh->GetNumVertices();
	Vertex* vertices = mApproxMesh->GetVertices();

  for ( int i = 0; i < mNumVertices; ++i ) {
    mIndex[vertices[i]] = -1;
  }

  Vertex smallestPoint, biggestPoint;
  smallestPoint.pos.x = smallestPoint.pos.y = smallestPoint.pos.z = FLT_MAX;
  biggestPoint.pos.x = biggestPoint.pos.y = biggestPoint.pos.z = FLT_MIN;
  
  for ( int i = 0; i < mNumVertices; ++i ) {
    if(mIndex[vertices[i]] == -1) {
      mIndex[vertices[i]] = i;
    }
		else{
			PD(L"override index entry at vertex ", i);
			PD(L"index entry already here is ", mIndex[vertices[i]]);
		}

    float x = vertices[i].pos.x;
    float y = vertices[i].pos.y;
    float z = vertices[i].pos.z;
    
    if(x < smallestPoint.pos.x) smallestPoint.pos.x = x;
    if(y < smallestPoint.pos.y) smallestPoint.pos.y = y;
    if(z < smallestPoint.pos.z) smallestPoint.pos.z = z;

    if(x > biggestPoint.pos.x) biggestPoint.pos.x = x;
    if(y > biggestPoint.pos.y) biggestPoint.pos.y = y;
    if(z > biggestPoint.pos.z) biggestPoint.pos.z = z;
    
    // debug
    if(mDebug) {
      PD(L"vertex ", i);
      PD(L"index: ", mIndex[vertices[i]]);
      PD(L"pos x: ", x);
      PD(L"pos y: ", y);
      PD(L"pos z: ", z);
      PD(L"norm x: ", vertices[i].normal.x);
      PD(L"norm y: ", vertices[i].normal.y);
      PD(L"norm z: ", vertices[i].normal.z);
    }

    kd_insert3f(mTree, x, y, z, &vertices[i]);   
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
	mDistanceScaling = pow(density, 1.0f/3.0f);

  PD(L"size: ", mNumVertices);
  PD(L"boudning box volume: ", mBoundingBoxVolume);
  PD(L"query radius: ", mQueryRadius);
  PD(L"query epsilon: ", mQueryEpsilon);
	PD(L"distance scaling: ", mDistanceScaling);
}

void K3Tree::GetMapping(Vertex v, int* indices,	float* weights)
{
  int K = 3;
	
  kdres *resultSet = kd_nearest_range3f(mTree,
                                        v.pos.x,
                                        v.pos.y,
                                        v.pos.z,
                                        mQueryRadius);
  
  if(kd_res_size(resultSet) < 2 * K) {
    mQueryRadius += 0.5f * mQueryEpsilon;
		if(mDebug) PD(L"query radius: ", mQueryRadius);
    GetMapping(v, indices, weights);
  }
  else{
    GetNearestNeighboursFromResultSet(v, indices, weights, K, resultSet);
    if(kd_res_size(resultSet) > 3*K && mQueryRadius - 0.1f*mQueryEpsilon > 0) {
      mQueryRadius -= 0.1f * mQueryEpsilon;
			if(mDebug) PD(L"query radius: ", mQueryRadius);
    }
  }
  
  kd_res_free(resultSet);
}

void K3Tree::GetNearestNeighboursFromResultSet(Vertex v, 
                                               int* indices,
																							 float* weights,
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
    candidates[i].value = 0;
    candidates[i].index = -1;
  }

	bool debug = false;
	if(mDebug) PD(L"nearest neighbours candidates for vertex ");
	
	for(int i = 0; i < kd_res_size(resultSet); ++i) {
		int index = -1;

    float position[3];
    Vertex* pVertex = (Vertex*)kd_res_itemf(resultSet, position);
    Vertex nn = *pVertex;
    index = mIndex[nn];

		if(mDebug) {
			PD(L"candidate: ", index);
			PD(L"Red, Point[{", nn.pos.x);
			PD(L",", nn.pos.y);
			PD(L",", nn.pos.z);
			PD(L"}]");
		}
    
		float value = GetValue(v, nn);
		if (value > candidates[K-1].value) {
      candidates[K-1].vertex = nn;
      candidates[K-1].index = index;
      candidates[K-1].value = value;
      Sort(candidates, K);
    }

    kd_res_next(resultSet);
  }

	mInterpolator->GetInterpolationWeight(candidates, indices, weights, v);
	
	delete [] candidates;
}


D3DXVECTOR3 K3Tree::Normalize(D3DXVECTOR3 v) {
	float length = DotProd(v, v);
	return (v / length);
}

bool K3Tree::EqualPosition(Vertex u, Vertex v) {
	if(u.pos.x != v.pos.x || u.pos.y != v.pos.y || u.pos.z != v.pos.z) {
		return false;
	}
	return true;
}

bool K3Tree::PerfectMatch(Vertex u, Vertex v) {
	if(abs(u.pos.x - v.pos.x) > 0.0001f) return false;
	if(abs(u.pos.y - v.pos.y) > 0.0001f) return false;
	if(abs(u.pos.z - v.pos.z) > 0.0001f) return false;
	if(abs(u.normal.x - v.normal.x) > 0.0001f) return false;
	if(abs(u.normal.y - v.normal.y) > 0.0001f) return false;
	if(abs(u.normal.z - v.normal.z) > 0.0001f) return false;
	return true;
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

float K3Tree::GetValue(Vertex v, Vertex u) {
	float value = 0.0f;
	float p = 0.5f;
	float q = 0.5f;
	float normDist = GetDistance(v,u) / mDistanceScaling;
	float dotProd = DotProd(v.normal, u.normal);

	value = p * 1/(1+normDist) + q * max(0, dotProd);
	return value;
}

float K3Tree::DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u) {
  return v.x * u.x + v.y * u.y + v.z * u.z;
}

void K3Tree::Sort(NNCandidate* candidates, int size){
  // precondition: candidates is always sorted 
  // after value (incrementing with index)
  // except for the last item
  
  for(int i = size-1; i > 0; --i){
    if(candidates[i].value > candidates[i-1].value){
      //swap candidates[i] and candidates[i-1]
      NNCandidate temp = candidates[i];
      candidates[i] = candidates[i-1];
      candidates[i-1] = temp;
    }
  }  
}