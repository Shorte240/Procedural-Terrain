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
	// Shaders
	TerrainShader* terrainShader;

	// Meshes
	Terrain* terrain;
	LSystem* lSystem;

	// Lights
	Light* directionalLight;

	// Displacement height for most terrain techniques
	float displacementHeight;
	// Corner values for midpoint displacement of the terrain
	float bottomLeft, bottomRight, topLeft, topRight;
	// Values for simplex noise of the terrain
	float perlinFrequency, perlinScale;
	// Values for fractal brownian motion of the terrain
	float fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves;
	// Diameter for picking function of the terrain
	int pickDiameter;
	// Toggle for toggling picking on the terrain
	bool picking;
	// Toggle for generating ridges on the terrain
	bool fBMRidged;
	// Region count for voronoi function on the terrain
	int regionCount;
	// Toggles for different options for corner values 
	// for midpoint displacement of the terrain
	bool currentCornerValues, setCornerValues, randomCornerValues;

	// Position of the l-system in the world
	float riverSystemPosition[3];
};

#endif