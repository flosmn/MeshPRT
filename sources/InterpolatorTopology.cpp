#include "InterpolatorTopology.h"

InterpolatorTopology::InterpolatorTopology(Mesh* approxMesh, Mesh* renderMesh) {
	mApproxMesh = approxMesh;
	mRenderMesh = renderMesh;
}

void InterpolatorTopology::GetInterpolationWeight(NNCandidate candidates[3], 
	int* indices, float* weights, Vertex v) 
{
	Vertex* vertices = mApproxMesh->GetVertices();
	DWORD* faces = mApproxMesh->GetFaces();
	vector<DWORD>* vertexFaceAdjazency = mApproxMesh->GetVertexFaceAdjazency();
	Vertex bestMatch = candidates[0].vertex;
	DWORD indexOfBestMatch = candidates[0].index;

	vector<DWORD> adjazentFaces = vertexFaceAdjazency[indexOfBestMatch];
	
	Vertex faceVertices[3];
	float maxDotProd = -2.0f;
	DWORD indexOfBestTriangle = 0;
	for(int i=0; i<adjazentFaces.size(); ++i) {
		DWORD face = adjazentFaces[i];
		faceVertices[0] = vertices[faces[face*3+0]];
		faceVertices[1] = vertices[faces[face*3+1]];
		faceVertices[2] = vertices[faces[face*3+2]];

		D3DXVECTOR3 faceNormal = Normalize(faceVertices[0].normal + 
																			 faceVertices[1].normal + 
																			 faceVertices[2].normal);
		
		if(DotProd(v.normal, faceNormal) > maxDotProd) {
			maxDotProd = DotProd(v.normal, faceNormal);
			indexOfBestTriangle = face;
		}
	}

	for(int i=0; i<3; ++i){
		DWORD index = faces[indexOfBestTriangle*3+i];
		indices[i] = index;
		faceVertices[i] = vertices[index];
	}
	
	D3DXVECTOR3 v01 = Normalize(faceVertices[1].pos-faceVertices[0].pos);
	D3DXVECTOR3 v02 = Normalize(faceVertices[2].pos-faceVertices[0].pos);
	if(abs(DotProd(v01,v02))>0.999f){
		PD(L"warning vectors spanning triangle might be linear dependent");
	}
	
	// project v onto plane <v01, v02> with hesse normal form
	D3DXVECTOR3 normal = Normalize(CrossProd(v01, v02));
	D3DXVECTOR3 pointOnPlane = faceVertices[0].pos;
	float d = DotProd(normal, pointOnPlane);
	float distance = DotProd(normal, v.pos) - d;
	v.pos = v.pos + distance * (-normal);
	
	// assure that v.pos != (0,0,0)
	float translate = 0.0f;
	if(v.pos.x == 0 && v.pos.y == 0 && v.pos.z == 0) {
		PD(L"translate les points to aviod trivial solution");
		translate = 10;
	}
	faceVertices[0].pos.x += translate;
	faceVertices[1].pos.x += translate;
	faceVertices[2].pos.x += translate;
	v.pos.x += translate;
	
	D3DXVECTOR3 result = SolveLES(faceVertices[0].pos, faceVertices[1].pos, 
																faceVertices[2].pos, v.pos);

	//check if solution is correct
	D3DXVECTOR3 check = result.x * faceVertices[0].pos + 
											result.y * faceVertices[1].pos +
											result.z * faceVertices[2].pos -
											v.pos;

	float error = abs(check.x) + abs(check.y) + abs(check.z);
	if(error > 0.00001) {
		PD(L"big error solving lgs: ", error);
	}
	
	weights[0] = result.x;
	weights[1] = result.y;
	weights[2] = result.z;
}