// App1.cpp
// Generates a terrain mesh which can be procedurally edited by the user
// Also generates an L-System river which can be moved around the world
#include "App1.h"

App1::App1()
{
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Create terrain and l-system meshes
	terrain = new Terrain(renderer->getDevice(), renderer->getDeviceContext(), 129, 129, XMFLOAT2(0.5,0.5));
	lSystem = new LSystem(renderer->getDevice(), hwnd);

	// Create terrain shader
	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);

	// Load in textures for the terrain and l-system
	textureMgr->loadTexture("Grass", L"../res/grass.png");
	textureMgr->loadTexture("Slope", L"../res/slope.png");
	textureMgr->loadTexture("Rock", L"../res/rock.png");
	textureMgr->loadTexture("Water", L"../res/water.jpg");

	// Initialise the directional light
	directionalLight = new Light;
	directionalLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight->setDirection(0.5f, -0.5f, 0.0f);

	// Give initial values for all the variables
	// used in the terrain manipulation functions
	displacementHeight = 1.0f;
	bottomLeft = 25;
	bottomRight = 30;
	topLeft = 100;
	topRight = 0;

	perlinFrequency = 0.135f;
	perlinScale = 1.0f;

	fBMFrequency = 10.0f;
	fBMGain = 0.25f;
	fBMAmplitude = 2.0f;
	fBMLacunarity = 20.0f;
	fBMOctaves = 10.0f;
	fBMRidged = false;

	pickDiameter = 9;
	picking = false;

	regionCount = 5;

	currentCornerValues = true;
	setCornerValues = false;
	randomCornerValues = false;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the terrain object.
	if (terrain)
	{
		delete terrain;
		terrain = 0;
	}

	// Release the l-system object.
	if (lSystem)
	{
		delete lSystem;
		lSystem = 0;
	}

	// Release the light object.
	if (directionalLight)
	{
		delete directionalLight;
		directionalLight = 0;
	}

	// Release the terrain shader.
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

	// Use the mouse to alter terrain height
	InteractWithTerrain();
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

void App1::PickRayVector(float mouseX, float mouseY, XMVECTOR & pickRayInWorldSpacePos, XMVECTOR & pickRayInWorldSpaceDir)
{
	// Initialise vectors for later calulation
	XMVECTOR pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR pickRayInViewSpacePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	// Vector position
	float PRVecX, PRVecY, PRVecZ;

	// Matrix for camera projection
	XMFLOAT4X4 camProj;
	XMStoreFloat4x4(&camProj, renderer->getProjectionMatrix());

	//Transform 2D pick position on screen space to 3D ray in View space
	PRVecX = (((2.0f * mouseX) / sWidth) - 1) / camProj(0,0);
	PRVecY = -(((2.0f * mouseY) / sHeight) - 1) / camProj(1, 1);
	PRVecZ = 1.0f;	//View space's Z direction ranges from 0 to 1, so we set 1 since the ray goes "into" the screen

	// Calculate the ray in view space
	pickRayInViewSpaceDir = XMVectorSet(PRVecX, PRVecY, PRVecZ, 0.0f);

	// Transform 3D Ray from View space to 3D ray in World space
	XMMATRIX pickRayToWorldSpaceMatrix;
	XMVECTOR matInvDeter;	//We don't use this, but the xna matrix inverse function requires the first parameter to not be null

	// Calulate the ray in world space
	pickRayToWorldSpaceMatrix = XMMatrixInverse(&matInvDeter, camera->getViewMatrix());	//Inverse of View Space matrix is World space matrix

	// Transform the rays
	pickRayInWorldSpacePos = XMVector3TransformCoord(pickRayInViewSpacePos, pickRayToWorldSpaceMatrix);
	pickRayInWorldSpaceDir = XMVector3TransformNormal(pickRayInViewSpaceDir, pickRayToWorldSpaceMatrix);
}

void App1::InteractWithTerrain()
{
	// Picking enabled, alter terrain
	if (picking)
	{
		// Increase the height of the terrain
		if (input->isLeftMouseDown())
		{
			// Mouse co-ords in screen co-ordinates
			POINT mousePos;

			// Get the current mouse position in screen co-ordinates
			// and convert it to world co-ordinates
			GetCursorPos(&mousePos);
			ScreenToClient(wnd, &mousePos);

			// Get the mouse position in world co-ords
			int mouseX = mousePos.x;
			int mouseY = mousePos.y;

			// Vectors to store the pick ray in world space's position and direction
			XMVECTOR prwsPos, prwsDir;

			// Calculate the pick ray vector
			PickRayVector(mouseX, mouseY, prwsPos, prwsDir);

			// Alter the terrain using the pick ray
			terrain->Pick(renderer->getDevice(), prwsPos, prwsDir, displacementHeight, pickDiameter);
		}
		// Decrease the height of the terrain
		if (input->isRightMouseDown())
		{
			// Mouse co-ords in screen co-ordinates
			POINT mousePos;

			// Get the current mouse position in screen co-ordinates
			// and convert it to world co-ordinates
			GetCursorPos(&mousePos);
			ScreenToClient(wnd, &mousePos);

			// Get the mouse position in world co-ords
			int mouseX = mousePos.x;
			int mouseY = mousePos.y;

			// Vectors to store the pick ray in world space's position and direction
			XMVECTOR prwsPos, prwsDir;

			// Calculate the pick ray vector
			PickRayVector(mouseX, mouseY, prwsPos, prwsDir);

			// Alter the terrain using the pick ray
			terrain->Pick(renderer->getDevice(), prwsPos, prwsDir, -displacementHeight, pickDiameter);
		}
	}
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

	// Send geometry data, set shader parameters, render terrain with shader
	terrain->sendData(renderer->getDeviceContext());
	terrainShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("Grass"), textureMgr->getTexture("Slope"), textureMgr->getTexture("Rock"), directionalLight);
	terrainShader->render(renderer->getDeviceContext(), terrain->getIndexCount());

	// Render the l-system
	lSystem->Render(renderer->getDeviceContext(), viewMatrix, projectionMatrix, textureMgr->getTexture("Water"), directionalLight);

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

	// Displacement value used for majority of terrain displacement
	ImGui::InputFloat("Displacement Value", &displacementHeight);

	// Generate a flat plane
	if (ImGui::Button("Flat Plane")) 
	{
		terrain->GeneratePlane(renderer->getDevice());
	}

	// Generate a random heighted terrain
	if (ImGui::Button("Generate Height")) 
	{
		terrain->GenerateHeightMap(renderer->getDevice());
	}

	// Smoothe the terrain
	if (ImGui::Button("Smooth Terrain")) 
	{
		terrain->SmoothVertices(renderer->getDevice());
	}

	// Perform faulting on the terrain
	if (ImGui::Button("Faulting")) 
	{
		terrain->Faulting(renderer->getDevice(), displacementHeight);
	}

	// Perform the circle algorithm for height displacement on the terrain
	if (ImGui::Button("Circle Algorithm")) 
	{
		terrain->CircleAlgorithm(renderer->getDevice(), displacementHeight);
	}

	// Displace the terrain height via midpoint displacement
	if (ImGui::TreeNode("Midpoint Displacement"))
	{
		// Toggles to choose between what corner values are used
		ImGui::Checkbox("Current Corner Values", &currentCornerValues);
		ImGui::Checkbox("Set Corner Values", &setCornerValues);
		ImGui::Checkbox("Random Corner Values", &randomCornerValues);

		// If the user wants to enter values for the corner heights
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

		// Displace the terrain via midpoint displacement
		if (ImGui::Button("Midpoint Displacement")) 
		{
			terrain->MidpointDisplacement(renderer->getDevice(), displacementHeight, bottomLeft, bottomRight, topLeft, topRight, currentCornerValues, setCornerValues, randomCornerValues);
		}

		ImGui::TreePop();
	}

	// Alter the terrain via simplex noise
	if (ImGui::TreeNode("Simplex Noise"))
	{
		// Values for frequency & scale
		if (ImGui::TreeNode("Frequency & Scale"))
		{
			ImGui::InputFloat("Frequency", &perlinFrequency);
			ImGui::InputFloat("Scale", &perlinScale);
			ImGui::TreePop();
		}

		// Simplex noise function
		if (ImGui::Button("Simplex Noise"))
		{
			terrain->SimplexNoiseFunction(renderer->getDevice(), perlinFrequency, perlinScale);
		}

		ImGui::TreePop();
	}

	// Fractal Brownian Motion
	if (ImGui::TreeNode("fBM"))
	{
		// Values for fractal brownian motion
		if (ImGui::TreeNode("Values"))
		{
			ImGui::InputFloat("Frequency", &fBMFrequency);
			ImGui::InputFloat("Gain", &fBMGain);
			ImGui::InputFloat("Amplitude", &fBMAmplitude);
			ImGui::InputFloat("Lacunarity", &fBMLacunarity);
			ImGui::InputFloat("Octaves", &fBMOctaves);
			ImGui::Checkbox("Ridged", &fBMRidged);
			ImGui::TreePop();
		}

		// Fractal brownian motion function
		if (ImGui::Button("fBM")) 
		{
			terrain->FractalBrownianMotion(renderer->getDevice(), fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves, fBMRidged);
		}

		ImGui::TreePop();
	}

	// Voronoi region height displacement of the terrain
	if (ImGui::TreeNode("Voronoi"))
	{
		// Value for number of regions for the function to use
		if (ImGui::TreeNode("Region Count"))
		{
			ImGui::InputInt("Region Count", &regionCount);
			ImGui::TreePop();
		}

		// Voronoi region function
		if (ImGui::Button("Voronoi"))
		{
			terrain->Voronoi(renderer->getDevice(), regionCount);
		}

		ImGui::TreePop();
	}

	// Function which takes mouse click and displaces terrain
	if (ImGui::TreeNode("Picking"))
	{
		// Alter diameter either via ImGui interface
		// or +/- numpad keys
		// Scroll wheel implementation was wanted for this
		ImGui::InputInt("Pick Diameter", &pickDiameter);
		if (input->isKeyDown(VK_ADD))
		{
			pickDiameter++;
			input->SetKeyUp(VK_ADD);
		}
		else if (input->isKeyDown(VK_SUBTRACT))
		{
			pickDiameter--;
			input->SetKeyUp(VK_SUBTRACT);
		}

		// Toggle to enable/disable picking on the terrain
		ImGui::Checkbox("Picking Mode", &picking);

		ImGui::TreePop();
	}

	// L-System functions
	if (ImGui::TreeNode("L-Systems"))
	{
		// Variables for the l-system
		if (ImGui::TreeNode("Variables"))
		{
			ImGui::DragFloat3("Position", riverSystemPosition);
			ImGui::InputInt("Iterations", &lSystem->lSystemParams.iterations);
			ImGui::InputFloat("Angle", &lSystem->lSystemParams.angle);
			ImGui::InputFloat("Width", &lSystem->lSystemParams.width);
			ImGui::InputFloat("Height", &lSystem->lSystemParams.height);
			ImGui::InputFloat("X Scale", &lSystem->lSystemParams.scaleX);
			ImGui::InputFloat("Y Scale", &lSystem->lSystemParams.scaleY);
			ImGui::TreePop();
		}

		// Get the elapsed time to pass to manipulation shader
		// used for the river
		lSystem->waveParams.elapsedTime += timer->getTime();
		if (ImGui::TreeNode("Wave Variables"))
		{
			ImGui::SliderFloat("Wave Height", &lSystem->waveParams.height, 0, 20);
			ImGui::SliderFloat("Wave Frequency", &lSystem->waveParams.frequency, 0, 15);
			ImGui::SliderFloat("Wave Speed", &lSystem->waveParams.speed, 0, 5);
			ImGui::TreePop();
		}

		// Generate a rive l-system
		if (ImGui::Button("L-System"))
		{
			XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
			worldMatrix = renderer->getWorldMatrix();
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = renderer->getProjectionMatrix();
			lSystem->Generate(renderer->getDevice(), renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, XMFLOAT3(riverSystemPosition[0], riverSystemPosition[1], riverSystemPosition[2]));
		}

		// Clear the l-system, empty all vectors to stop it being rendered
		if (ImGui::Button("Clear System"))
		{
			lSystem->ClearSystem();
		}

		ImGui::TreePop();
	}
	
	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

