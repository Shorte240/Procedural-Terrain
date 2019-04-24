// Manipulation shader.h
// Used to alter the vertices of a given shape
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class ManipulationShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT3 direction;
		float padding;
	};

	struct TimeBufferType
	{
		float time;
		float height;
		float frequency;
		float speed;
	};

public:
	ManipulationShader(ID3D11Device* device, HWND hwnd);
	~ManipulationShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, Light* light, XMFLOAT4 waveVariables);

private:
	void initShader(WCHAR*, WCHAR*);

private:
	// Matrix buffer
	ID3D11Buffer * matrixBuffer;
	// Sample states
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	// Light buffer
	ID3D11Buffer* lightBuffer;
	// Time buffer
	ID3D11Buffer* timeBuffer;
};

