// L-System class
// Generates a river
#pragma once

#include "DXF.h"
#include <map>
#include <stack>

class LSystem
{
public:
	LSystem();
	~LSystem();

	struct SavedTransform
	{
		XMFLOAT3 position;
		float rotation;
	};

	int iterations;
	float angle;
	float width;
	float minLeafLength;
	float maxLeafLength;
	float minBranchLength;
	float maxBranchLength;
	float variance;

	void Awake();
	void Generate();
	float RandomIntRange(int min, int max);

private:
	const string axiom = "X";
	std::map<char, string> rules;
	std::stack<SavedTransform> savedTransforms;
	XMFLOAT3 initialPosition;

	std::string currentPath = "";
	float randomRotations[5];

};

