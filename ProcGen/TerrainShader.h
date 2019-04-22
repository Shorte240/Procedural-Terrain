// Terrain shader.h
// Basic single light shader setup
#pragma once

#include "DXF.h"

using namespace std;
using namespace DirectX;

class TerrainShader : public BaseShader
{
private:
	struct LightBufferType
	{
		XMFLOAT4 ambient;
		XMFLOAT4 diffuse;
		XMFLOAT4 direction;
	};
public:
	TerrainShader(ID3D11Device* device, HWND hwnd);
	~TerrainShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* grassTexture, ID3D11ShaderResourceView* slopeTexture, ID3D11ShaderResourceView* rockTexture, Light* light);

private:
	void initShader(WCHAR* vsFilename, WCHAR* psFilename);

private:
	// Matrix buffer
	ID3D11Buffer* matrixBuffer;
	// Sample state
	ID3D11SamplerState* sampleState;
	// Light buffer
	ID3D11Buffer* lightBuffer;
};

