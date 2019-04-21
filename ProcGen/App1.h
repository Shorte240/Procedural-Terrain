// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "Terrain.h"
#include "TerrainShader.h"
#include "LSystem.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	void PickRayVector(float mouseX, float mouseY, XMVECTOR& pickRayInWorldSpacePos, XMVECTOR& pickRayInWorldSpaceDir);
	void InteractWithTerrain();

	bool render();
	void gui();

private:
	TerrainShader* terrainShader;
	LSystem* lSystem;

	Terrain* terrain;

	Light* directionalLight;

	float displacementHeight;
	float bottomLeft, bottomRight, topLeft, topRight;
	float perlinFrequency, perlinScale;
	float fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves;
	int pickDiameter;
	bool picking;
	bool fBMRidged;
	int regionCount;
	bool currentCornerValues, setCornerValues, randomCornerValues;

	XMFLOAT3 mwP;
	float riverSystemPosition[3];
};

#endif