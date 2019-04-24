// L-System class
// Generates a river
#pragma once

#include <d3d11.h>
#include "DXF.h"
#include "ManipulationShader.h"
#include "SizableQuad.h"
#include <map>
#include <stack>

using namespace DirectX;

class LSystem
{
public:
	LSystem(ID3D11Device* device, HWND hwnd);
	~LSystem();

	// Struct to save the position, rotation and world matrix
	struct SavedTransform
	{
		XMFLOAT3 position;
		XMFLOAT3 rotation;
		XMMATRIX world;
	};

	// Parameters to set up the dimensions
	// and settings for the l-system
	struct LSystemParameters
	{
		int iterations;
		float angle;
		float width;
		float height;
		float scaleX;
		float scaleY;
	};

	// Struct for holding the variables of the wave
	struct WaveVariables
	{
		float elapsedTime;
		float height;
		float frequency;
		float speed;
	};

	// Functions
	void Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world, XMMATRIX& view, XMMATRIX& proj, XMFLOAT3 positionOffset_);
	void Render(ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj, ID3D11ShaderResourceView* waterTexture, Light* light);
	void ClearSystem();

	LSystemParameters lSystemParams;
	
	WaveVariables waveParams;

private:
	// Functions
	void BuildString();
	float RandomFloatInRange(float min, float max);

	// Shaders
	ManipulationShader* manipulationShader;

	// Vector to hold all the quads in the l-system
	std::vector<SizableQuad*> sizeQuadVector;

	// Starting character of the l-system
	const string axiom = "X";
	// Map to hold the rules of the l-system
	std::map<char, string> rules;
	// Stack of saved transforms for the l-system
	std::stack<SavedTransform> savedTransforms;
	// Vector to hold the worlds used when rendering the quads of the l-system
	std::vector<XMMATRIX> worlds;

	// String for the current path of the l-system
	std::string currentPath = "";

	// Initial position of the l-system
	XMFLOAT3 initialPos;

	// Current position of the l-system
	XMFLOAT3 currentPos;

	// Current rotation of the l-system
	XMFLOAT3 currentRot;
};

