// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create Mesh object and shader object
	terrain = new Terrain(renderer->getDevice(), renderer->getDeviceContext(), 129, 129, XMFLOAT2(0.5,0.5));
	
	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);

	textureMgr->loadTexture("Grass", L"../res/grass.png");
	textureMgr->loadTexture("Slope", L"../res/slope.png");
	textureMgr->loadTexture("Rock", L"../res/rock.png");

	directionalLight = new Light;
	directionalLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight->setDiffuseColour(0.6f, 0.6f, 0.6f, 1.0f);
	directionalLight->setDirection(-1.0f, -0.7f, 0.0f);

	displacementHeight = 1.0f;
	bottomLeft = 25;
	bottomRight = 30;
	topLeft = 100;
	topRight = 0;

	perlinFrequency = 1.0f;
	perlinScale = 1.0f;

	fBMFrequency = 1.0f;
	fBMGain = 0.5f;
	fBMAmplitude = 1.0f;
	fBMLacunarity = 2.0f;
	fBMOctaves = 10;
	fBMScale = 27.6;

	regionCount = 5;

	currentCornerValues = true;
	setCornerValues = false;
	randomCornerValues = false;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (terrain)
	{
		delete terrain;
		terrain = 0;
	}

	if (terrainShader)
	{
		delete terrainShader;
		terrainShader = 0;
	}
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	// Send geometry data, set shader parameters, render object with shader
	terrain->sendData(renderer->getDeviceContext());
	terrainShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("Grass"), textureMgr->getTexture("Slope"), textureMgr->getTexture("Rock"), directionalLight);
	terrainShader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);

	ImGui::InputFloat("Displacement Value", &displacementHeight);

	if (ImGui::Button("Flat Plane")) {
		terrain->GeneratePlane(renderer->getDevice());
	}
	if (ImGui::Button("Generate Height")) {
		terrain->GenerateHeightMap(renderer->getDevice());
	}
	if (ImGui::Button("Smooth Terrain")) {
		terrain->SmoothVertices(renderer->getDevice());
	}
	if (ImGui::Button("Faulting")) {
		terrain->Faulting(renderer->getDevice(), displacementHeight);
	}
	if (ImGui::Button("Circle Algorithm")) {
		terrain->CircleAlgorithm(renderer->getDevice(), displacementHeight);
	}
	if (ImGui::TreeNode("Midpoint Displacement"))
	{
		ImGui::Checkbox("Current Corner Values", &currentCornerValues);
		ImGui::Checkbox("Set Corner Values", &setCornerValues);
		ImGui::Checkbox("Random Corner Values", &randomCornerValues);
		if (setCornerValues)
		{
			if (ImGui::TreeNode("Corner Values"))
			{
				ImGui::InputFloat("Bottom Left", &bottomLeft);
				ImGui::InputFloat("Bottom Right", &bottomRight);
				ImGui::InputFloat("Top Left", &topLeft);
				ImGui::InputFloat("Top Right", &topRight);
				ImGui::TreePop();
			}
		}

		if (ImGui::Button("Midpoint Displacement")) 
		{
			terrain->MidpointDisplacement(renderer->getDevice(), displacementHeight, bottomLeft, bottomRight, topLeft, topRight, currentCornerValues, setCornerValues, randomCornerValues);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Simplex Noise"))
	{
		if (ImGui::TreeNode("Frequency & Scale"))
		{
			ImGui::InputFloat("Frequency", &perlinFrequency);
			ImGui::InputFloat("Scale", &perlinScale);
			ImGui::TreePop();
		}

		if (ImGui::Button("Simplex Noise"))
		{
			terrain->SimplexNoiseFunction(renderer->getDevice(), perlinFrequency, perlinScale);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("fBM"))
	{
		if (ImGui::TreeNode("Values"))
		{
			ImGui::InputFloat("Frequency", &fBMFrequency);
			ImGui::InputFloat("Gain", &fBMGain);
			ImGui::InputFloat("Amplitude", &fBMAmplitude);
			ImGui::InputFloat("Lacunarity", &fBMLacunarity);
			ImGui::InputFloat("Octaves", &fBMOctaves);
			ImGui::InputFloat("Scale", &fBMScale);
			ImGui::TreePop();
		}

		if (ImGui::Button("fBM")) 
		{
			terrain->FractalBrownianMotion(renderer->getDevice(), fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves, fBMScale);
		}

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Voronoi"))
	{
		if (ImGui::TreeNode("Region Count"))
		{
			ImGui::InputInt("Region Count", &regionCount);
			ImGui::TreePop();
		}

		if (ImGui::Button("Voronoi"))
		{
			terrain->Voronoi(renderer->getDevice(), regionCount);
		}

		ImGui::TreePop();
	}
	

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

