#include "NNMapping.h"

NNMapping::NNMapping(DWORD numNN) {
	indices = new DWORD[numNN];
	weights = new float[numNN];
}

NNMapping::~NNMapping() {
	delete [] indices;
	delete [] weights;
}