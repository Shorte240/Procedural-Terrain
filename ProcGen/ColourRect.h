#pragma once

#include "DXF.h"

using namespace DirectX;

class ColourRect : public BaseMesh
{
public:
	ColourRect(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	~ColourRect();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
protected:
	void initBuffers(ID3D11Device* device);
};