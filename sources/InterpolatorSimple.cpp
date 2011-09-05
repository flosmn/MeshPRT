#include "InterpolatorSimple.h"

void InterpolatorSimple::GetInterpolationWeight(NNCandidate candidates[3], int* indices, float* weights, Vertex v) {
	
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