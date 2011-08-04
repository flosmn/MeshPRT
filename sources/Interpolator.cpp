#include "Interpolator.h"

void Interpolator::GetInterpolationWeight(NNCandidate candidates[3], Vertex v) {
	// assume k=3
	D3DXVECTOR3 cand0pos = candidates[0].vertex.pos;
	D3DXVECTOR3 cand1pos = candidates[1].vertex.pos;
	D3DXVECTOR3 cand2pos = candidates[2].vertex.pos;
	candidates[0].vertex.normal = Normalize(candidates[0].vertex.normal);
	candidates[1].vertex.normal = Normalize(candidates[1].vertex.normal);
	candidates[2].vertex.normal = Normalize(candidates[2].vertex.normal);

	D3DXVECTOR3 v_01 = Normalize(cand1pos-cand0pos);
	D3DXVECTOR3 v_02 = Normalize(cand2pos-cand0pos);
	float dotp = DotProd(v_01, v_02);
	
	// if all candidates are in general arrangement
	if(dotp < 1 && dotp > -1) {
		CalculateBaryzentricCoordinates(candidates, v);
	}
	// if all candidates have the same position
	else if( EqualPosition(candidates[0].vertex, candidates[1].vertex) &&
					 EqualPosition(candidates[0].vertex, candidates[2].vertex)) {
						 PD(L"all same position");
						 AllSamePosition(candidates, v);
	}
	// if the candidates are collinear
	else {
		if(EqualPosition(candidates[0].vertex, candidates[1].vertex)) {
			PD(L"KollinearWithSamePosition");
			KollinearWithSamePosition(candidates[2], candidates[0], candidates[1], v);
		}
		else if (EqualPosition(candidates[0].vertex, candidates[2].vertex)) {
			PD(L"KollinearWithSamePosition");
			KollinearWithSamePosition(candidates[1], candidates[0], candidates[2], v);
		}
		else if (EqualPosition(candidates[1].vertex, candidates[2].vertex)) {
			PD(L"KollinearWithSamePosition");
			KollinearWithSamePosition(candidates[0], candidates[1], candidates[2], v);
		}
		else {
			PD(L"KollinearWithDistinctPositions");
			KollinearWithDistinctPositions(candidates, v);
		}
	}
}


void Interpolator::KollinearWithDistinctPositions(NNCandidate candidates[3], Vertex v) {
	// project v onto line <v_01, v_02> with hesse normal form	
	D3DXVECTOR3 v_01 = candidates[1].vertex.pos-candidates[0].vertex.pos;
	D3DXVECTOR3 v_02 = candidates[2].vertex.pos-candidates[0].vertex.pos;
	D3DXVECTOR3 v_0v = v.pos-candidates[0].vertex.pos;
	D3DXVECTOR3 v_1v = v.pos-candidates[1].vertex.pos;
	D3DXVECTOR3 v_2v = v.pos-candidates[2].vertex.pos;
	D3DXVECTOR3 v_2 = CrossProd(v_01, v_0v); 
	if(v_2.x != 0.0f || v_2.y != 0.0f || v_2.z != 0.0f) {
		D3DXVECTOR3 normal = Normalize(CrossProd(v_01, v_2));
		D3DXVECTOR3 pointOnPlane = candidates[0].vertex.pos;
		float d = DotProd(normal, pointOnPlane);
		float distance = DotProd(normal, v.pos) - d;
		v.pos = v.pos + distance * (-normal);
	}

	if(DotProd(v_01, v_02) < -0.9f) {
		// 0 is in the middle
		if(DotProd(v_01, v_0v) > 0.9f){
			// v on the same side as 1
			InterpolateLinear(candidates[0], candidates[1], v);
			candidates[2].weight = 0.0f;
		}
		else{
			// v on the same side as 2
			InterpolateLinear(candidates[0], candidates[2], v);
			candidates[1].weight = 0.0f;
		}
	}
	if(Length(v_01) < Length(v_02)) {
		// 1 is in the middle
		if(DotProd(-v_01, v_1v) > 0.9f){
			// v on the same side as 0
			InterpolateLinear(candidates[1], candidates[0], v);
			candidates[2].weight = 0.0f;
		}
		else{
			// v on the same side as 2
			InterpolateLinear(candidates[1], candidates[2], v);
			candidates[0].weight = 0.0f;
		}
	}
	else{
		// 2 is in the middle
		if(DotProd(-v_02, v_2v) > 0.9f){
			// v on the same side as 0
			InterpolateLinear(candidates[2], candidates[0], v);
			candidates[1].weight = 0.0f;
		}
		else{
			// v on the same side as 1
			InterpolateLinear(candidates[2], candidates[1], v);
			candidates[0].weight = 0.0f;
		}
	}
}

float Interpolator::Length(D3DXVECTOR3 v) {
	return sqrt(DotProd(v, v));
}

void Interpolator::InterpolateLinear(NNCandidate &c1, NNCandidate &c2, Vertex v){
	float length_c1_c2 = Length(c1.vertex.pos-c2.vertex.pos);
	float length_c1_v = Length(c1.vertex.pos-v.pos);
	
	float p = length_c1_v / length_c1_c2; 
	c1.weight = 1-p;
	c2.weight = p;
}

float Interpolator::GetDistance(D3DXVECTOR3 u, D3DXVECTOR3 v) {
	float x = (u.x - v.x)*(u.x - v.x);
	float y = (u.y - v.y)*(u.y - v.y);
	float z = (u.z - v.z)*(u.z - v.z);
	return sqrt(x+y+z);
}

void Interpolator::AllSamePosition(NNCandidate candidates[3], Vertex v) {
	float weights[3];
	float sum = 0.0f;
	for(int i=0; i < 3; ++i) {
	weights[i] = max(0, DotProd(candidates[i].vertex.normal, v.normal));
		sum += weights[i];
	}
	if(sum == 0){
		sum = 3.0f;
		weights[0] = 1.0f; weights[1] = 1.0f; weights[2] = 1.0f;  
	}
	candidates[0].weight = weights[0] / sum;
	candidates[1].weight = weights[1] / sum;
	candidates[2].weight = weights[2] / sum;
}

void Interpolator::KollinearWithSamePosition(NNCandidate &c1, NNCandidate &c21, NNCandidate &c22, Vertex v) {
	D3DXVECTOR3 sv = v.pos - c1.vertex.pos;
	D3DXVECTOR3 st = c21.vertex.pos - c1.vertex.pos;
	float p = DotProd(sv, st) / DotProd(st, st); 
	float weights[2];
	weights[0] = max(0, DotProd(c21.vertex.normal, v.normal));
	weights[1] = max(0, DotProd(c22.vertex.normal, v.normal));
	if(weights[0]+weights[1] == 0){
		weights[0] = 0.5f;
		weights[1] = 0.5f;
	}		
	c1.weight = 1-p;
	c21.weight = p * weights[0] / (weights[0]+weights[1]);
	c22.weight = p * weights[1] / (weights[0]+weights[1]);
}

void Interpolator::CalculateBaryzentricCoordinates(NNCandidate candidates[3], Vertex v) {
	D3DXVECTOR3 v_01 = Normalize(candidates[1].vertex.pos-candidates[0].vertex.pos);
	D3DXVECTOR3 v_02 = Normalize(candidates[2].vertex.pos-candidates[0].vertex.pos);	
	// project v onto plane <v_01, v_02> with hesse normal form
	D3DXVECTOR3 normal = Normalize(CrossProd(v_01, v_02));
	D3DXVECTOR3 pointOnPlane = candidates[0].vertex.pos;
	float d = DotProd(normal, pointOnPlane);
	float distance = DotProd(normal, v.pos) - d;
	v.pos = v.pos + distance * (-normal);
	
	float translate = 0.0f;
	if(v.pos.x == 0 && v.pos.y == 0 && v.pos.z == 0) {
		translate = 10;
	}
		
	candidates[0].vertex.pos.x += translate;
	candidates[1].vertex.pos.x += translate;
	candidates[2].vertex.pos.x += translate;
	v.pos.x += translate;
	
	D3DXVECTOR3 res = SolveLES( candidates[0].vertex.pos, 
															candidates[1].vertex.pos, 
															candidates[2].vertex.pos, 
															v.pos);
	
	candidates[0].weight = res.x;
	candidates[1].weight = res.y;
	candidates[2].weight = res.z;

	D3DXVECTOR3 check = candidates[0].weight * candidates[0].vertex.pos + 
											candidates[1].weight * candidates[1].vertex.pos +
											candidates[2].weight * candidates[2].vertex.pos -
											v.pos;

	float error = abs(check.x) + abs(check.y) + abs(check.z);
	if(error > 0.00001) {
		PD(L"big error solving lgs: ", error);
	}
}

D3DXVECTOR3 Interpolator::SolveLES(D3DXVECTOR3 a1, D3DXVECTOR3 a2, D3DXVECTOR3 a3, D3DXVECTOR3 b) {
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

bool Interpolator::EqualPosition(Vertex u, Vertex v) {
	if(u.pos.x != v.pos.x || u.pos.y != v.pos.y || u.pos.z != v.pos.z) {
		return false;
	}
	return true;
}

void Interpolator::Print(float A[3][4] ){
	std::cout << "A:"<< std::endl;
	for(int row=0; row<3; ++row){
		for(int col=0; col<4; ++col){
			std::cout << A[row][col] << "\t";
		}
		std::cout << "\n";
	}
}