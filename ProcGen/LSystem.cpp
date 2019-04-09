// L-System cpp
// Generates a river
#include "LSystem.h"

LSystem::LSystem(ID3D11Device* device, HWND hwnd)
{
	iterations = 3;
	angle = 12.5f;
	width = 1.0f;
	minLeafLength = 0.5f;
	maxLeafLength = 1.0f;
	minBranchLength = 1.0f;
	maxBranchLength = 2.0f;
	variance = 2.0f;

	colourShader = new ColourShader(device, hwnd);
}

LSystem::~LSystem()
{
}

void LSystem::Awake(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3& position_, float &rot_, XMMATRIX& world_, XMMATRIX& view_, XMMATRIX& proj_)
{
	for (int i = 0; i < 5; i++)
	{
		randomRotations[i] = RandomIntRange(-1, 1);
	}

	rules.insert(std::pair<char, string>('X', "[-FX][+FX]"));
	rules.insert(std::pair<char, string>('F', "FF"));

	Generate(device, deviceContext, position_, rot_, world_, view_, proj_);
}

void LSystem::Generate(ID3D11Device* device, ID3D11DeviceContext* deviceContext, XMFLOAT3& position_, float &rot_, XMMATRIX& world_, XMMATRIX& view_, XMMATRIX& proj_)
{
	currentPath = axiom;


	for (int i = 0; i < iterations; i++)
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
			//std::replace(stringBuilder.begin(), stringBuilder.end(), currentPath[j], n[j]);
			stringBuilder.append(rules.find(currentPath[j]) != rules.end() ? rules[currentPathChars[j]] : n);
		}

		currentPath = stringBuilder;
	}

	for (int i = 0; i < currentPath.length(); i++)
	{
		switch (currentPath[i])
		{
		case 'F':
		{
			initialPosition = position_;
			bool isLeaf = false;

			//GameObject currentElement;
			if (currentPath[i + 1] % currentPath.length() == 'X' || currentPath[i + 3] % currentPath.length() == 'F' && currentPath[i + 4] % currentPath.length() == 'X')
			{
				//currentElement = Instantiate(leaf);
				isLeaf = true;
			}
			else
			{
				//currentElement = Instantiate(branch);
				isLeaf = false;
			}

			//currentElement.transform.SetParent(tree.transform);

			//TreeElement currentTreeElement = currentElement.GetComponent<TreeElement>();

			if (isLeaf)
			{
				//transform.Translate(Vector3.up * 2f * RandomIntRange(minLeafLength, maxLeafLength));
				float translation = 2.0f * RandomIntRange(minLeafLength, maxLeafLength);
				world_ *= XMMatrixTranslation(0 * translation, 1.0f * translation, 0.0f * translation);
			}
			else
			{
				//transform.Translate(Vector3.up * 2f * RandomIntRange(minBranchLength, maxBranchLength));
				float translation = 2.0f * RandomIntRange(minBranchLength, maxBranchLength);
				world_ *= XMMatrixTranslation(0 * translation, 1.0f * translation, 0.0f * translation);
			}

			//currentTreeElement.lineRenderer.startWidth = width;
			//currentTreeElement.lineRenderer.endWidth = width;
			//currentTreeElement.lineRenderer.sharedMaterial = currentTreeElement.material;
			break;
		}

		case 'X':
			break;

		case '+':
		{
			//transform.Rotate(Vector3.forward * angle * (1f + variance / 100f + randomRotations[i % 5]));
			float rotation = angle;// *(1.0f + variance / 100.0f + randomRotations[i % 5]);
			world_ *= XMMatrixRotationRollPitchYaw(0.0f * rotation, 0.0f * rotation, 1.0f * rotation);
			break;
		}
		case '-':
		{
			//transform.Rotate(Vector3.back * angle * (1f + variance / 100f + randomRotations[i % 5]));
			float rotation = angle;// *(1.0f + variance / 100.0f + randomRotations[i % 5]);
			world_ *= XMMatrixRotationRollPitchYaw(0.0f * rotation, 0.0f * rotation, -1.0f * rotation);
			break;
		}
		case '*':
		{
			//transform.Rotate(Vector3.up * 120f * (1f + variance / 100f + randomRotations[i % 5]));
			float rotation = 120.f * (1.0f + variance / 100.f + randomRotations[i % 5]);
			//world_ *= XMMatrixRotationRollPitchYaw(0 * rotation, 1.0f * rotation, 0.0f * rotation);
			break;
		}
		case '/':
		{
			//transform.Rotate(Vector3.down * 120f * (1f + variance / 100f + randomRotations[i % 5]));
			float rotation = 120.f * (1.0f + variance / 100.f + randomRotations[i % 5]);
			//world_ *= XMMatrixRotationRollPitchYaw(0 * rotation, -1.0f * rotation, 0.0f * rotation);
			break;
		}
		case '[':
		{
			SavedTransform s = { position_, rot_, world_ };
			savedTransforms.push(s);
			break;
		}
		case ']':
			SavedTransform savedTransform = savedTransforms.top();
			/*position_ = savedTransform.position;
			rot_ = savedTransform.rotation;*/
			worlds.push_back(world_);
			world_ = savedTransform.world;
			quadVector.push_back(new QuadMesh(device, deviceContext));
			//savedTransforms.pop();
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

float LSystem::RandomIntRange(int min, int max)
{
	return (min + (std::rand() % (max - min + 1)));
}
