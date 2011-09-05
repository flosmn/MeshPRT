#ifndef INTERPOLATOR_H
#define INTERPOLATOR_H

#include "Structs.h"
#include "d3dUtil.h"
#include <iostream>

class Interpolator {
public:
	virtual void GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v);
protected:
	float DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u);
	D3DXVECTOR3 Normalize(D3DXVECTOR3 v);
	D3DXVECTOR3 CrossProd(D3DXVECTOR3 v, D3DXVECTOR3 u);
	
	D3DXVECTOR3 SolveLES(	D3DXVECTOR3 a1, D3DXVECTOR3 a2, D3DXVECTOR3 a3, 
												D3DXVECTOR3 b);

	float GetDistance(D3DXVECTOR3 u, D3DXVECTOR3 v);
	
};

#endif // INTERPOLATOR_H