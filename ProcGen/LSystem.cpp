#include "LSystem.h"



LSystem::LSystem()
{
}


LSystem::~LSystem()
{
}

void LSystem::Awake()
{
	for (int i = 0; i < 5; i++)
	{
		randomRotations[i] = RandomIntRange(-1, 1);
	}

	rules.insert(std::pair<char, string>('X', "[-FX][+FX][FX]"));
	rules.insert(std::pair<char, string>('F', "FF"));

	Generate();
}

void LSystem::Generate()
{
	currentPath = axiom;

	string stringBuilder;

	for (int i = 0; i < iterations; i++)
	{
		string currentPathChars = currentPath;

		for (int j = 0; j < currentPathChars.length(); j++)
		{
			//stringBuilder.append(rules.contains(currentPath[j]) ? rules[currentPathChars[j]] : currentPathChars[j].ToString());
		}

		// currentPath = stringBuilder.ToString();
		// stringBuilder = new StringBuilder();
	}

	for (int i = 0; i < currentPath.length(); i++)
	{
		switch (currentPath[i])
		{
		case 'F':
		{
			//initialPosition = transform.position;
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
			}

			//currentElement.transform.SetParent(tree.transform);

			//TreeElement currentTreeElement = currentElement.GetComponent<TreeElement>();

			if (isLeaf)
			{
				//transform.Translate(Vector3.up * 2f * RandomIntRange(minLeafLength, maxLeafLength));
			}
			else
			{
				//transform.Translate(Vector3.up * 2f * RandomIntRange(minBranchLength, maxBranchLength));
			}

			//currentTreeElement.lineRenderer.startWidth = width;
			//currentTreeElement.lineRenderer.endWidth = width;
			//currentTreeElement.lineRenderer.sharedMaterial = currentTreeElement.material;
			break;
		}

		case 'X':
			break;

		case '+':
			//transform.Rotate(Vector3.forward * angle * (1f + variance / 100f + randomRotations[i % 5]));
			break;

		case '-':
			//transform.Rotate(Vector3.back * angle * (1f + variance / 100f + randomRotations[i % 5]));
			break;

		case '*':
			//transform.Rotate(Vector3.up * 120f * (1f + variance / 100f + randomRotations[i % 5]));
			break;

		case '/':
			//transform.Rotate(Vector3.down * 120f * (1f + variance / 100f + randomRotations[i % 5]));
			break;

		case '[':
			//transformStack.Push(new SavedTransform()
			//{
			//	position = transform.position;
			//	rotation = transform.rotation;
			//});
			break;

		case ']':
			//SavedTransform savedTransform = transformStack.Pop();

			//transform.position = savedTransform.position;
			//transform.rotation = savedTransform.rotation;
			break;

		default:
			break;
		}
	}
}

float LSystem::RandomIntRange(int min, int max)
{
	return (min + (std::rand() % (max - min + 1)));
}
