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

	VertexType* vertices;
	XMFLOAT2 size_;
	float width, height;
};