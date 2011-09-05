#include "Interpolator.h"

void Interpolator::GetInterpolationWeight(NNCandidate candidates[3], 
	int* indices,	float* weights, Vertex v) 
{
	int K=3;
	float sum = 0.0f;
	for(int i=0; i<K; ++i){
		indices[i] = candidates[i].index;
		weights[i] = candidates[i].value;
		sum += weights[i];
	}
	if(sum == 0.0f){
		sum = 3.0f;
		weights[0] = 1.0f;
		weights[1] = 1.0f;
		weights[2] = 1.0f;
	}
	for(int i=0; i<K; ++i){
		weights[i] = weights[i] / sum;
	}
}

float Interpolator::DotProd(D3DXVECTOR3 v, D3DXVECTOR3 u) {
  return v.x * u.x + v.y * u.y + v.z * u.z;
}

D3DXVECTOR3 Interpolator::CrossProd(D3DXVECTOR3 a, D3DXVECTOR3 b) {
  float x = a.y * b.z - a.z * b.y;
	float y = a.z * b.x - a.x * b.z;
	float z = a.x * b.y - a.y * b.x;
	return D3DXVECTOR3(x, y, z);
}

D3DXVECTOR3 Interpolator::Normalize(D3DXVECTOR3 v) {
	float length = sqrt(DotProd(v, v));
	return (v / length);
}

float Interpolator::GetDistance(D3DXVECTOR3 u, D3DXVECTOR3 v) {
	float x = (u.x - v.x)*(u.x - v.x);
	float y = (u.y - v.y)*(u.y - v.y);
	float z = (u.z - v.z)*(u.z - v.z);
	return sqrt(x+y+z);
}

D3DXVECTOR3 Interpolator::SolveLES(	D3DXVECTOR3 a1, D3DXVECTOR3 a2, 
																		D3DXVECTOR3 a3, D3DXVECTOR3 b)
{
	float A[3][4];
	A[0][0] = a1.x;	A[0][1] = a2.x;	A[0][2] = a3.x;	A[0][3] = b.x;
	A[1][0] = a1.y;	A[1][1] = a2.y;	A[1][2] = a3.y;	A[1][3] = b.y;
	A[2][0] = a1.z;	A[2][1] = a2.z;	A[2][2] = a3.z;	A[2][3] = b.z;	
	
	float res[3];
	res[0] = 0.0f; res[1] = 0.0f; res[2] = 0.0f;
	int index[3]; index[0] = -1; index[1] = -1; index[2] = -1;
	float max = 0.0f;	float factor = 0.0f; float sum = 0.0f;

	// first row
	for(int i=0; i<3; ++i){
		if(abs(A[0][i])>max){
			max = abs(A[0][i]);
			index[0] = i;
		}
	}

	if(max == 0) {
		std::cout << "error max is 0" << std::endl;
		return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	factor = 1 / A[0][index[0]];
	for(int j=0; j<4; ++j){
		A[0][j] = A[0][j] * factor;
	}

	for(int row=1; row<3; ++row){
		factor = A[row][index[0]];
		for(int col=0; col<4; ++col){
			A[row][col] = A[row][col] - A[0][col] * factor;
		}
	}
	
	// second row
	max = 0.0f;
	for(int i=0; i<3; ++i){
		if(abs(A[1][i])>max){
			max = abs(A[1][i]);
			index[1] = i;
		}
	}

	if(max == 0) {
		std::cout << "error max is 0" << std::endl;
		return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	factor = 1 / A[1][index[1]];
	for(int i=0; i<4; ++i){
		A[1][i] = A[1][i] * factor;
	}
	
	factor = A[2][index[1]];
	for(int col=0; col<4; ++col){
			A[2][col] = A[2][col] - A[1][col] * factor;
	}
	
	// third row
	max = 0.0f;
	for(int i=0; i<3; ++i){
		if(abs(A[2][i])>max){
			max = abs(A[2][i]);
			index[2] = i;
		}
	}

	if(max == 0) {
		std::cout << "error max is 0" << std::endl;
		return D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	}

	factor = 1 / A[2][index[2]];
	for(int i=0; i<4; ++i){
		A[2][i] = A[2][i] * factor;
	}
	
	res[index[2]] = A[2][3];
	for(int j=1; j>=0; --j){
		sum = 0.0f;
		for(int i=0; i<3; ++i){
			if(i!=index[j]){
				sum += A[j][i] * res[i];
			}
		}
		res[index[j]] = A[j][3]-sum;
	}

	return D3DXVECTOR3(res[0], res[1], res[2]); 
}