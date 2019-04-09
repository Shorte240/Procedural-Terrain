#pragma once

#include "DXF.h"

using namespace DirectX;

class RiverQuad : public BaseMesh
{
public:
	RiverQuad(ID3D11Device* device, ID3D11DeviceContext* deviceContext, float width_, float height_);
	~RiverQuad();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
protected:
	void initBuffers(ID3D11Device* device);

	float width, height;
};