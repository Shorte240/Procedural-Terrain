// L-System cpp
// Generates a river
#include "LSystem.h"

LSystem::LSystem(ID3D11Device* device, HWND hwnd)
{
	colourShader = new ColourShader(device, hwnd);

	lSystemParams.iterations = 2;
	lSystemParams.angle = 12.5f;
	lSystemParams.width = 0.5f;
	lSystemParams.minLeafLength = 0.5f;
	lSystemParams.maxLeafLength = 1.0f;
	lSystemParams.minBranchLength = 1.0f;
	lSystemParams.maxBranchLength = 2.0f;
	lSystemParams.variance = 2.0f;

	for (int i = 0; i < 5; i++)
	{
		randomRotations[i] = RandomFloatInRange(-1, 1);
	}

	rules.insert(std::pair<char, string>('X', "[-FX][+FX]"));
	rules.insert(std::pair<char, string>('F', "FF"));
}

LSystem::~LSystem()
{
}

void LSystem::Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMMATRIX& world_, XMMATRIX& view_, XMMATRIX& proj_)
{
	currentPath = axiom;
	quadVector.clear();
	worlds.clear();
	
	BuildString();

	XMVECTOR scale;
	XMVECTOR translation;
	XMVECTOR rotationVector;
	XMMatrixDecompose(&scale, &rotationVector, &translation, world_);
	XMFLOAT3 pos;
	XMFLOAT3 rot;
	XMStoreFloat3(&pos, translation);
	XMStoreFloat3(&rot, rotationVector);

	for (int i = 0; i < currentPath.length(); i++)
	{
		switch (currentPath[i])
		{
		case 'F':
		{

			if (currentPath[i + 1] == 'X' || currentPath[i + 3] == 'F' && currentPath[i + 4] == 'X')
			{
				isLeaf = true;
				// Translate up along leaf length;
				float translation = lSystemParams.maxBranchLength;//RandomFloatInRange(lSystemParams.minLeafLength, lSystemParams.maxLeafLength);
				//world_ *= XMMatrixTranslation(0.0f, translation, 0.0f);
				pos.y += translation;
			}
			else
			{
				isLeaf = false;
				// Translate up along branch length;
				float translation = lSystemParams.minLeafLength;//RandomFloatInRange(lSystemParams.minBranchLength, lSystemParams.maxBranchLength);
				//world_ *= XMMatrixTranslation(0.0f, translation, 0.0f);
				pos.y += translation;
			}

			break;
		}

		case 'X':
			break;

		case '+':
		{
			// Rotate clockwise in the Z-axis
			float rotation = lSystemParams.angle;// *(1.0f + variance / 100.0f + randomRotations[i % 5]);
			//world_ *= XMMatrixRotationRollPitchYaw(0.0f, 0.0f, rotation);
			rot.z += rotation;
			break;
		}
		case '-':
		{
			// Rotate counter-clockwise in the Z-axis
			float rotation = lSystemParams.angle;// *(1.0f + variance / 100.0f + randomRotations[i % 5]);
			//world_ *= XMMatrixRotationRollPitchYaw(0.0f, 0.0f, -rotation);
			rot.z -= rotation;
			break;
		}
		//case '*':
		//{
		//	// Rotate positively in the Y-axis
		//	float rotation = 120.f * (1.0f + lSystemParams.variance / 100.f + randomRotations[i % 5]);
		//	//world_ *= XMMatrixRotationRollPitchYaw(0 * rotation, 1.0f * rotation, 0.0f * rotation);
		//	break;
		//}
		//case '/':
		//{
		//	// Rotate negatively in the Y-axis
		//	float rotation = 120.f * (1.0f + lSystemParams.variance / 100.f + randomRotations[i % 5]);
		//	//world_ *= XMMatrixRotationRollPitchYaw(0 * rotation, -1.0f * rotation, 0.0f * rotation);
		//	break;
		//}
		case '[':
		{
			// Save where we are
			SavedTransform s = { pos, rot.z, world_ };
			savedTransforms.push(s);
			break;
		}
		case ']':
			SavedTransform savedTransform = savedTransforms.top();
			world_ *= XMMatrixTranslation(pos.x, pos.y, pos.z);
			world_ *= XMMatrixRotationRollPitchYaw(rot.x, rot.y, rot.z);
			worlds.push_back(world_);
			world_ = savedTransform.world;
			pos = savedTransform.position;
			rot.z = savedTransform.rotation;
			if (currentPath[i - 1] == 'X' || currentPath[i - 2] == 'F' && currentPath[i - 1] == 'X')
			{
				isLeaf = true;
				lSystemParams.width = lSystemParams.minLeafLength;
			}
			else
			{
				isLeaf = false;
				lSystemParams.width = lSystemParams.minBranchLength;
			}
			quadVector.push_back(new RiverQuad(device, deviceContext, 0.03125f, lSystemParams.width));
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
