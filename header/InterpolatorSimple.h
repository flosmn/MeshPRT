#ifndef INTERPOLATORSIMPLE_H
#define INTERPOLATORSIMPLE_H

#include "Interpolator.h"
#include "Structs.h"
#include "d3dUtil.h"
#include <iostream>

class InterpolatorSimple : public Interpolator {
public:
	void GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v);
};

#endif // INTERPOLATORSIMPLE_H