// Terrain class
// Generates a rectangle that can be manipulated
#pragma once

#include <d3d11.h>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <algorithm>
#include "DXF.h"
#include "SimplexNoise.h"

using namespace DirectX;

class Terrain : public BaseMesh
{
public:
	Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int _width, int _height, XMFLOAT2 quadScale);
	~Terrain();

	virtual void sendData(ID3D11DeviceContext* deviceContext, D3D11_PRIMITIVE_TOPOLOGY top = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	void GeneratePlane(ID3D11Device* device);
	void GenerateHeightMap(ID3D11Device* device);
	void SmoothVertices(ID3D11Device* device);
	void Faulting(ID3D11Device* device, float displacement);
	void CircleAlgorithm(ID3D11Device* device, float displacement);
	void CircleAlgorithm(ID3D11Device* device, float displacement, XMFLOAT3& point, int diameter);
	void MidpointDisplacement(ID3D11Device* device, float displacement, float bottomLeftCornerValue, float bottomRightCornerValue, float topLeftCornerValue, float topRightCornerValue, bool currentCornerValues, bool setCornerValues, bool randomCornerValues);
	void SimplexNoiseFunction(ID3D11Device * device, float frequency, float scale);
	void FractalBrownianMotion(ID3D11Device * device, float frequency_, float gain_, float amplitude_, float lacunarity_, int octaves_, float scale_, bool ridged);
	void Voronoi(ID3D11Device* device, int regionCount);
	void Pick(ID3D11Device * device, XMVECTOR pickRayInWorldSpacePos, XMVECTOR pickRayInWorldSpaceDir, float displacement, int diameter);

private:
	void initBuffers(ID3D11Device* device);
	void CalculateNormals();
	float RandomIntRange(int min, int max);
	float jitter(XMFLOAT3& point, float d);
	int midpoint(int index, int index2);
	float average2(XMFLOAT3& point, XMFLOAT3& point2);
	float average4(XMFLOAT3& point, XMFLOAT3& point2, XMFLOAT3& point3, XMFLOAT3& point4);
	void mpdDisplace(int lx, int rx, int by, int ty, float spread);
	bool PointInTriangle(XMVECTOR& triV1, XMVECTOR& triV2, XMVECTOR& triV3, XMVECTOR& point);

	int width, height;

	VertexType* vertices;
	unsigned long* indices2;
	XMFLOAT2 size_;

	SimplexNoise simplexNoise;
};

