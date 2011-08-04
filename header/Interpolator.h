#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "Structs.h"
#include "d3dUtil.h"
#include <iostream>

class Interpolator {
public:
	void GetInterpolationWeight(NNCandidate candidates[3], Vertex v);
private:
	void CalculateBaryzentricCoordinates(NNCandidate candidates[3], Vertex v);
	void KollinearWithSamePosition(NNCandidate &c1, NNCandidate &c21, NNCandidate &c22, Vertex v);
	void AllSamePosition(NNCandidate candidates[3], Vertex v);
	void KollinearWithDistinctPositions(NNCandidate candidates[3], Vertex v);
	void InterpolateLinear(NNCandidate &c1, NNCandidate &c2, Vertex v);
	D3DXVECTOR3 SolveLES(D3DXVECTOR3 a1, D3DXVECTOR3 a2, D3DXVECTOR3 a3, D3DXVECTOR3 b);
	float DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u);
	D3DXVECTOR3 CrossProd(D3DXVECTOR3 v, D3DXVECTOR3 u);
	D3DXVECTOR3 Normalize(D3DXVECTOR3 v);
	bool EqualPosition(Vertex u, Vertex v);
	float GetDistance(D3DXVECTOR3 u, D3DXVECTOR3 v);
	float Length(D3DXVECTOR3 v);
	void Print(float A[3][4] );
};

#endif // INTERPOLATOR_H