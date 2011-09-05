#ifndef INTERPOLATORDISTANCE2_H
#define INTERPOLATORDISTANCE2_H

#include "Structs.h"
#include "Interpolator.h"
#include "Mesh.h"
#include "d3dUtil.h"
#include <iostream>

using namespace std;

class InterpolatorDistance2 : public Interpolator {
public:
	void GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v);
};

#endif // INTERPOLATORDISTANCE2_H