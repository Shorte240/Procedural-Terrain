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
	ID3D11Buffer * matrixBuffer;
	ID3D11SamplerState* sampleState;
	ID3D11SamplerState* sampleState2;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* timeBuffer;
};

