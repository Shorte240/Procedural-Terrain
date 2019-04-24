// L-System cpp
// Generates a river
#include "LSystem.h"

LSystem::LSystem(ID3D11Device* device, HWND hwnd)
{
	// Initialise the shader used to render the quads in the l-system
	manipulationShader = new ManipulationShader(device, hwnd);

	// Initialise the variables used in generating the l-system
	lSystemParams.iterations = 2;
	lSystemParams.angle = 45.0f;
	lSystemParams.width = 10.0f;
	lSystemParams.height = 10.0f;
	lSystemParams.scaleX = 0.1f;
	lSystemParams.scaleY = 0.5f;

	// Initialise the variables used in the wave manipulation
	// of the quads in the l-system
	waveParams.elapsedTime = 0.0f;
	waveParams.height = 0.4f;
	waveParams.frequency = 1.4f;
	waveParams.speed = 1.0f;

	// Add rules for the l-system to follow
	rules.insert(std::pair<char, string>('X', "[-FX][+FX]"));
	rules.insert(std::pair<char, string>('F', "FF"));
}

LSystem::~LSystem()
{
	// Release the manipulation shader.
	if (manipulationShader)
	{
		delete manipulationShader;
		manipulationShader = 0;
	}
}

void LSystem::Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world_, XMMATRIX& view_, XMMATRIX& proj_, XMFLOAT3 positionOffset_)
{
	// Set the current path to the starting character
	currentPath = axiom;

	// Clear any prior quad/world vectors
	sizeQuadVector.clear();
	worlds.clear();
	
	// Build the string following the rules supplied
	BuildString();

	// Translate the position of the l-system in the world
	XMMATRIX translationMatrix = XMMatrixTranslation(positionOffset_.x, positionOffset_.y, positionOffset_.z);
	world_ = translationMatrix;

	// Vectors to hold scale, translation and rotation from the decomposed world matrix
	XMVECTOR scale;
	XMVECTOR translation;
	XMVECTOR rotationVector;

	// Decompose the world matrix into scale, rotation and translation
	XMMatrixDecompose(&scale, &rotationVector, &translation, world_);

	// Store the values for position and rotation in float 3's
	XMStoreFloat3(&currentPos, translation);
	XMStoreFloat3(&currentRot, rotationVector);

	// Loop through the generated string
	// and build an l-system accordingly
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
				float translation = 2.0f * ((lSystemParams.height * lSystemParams.scaleY) - lSystemParams.scaleY);

				// Get current world position
				XMVECTOR s;
				XMVECTOR t;
				XMVECTOR r;
				XMMatrixDecompose(&s, &r, &t, world_);

				// Push back the world matrix to be used when rendering the quad
				worlds.push_back(world_);

				// Set initial position to current world position
				initialPos = currentPos;

				// Set forward vector
				XMVECTOR Forward = { 0.0f, 0.0f, 1.0f, 1.0f };

				// Calculate translation vector
				XMVECTOR translationVector = XMVectorScale(Forward, translation);

				// Set up translation matrix using translation vector
				XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translationVector);

				// Multiply the world matrix by the translation matrix
				world_ = XMMatrixMultiply(translationMatrix, world_);

				// Get the new position from the translated world matrix
				XMMatrixDecompose(&s, &r, &t, world_);

				// Store the position
				XMStoreFloat3(&currentPos, t);
			
				// Render quad using initialPos and currentPos
				sizeQuadVector.push_back(new SizableQuad(device, deviceContext, lSystemParams.width, lSystemParams.height, XMFLOAT2(lSystemParams.scaleX, lSystemParams.scaleY)));

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

				// Set up vector
				XMVECTOR Up = { 0.0f, 1.0f, 0.0f, 1.0f };

				// Rotate counter-clockwise in the Y-axis
				float rotation = XMConvertToRadians(lSystemParams.angle);

				// Calculate rotation vector
				XMVECTOR rotationVector = XMVectorScale(Up, rotation);

				// Set up rotation matrix using rotation vector
				XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);

				// Multiply the world matrix by the rotation matrix
				world_ = XMMatrixMultiply(rotationMatrix, world_);

				// Get the new rotation from the rotated world matrix
				XMMatrixDecompose(&s, &r, &t, world_);

				// Store the rotation
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

				// Set up vector
				XMVECTOR Up = { 0.0f, 1.0f, 0.0f, 1.0f};

				// Rotate clockwise in the Y-axis
				float rotation = XMConvertToRadians(-lSystemParams.angle);

				// Calculate rotation vector
				XMVECTOR rotationVector = XMVectorScale(Up, rotation);

				// Set up rotation matrix using rotation vector
				XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotationVector);

				// Multiply the world matrix by the rotation matrix
				world_ = XMMatrixMultiply(rotationMatrix, world_);

				// Get the new rotation from the rotated world matrix
				XMMatrixDecompose(&s, &r, &t, world_);

				// Store the rotation
				XMStoreFloat3(&currentRot, r);

				break;
			}
			case '[':
			{
				// Save current position, rotation and world matrix
				SavedTransform s = { currentPos, currentRot, world_ };
				savedTransforms.push(s);
				break;
			}
			case ']':
				// Reset position, rotation and world matrix to previously saved values
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

void LSystem::Render(ID3D11DeviceContext* deviceContext, XMMATRIX view, XMMATRIX proj, ID3D11ShaderResourceView* waterTexture, Light* light)
{
	// Loop through the quad vector and render and quads using the manipulation shader
	for (int i = 0; i < sizeQuadVector.size(); i++)
	{
		sizeQuadVector[i]->sendData(deviceContext);
		manipulationShader->setShaderParameters(deviceContext, worlds[i], view, proj, waterTexture, light, XMFLOAT4(waveParams.elapsedTime, waveParams.height, waveParams.frequency, waveParams.speed));
		manipulationShader->render(deviceContext, sizeQuadVector[i]->getIndexCount());
	}
}

void LSystem::ClearSystem()
{
	// Clear all data storing vectors
	// To remove the L-System from the world
	sizeQuadVector.clear();
	worlds.clear();
}

float LSystem::RandomFloatInRange(float min, float max)
{
	// Calculate random float in range
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = max - min;
	float r = random * diff;
	return min + r;
}

void LSystem::BuildString()
{
	// For number of iterations
	for (int i = 0; i < lSystemParams.iterations; i++)
	{
		string stringBuilder;
		string currentPathChars = currentPath;

		// Loop through current path and alter according to rules
		for (int j = 0; j < currentPathChars.length(); j++)
		{
			char currentChar = currentPathChars[j];
			string ruleString;

			// If char matches key for one of the rules
			if (rules.find(currentPath[j]) != rules.end())
			{
				// Set rule string to that value
				ruleString = rules[currentPathChars[j]];
			}
			else
			{
				// Otherwise set to current char
				ruleString = currentChar;
			}
			// Append to string following the rules of the l-system
			stringBuilder.append(rules.find(currentPath[j]) != rules.end() ? rules[currentPathChars[j]] : ruleString);
		}
		// Set current path to built string
		currentPath = stringBuilder;
	}
}
