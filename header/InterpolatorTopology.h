#ifndef INTERPOLATORTOPOLOGY_H
#define INTERPOLATORTOPOLOGY_H

#include "Structs.h"
#include "Interpolator.h"
#include "Mesh.h"
#include "d3dUtil.h"
#include <iostream>

using namespace std;

class InterpolatorTopology : public Interpolator {
public:
	InterpolatorTopology(Mesh* approxMesh, Mesh* renderMesh);

	void GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v);
private:
	Mesh* mRenderMesh;
	Mesh* mApproxMesh;
};

#endif // INTERPOLATORTOPOLOGY_H