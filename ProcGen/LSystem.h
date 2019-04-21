// L-System class
// Generates a river
#pragma once

#include <d3d11.h>
#include "DXF.h"
#include "ColourShader.h"
#include "ManipulationShader.h"
#include "RiverQuad.h"
#include "SizableQuad.h"
#include <map>
#include <stack>

using namespace DirectX;

class LSystem
{
public:
	LSystem(ID3D11Device* device, HWND hwnd);
	~LSystem();

	struct SavedTransform
	{
		XMFLOAT3 position;
		XMFLOAT3 rotation;
		XMMATRIX world;
	};

	struct LSystemParameters
	{
		int iterations;
		float angle;
		float width;
		float height;
		float scaleX;
		float scaleY;
	};

	struct WaveVariables
	{
		float elapsedTime;
		float height;
		float frequency;
		float speed;
	};

	void Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj, XMFLOAT3 positionOffset_);
	void Render(ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj, ID3D11ShaderResourceView* waterTexture, Light* light);
	void ClearSystem();

	LSystemParameters lSystemParams;
	
	WaveVariables waveParams;

private:
	ColourShader * colourShader;
	ManipulationShader* manipulationShader;

	void BuildString();
	float RandomFloatInRange(float min, float max);
	
	std::vector<SizableQuad*> sizeQuadVector;

	const string axiom = "X";
	std::map<char, string> rules;
	std::stack<SavedTransform> savedTransforms;
	std::vector<XMMATRIX> worlds;

	std::string currentPath = "";
	XMFLOAT3 initialPos;
	XMFLOAT3 currentPos;
	XMFLOAT3 currentRot;
};

