// L-System class
// Generates a river
#pragma once

#include <d3d11.h>
#include "DXF.h"
#include "ColourShader.h"
#include "RiverQuad.h"
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
		float rotation;
		XMMATRIX world;
	};

	struct LSystemParameters
	{
		int iterations;
		float angle;
		float width;
		float height;
		float minLeafLength;
		float maxLeafLength;
		float minBranchLength;
		float maxBranchLength;
		float variance;
	};

	void Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj);
	void Render(ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj);
	float RandomFloatInRange(float min, float max);
	std::vector<RiverQuad*> quadVector;

	LSystemParameters lSystemParams;

private:
	ColourShader * colourShader;

	void BuildString();

	const string axiom = "X";
	std::map<char, string> rules;
	std::stack<SavedTransform> savedTransforms;
	std::vector<XMMATRIX> worlds;

	std::string currentPath = "";
	float randomRotations[5];
	bool isLeaf;
};

