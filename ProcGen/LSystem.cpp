// L-System cpp
// Generates a river
#include "LSystem.h"

LSystem::LSystem(ID3D11Device* device, HWND hwnd)
{
	colourShader = new ColourShader(device, hwnd);

	lSystemParams.iterations = 2;
	lSystemParams.angle = 45.0f;
	lSystemParams.width = 0.03125f;
	lSystemParams.height = 0.5f;
	lSystemParams.minLeafLength = 0.5f;
	lSystemParams.maxLeafLength = 1.0f;
	lSystemParams.minBranchLength = 1.0f;
	lSystemParams.maxBranchLength = 2.0f;
	lSystemParams.variance = 2.0f;

	rules.insert(std::pair<char, string>('X', "[-FX][+FX]"));
	rules.insert(std::pair<char, string>('F', "FF"));
}

LSystem::~LSystem()
{
}

void LSystem::Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world_, XMMATRIX& view_, XMMATRIX& proj_, XMFLOAT3 positionOffset_)
{
	currentPath = axiom;
	quadVector.clear();
	worlds.clear();
	
	BuildString();

	XMMATRIX translationMatrix = XMMatrixTranslation(positionOffset_.x, positionOffset_.y, positionOffset_.z);
	world_ = translationMatrix;

	XMVECTOR scale;
	XMVECTOR translation;
	XMVECTOR rotationVector;
	XMMatrixDecompose(&scale, &rotationVector, &translation, world_);
	XMStoreFloat3(&currentPos, translation);
	XMStoreFloat3(&currentRot, rotationVector);

	for (int i = 0; i < currentPath.length(); i++)
	{
		switch (currentPath[i])
		{
		case 'F':
		{
			// Set initialPos = currentPos
			// translationVector = Forward vector * translation
			// Translate current position by translationVector
			// Draw/Push back new quad using initialPos, translatedPos

			// Translation amount
			float translation = 2.0f * lSystemParams.height;

			// Get current world position
			XMVECTOR s;
			XMVECTOR t;
			XMVECTOR r;
			XMMatrixDecompose(&s, &r, &t, world_);

			// Push back the world matrix to be used when rendering the quad
			worlds.push_back(world_);

			// Set initial position to current world position
			initialPos = currentPos;
			XMVECTOR Forward = { 0.0f, 0.0f, 1.0f, 1.0f };

			// Calculate translation vector
			XMVECTOR translationVector = XMVectorScale(Forward, translation);

			// Multiply current position by translation vector
			XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translationVector);
			world_ = XMMatrixMultiply(translationMatrix, world_);
			XMMatrixDecompose(&s, &r, &t, world_);
			XMStoreFloat3(&currentPos, t);
			
			// Render quad using initialPos and currentPos
			quadVector.push_back(new RiverQuad(device, deviceContext, lSystemParams.width, lSystemParams.height, initialPos, currentPos));

			break;
		}

		case 'X':
			break;

		case '+':
		{
			// rotationVector = Up vector * positive angle
			// rotate current position by rotationVectortranslatedPos
			// Get current world rotation
			XMVECTOR s;
			XMVECTOR t;
			XMVECTOR r;
			XMMatrixDecompose(&s, &r, &t, world_);

			// Set forward vector
			XMVECTOR Up = { 0.0f, 1.0f, 0.0f, 1.0f };

			// Rotate counter-clockwise in the Y-axis
			float rotation = XMConvertToRadians(lSystemParams.angle);

			// Calculate rotation vector
			XMVECTOR rotationVector = XMVectorScale(Up, rotation);

			// Multiply current rotation by rotation vector
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
			world_ = XMMatrixMultiply(rotationMatrix, world_);
			XMMatrixDecompose(&s, &r, &t, world_);
			XMStoreFloat3(&currentRot, r);

			break;
		}
		case '-':
		{
			// rotationVector = Up vector * negative angle
			// rotate current position by rotationVectortranslatedPos
			// Get current world rotation
			XMVECTOR s;
			XMVECTOR t;
			XMVECTOR r;
			XMMatrixDecompose(&s, &r, &t, world_);

			// Set forward vector
			XMVECTOR Up = { 0.0f, 1.0f, 0.0f, 1.0f};

			// Rotate clockwise in the Y-axis
			float rotation = XMConvertToRadians(-lSystemParams.angle);

			// Calculate rotation vector
			XMVECTOR rotationVector = XMVectorScale(Up, rotation);

			// Multiply current rotation by rotation vector
			XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);
			world_ = XMMatrixMultiply(rotationMatrix, world_);
			XMMatrixDecompose(&s, &r, &t, world_);
			XMStoreFloat3(&currentRot, r);

			break;
		}
		case '[':
		{
			// Save where we are
			SavedTransform s = { currentPos, currentRot, world_ };
			savedTransforms.push(s);
			break;
		}
		case ']':
			SavedTransform savedTransform = savedTransforms.top();
			world_ = savedTransform.world;
			currentPos = savedTransform.position;
			currentRot = savedTransform.rotation;
			savedTransforms.pop();
			break;

		default:
			break;
		}
	}
}

void LSystem::Render(ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj)
{
	for (int i = 0; i < quadVector.size(); i++)
	{
		quadVector[i]->sendData(deviceContext);
		colourShader->setShaderParameters(deviceContext, worlds[i], view, proj);
		colourShader->render(deviceContext, quadVector[i]->getIndexCount());
	}
}

void LSystem::ClearSystem()
{
	// Clear all data storing vectors
	// To remove the L-System from the world
	quadVector.clear();
	worlds.clear();
}

float LSystem::RandomFloatInRange(float min, float max)
{
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

void LSystem::BuildString()
{
	for (int i = 0; i < lSystemParams.iterations; i++)
	{
		string stringBuilder;
		string currentPathChars = currentPath;

		for (int j = 0; j < currentPathChars.length(); j++)
		{
			char s = currentPathChars[j];
			string n;
			if (rules.find(currentPath[j]) != rules.end())
			{
				n = rules[currentPathChars[j]];
			}
			else
			{
				n = s;
			}
			stringBuilder.append(rules.find(currentPath[j]) != rules.end() ? rules[currentPathChars[j]] : n);
		}

		currentPath = stringBuilder;
	}
}
