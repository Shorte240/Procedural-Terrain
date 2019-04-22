// SizeableQuad.h
// Used in generation of the l-system
#pragma once

#include "DXF.h"

using namespace DirectX;

class SizableQuad : public BaseMesh
{
public:
	SizableQuad(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width_, float height_, XMFLOAT2 quadScale);
	~SizableQuad();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
protected:
	void initBuffers(ID3D11Device* device);

	// Vertices
	VertexType* vertices;

	// Scale of the quad
	XMFLOAT2 scale_;

	// Dimensions of the quad
	float width, height;
};