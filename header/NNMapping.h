#ifndef NNMAPPING_H
#define NNMAPPING_H

#include "d3dUtil.h"

class NNMapping{
public:
	NNMapping(DWORD numberOfNearestNeightbours);
	~NNMapping();
private:
	DWORD* indices;
	float* weights;
};

#endif // NNMAPPING_H