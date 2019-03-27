#ifndef _TESSELATEDPLANE_H_
#define _TESSELATEDPLANE_H_

#include "BaseMesh.h"
#include <cmath>

using namespace DirectX;

class TessellatedPlane : public BaseMesh
{
public:
	TessellatedPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int _scale, int _patchSize);
	~TessellatedPlane();
	
	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	void GenerateHeightMap(ID3D11Device* device);
	void SmoothVertices(ID3D11Device* device);
	void Faulting(ID3D11Device* device, float displacement);
	void CircleAlgorithm(ID3D11Device* device, float displacement);
	void MidpointDisplacement(ID3D11Device* device, float displacement);

private:
	void initBuffers(ID3D11Device* device);
	void FlatPlane();
	void CalculateNormals();
	float RandomIntRange(int min, int max);
	
	int scale, patchSize;

	VertexType* vertices;
};

#endif