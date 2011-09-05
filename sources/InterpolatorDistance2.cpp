#include "InterpolatorDistance2.h"

void InterpolatorDistance2::GetInterpolationWeight(NNCandidate candidates[3], 
	int* indices, float* weights, Vertex v) 
{
	int K=3;
	float dist_v_c1 = GetDistance(v.pos, candidates[0].vertex.pos);
	float dist_v_c2 = GetDistance(v.pos, candidates[1].vertex.pos);
	float dist_v_c3 = GetDistance(v.pos, candidates[2].vertex.pos);
	
	float normalize = dist_v_c1 * dist_v_c2 + 
										dist_v_c1 * dist_v_c3 + 
										dist_v_c2 * dist_v_c3;

	if(normalize == 0.0f){
		for(int i=0; i<K; ++i){
			weights[i] = 1.0f/3.0f;
		}
	}
	else{
		weights[0] = dist_v_c2 * dist_v_c3 / normalize;
		weights[1] = dist_v_c1 * dist_v_c3 / normalize;
		weights[2] = dist_v_c1 * dist_v_c2 / normalize;
	}

	for(int i=0; i<K; ++i){
		indices[i] = candidates[i].index;
	}
}