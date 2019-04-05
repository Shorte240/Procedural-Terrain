// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "ColourShader.h"
#include "ColourTriangle.h"
#include "ColourRect.h"
#include "TessellatedPlane.h"
#include "TessellationShader.h"
#include "Terrain.h"
#include "TerrainShader.h"

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

	Terrain* terrain;

	Light* directionalLight;
	float displacementHeight;
	float bottomLeft, bottomRight, topLeft, topRight;
	float perlinFrequency, perlinScale;
	float fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves, fBMScale;
	int pickDiameter;
	bool fBMRidged;
	int regionCount;
	bool currentCornerValues, setCornerValues, randomCornerValues;
};

#endif