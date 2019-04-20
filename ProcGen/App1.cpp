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
	lSystem = new LSystem(renderer->getDevice(), hwnd);

	terrainShader = new TerrainShader(renderer->getDevice(), hwnd);

	textureMgr->loadTexture("Grass", L"../res/grass.png");
	textureMgr->loadTexture("Slope", L"../res/slope.png");
	textureMgr->loadTexture("Rock", L"../res/rock.png");
	textureMgr->loadTexture("Water", L"../res/water.jpg");

	directionalLight = new Light;
	directionalLight->setAmbientColour(0.1f, 0.1f, 0.1f, 1.0f);
	directionalLight->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	directionalLight->setDirection(0.5f, -0.5f, 0.0f);

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
	XMVECTOR pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR pickRayInViewSpacePos = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

	float PRVecX, PRVecY, PRVecZ;

	// *** ---- MAY NEED TO CHANGE TO RENDERER'S PROJ MATRIX ----- *** //
	XMFLOAT4X4 camProj;
	XMStoreFloat4x4(&camProj, renderer->getProjectionMatrix());
	// *** ---- MAY NEED TO CHANGE TO RENDERER'S PROJ MATRIX ----- *** //

	//Transform 2D pick position on screen space to 3D ray in View space
	PRVecX = (((2.0f * mouseX) / sWidth) - 1) / camProj(0,0);
	PRVecY = -(((2.0f * mouseY) / sHeight) - 1) / camProj(1, 1);
	PRVecZ = 1.0f;	//View space's Z direction ranges from 0 to 1, so we set 1 since the ray goes "into" the screen

	pickRayInViewSpaceDir = XMVectorSet(PRVecX, PRVecY, PRVecZ, 0.0f);

	//Uncomment this line if you want to use the center of the screen (client area)
	//to be the point that creates the picking ray (eg. first person shooter)
	//pickRayInViewSpaceDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);

	// Transform 3D Ray from View space to 3D ray in World space
	XMMATRIX pickRayToWorldSpaceMatrix;
	XMVECTOR matInvDeter;	//We don't use this, but the xna matrix inverse function requires the first parameter to not be null

	pickRayToWorldSpaceMatrix = XMMatrixInverse(&matInvDeter, camera->getViewMatrix());	//Inverse of View Space matrix is World space matrix

	pickRayInWorldSpacePos = XMVector3TransformCoord(pickRayInViewSpacePos, pickRayToWorldSpaceMatrix);
	pickRayInWorldSpaceDir = XMVector3TransformNormal(pickRayInViewSpaceDir, pickRayToWorldSpaceMatrix);
}

void App1::InteractWithTerrain()
{
	if (picking)
	{
		if (input->isLeftMouseDown())
		{
			POINT mousePos;

			GetCursorPos(&mousePos);
			ScreenToClient(wnd, &mousePos);

			int mouseX = mousePos.x;
			int mouseY = mousePos.y;

			XMVECTOR prwsPos, prwsDir;
			PickRayVector(mouseX, mouseY, prwsPos, prwsDir);

			terrain->Pick(renderer->getDevice(), prwsPos, prwsDir, displacementHeight, pickDiameter);
			//input->setLeftMouse(false);
		}
		if (input->isRightMouseDown())
		{
			POINT mousePos;

			GetCursorPos(&mousePos);
			ScreenToClient(wnd, &mousePos);

			int mouseX = mousePos.x;
			int mouseY = mousePos.y;

			XMVECTOR prwsPos, prwsDir;
			PickRayVector(mouseX, mouseY, prwsPos, prwsDir);

			terrain->Pick(renderer->getDevice(), prwsPos, prwsDir, -displacementHeight, pickDiameter);
			//input->setRightMouse(false);
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

	// Send geometry data, set shader parameters, render object with shader
	terrain->sendData(renderer->getDeviceContext());
	terrainShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture("Grass"), textureMgr->getTexture("Slope"), textureMgr->getTexture("Rock"), directionalLight);
	terrainShader->render(renderer->getDeviceContext(), terrain->getIndexCount());

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
			ImGui::Checkbox("Ridged", &fBMRidged);
			ImGui::TreePop();
		}

		if (ImGui::Button("fBM")) 
		{
			terrain->FractalBrownianMotion(renderer->getDevice(), fBMFrequency, fBMGain, fBMAmplitude, fBMLacunarity, fBMOctaves, fBMRidged);
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

	if (ImGui::TreeNode("Picking"))
	{
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

		ImGui::Checkbox("Picking Mode", &picking);

		ImGui::TreePop();
	}

	if (ImGui::TreeNode("L-Systems"))
	{
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

		lSystem->waveParams.elapsedTime += timer->getTime();
		if (ImGui::TreeNode("Wave Variables"))
		{
			ImGui::SliderFloat("Wave Height", &lSystem->waveParams.height, 0, 20);
			ImGui::SliderFloat("Wave Frequency", &lSystem->waveParams.frequency, 0, 15);
			ImGui::SliderFloat("Wave Speed", &lSystem->waveParams.speed, 0, 5);
			ImGui::TreePop();
		}

		if (ImGui::Button("L-System"))
		{
			XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
			worldMatrix = renderer->getWorldMatrix();
			viewMatrix = camera->getViewMatrix();
			projectionMatrix = renderer->getProjectionMatrix();
			lSystem->Generate(renderer->getDevice(), renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, XMFLOAT3(riverSystemPosition[0], riverSystemPosition[1], riverSystemPosition[2]));
		}

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

