#ifndef INTERPOLATORKNN_H
#define INTERPOLATORKNN_H

#include "Interpolator.h"
#include "Structs.h"
#include "d3dUtil.h"
#include <iostream>

class InterpolatorKNN : public Interpolator {
public:
	void GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v);
private:
	void CalculateBaryzentricCoordinates(NNCandidate candidates[3], Vertex v);
	void KollinearWithSamePosition(NNCandidate &c1, NNCandidate &c21, NNCandidate &c22, Vertex v);
	void AllSamePosition(NNCandidate candidates[3], Vertex v);
	void KollinearWithDistinctPositions(NNCandidate candidates[3], Vertex v);
	void InterpolateLinear(NNCandidate &c1, NNCandidate &c2, Vertex v);
	bool EqualPosition(Vertex u, Vertex v);
	float Length(D3DXVECTOR3 v);
	void Print(float A[3][4] );
};

#endif // INTERPOLATORKNN_H