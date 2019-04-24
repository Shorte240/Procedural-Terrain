// Terrain.cpp
// Contains all the setup for the terrain class and functions to manipulate it
#include "Terrain.h"

Terrain::Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int _width, int _height, XMFLOAT2 quadScale)
{
	// Set width, height and scale of terrain
	width = _width;
	height = _height;
	scale_ = quadScale;

	// Set the amount of times the texture will repeat
	textureRepeatAmount = 32;

	// Vertex count is width and height
	vertexCount = height * width;

	// Initialise vertices using vertex count
	vertices = new VertexType[vertexCount];

	// Generate a flat plane
	GeneratePlane(device);
}

Terrain::~Terrain()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();

	// Delete the vertices array
	delete[] vertices;
	vertices = 0;

	// Delete the seconday indices array
	// used in terrain picking
	delete[] indices2;
	indices2 = 0;
}

void Terrain::initBuffers(ID3D11Device* device)
{
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	// index count calculation 
	// How many quads
	int quad_x = width - 1;
	int quad_z = height - 1;
	// Total quads
	int total_quads = quad_x * quad_z;
	// 6 verts per quad
	unsigned long* indices = new unsigned long[total_quads * 6];
	indices2 = new unsigned long[total_quads * 6];

	// Set the index count
	indexCount = total_quads * 6;

	// for all the quads to be rendered
	int quadIndex = 0;
	int vertRef = 0;
	int lineCheck = quad_x;
	// boolean to trick if tri has been flipped
	// to make the quilt patterened terrain
	bool flipped = false;
	for (int qz = 0; qz < quad_z; qz++)
	{
		for (int qx = 0; qx < quad_x; qx++)
		{
			if (flipped)
			{
				// Assigne the indices based on the current quad 
				indices[quadIndex] = vertRef;			// TopLeft
				indices[quadIndex + 1] = vertRef + width;		// BottomLeft
				indices[quadIndex + 2] = vertRef + width + 1;  // BottomRight

				indices[quadIndex + 3] = vertRef;			// TopLeft
				indices[quadIndex + 4] = vertRef + width + 1;  // BottomRight
				indices[quadIndex + 5] = vertRef + 1;		// TopRight

				// It's the quad combined with the number of verts wide
				vertRef++;

				if (vertRef%lineCheck == 0)
				{
					vertRef++;
					lineCheck += width;
				}

				// Move to the next quad
				quadIndex += 6;
			}
			else if (!flipped)
			{
				// Assigne the indices based on the current quad 
				indices[quadIndex] = vertRef;			// TopLeft
				indices[quadIndex + 1] = vertRef + width;		// BottomLeft
				indices[quadIndex + 2] = vertRef + 1;		// TopRight

				indices[quadIndex + 3] = vertRef + 1;		// TopRight
				indices[quadIndex + 4] = vertRef + width;		// BottomLeft
				indices[quadIndex + 5] = vertRef + width + 1;  // BottomRight
				
				// It's the quad combined with the number of verts wide
				vertRef++;

				if (vertRef%lineCheck == 0)
				{
					vertRef++;
					lineCheck += width;
				}

				// Move to the next quad
				quadIndex += 6;
			}
			flipped = !flipped;
			if (qx == quad_x - 1)
			{
				flipped = !flipped;
			}
		}
	}

	// Initialise secondary indices array for terrain picking
	for (int i = 0; i < indexCount; i++)
	{
		indices2[i] = indices[i];
	}

	// Create the buffers 
	D3D11_BUFFER_DESC vertexBufferDesc = { sizeof(VertexType) * vertexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vertexData = { vertices, 0 , 0 };
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc = { sizeof(unsigned long) * indexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	indexData = { indices, 0, 0 };
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] indices;
	indices = 0;
}

void Terrain::GeneratePlane(ID3D11Device* device)
{
	// index count calculation 
	// How many quads
	int quad_x = width - 1;
	int quad_z = height - 1;

	// Position offsets
	XMFLOAT3 topLeft = XMFLOAT3(-(float)quad_x*scale_.x / 2.0f, 0.0f, (float)quad_z*scale_.y / 2.0f);

	// Create all the vers based on the top left position 
	int index = 0;

	for (int l = 0; l < height; l++)
	{
		for (int w = 0; w < width; w++)
		{
			VertexType vert;

			// Set position and normal
			vert.position = XMFLOAT3(topLeft.x + ((float)w * scale_.x), 0.0f, topLeft.z - ((float)l * scale_.y));
			vert.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			// Calculate the texture coordinate
			//vert.texture = XMFLOAT2(vert.position.x, vert.position.z);
			vert.texture = XMFLOAT2((float)w * textureRepeatAmount / (float)width, (float)l * textureRepeatAmount / (float)height);

			// Store the vert position 
			vertices[index] = vert;

			// keep track of where in the array
			index++;
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::sendData(ID3D11DeviceContext* deviceContext, D3D11_PRIMITIVE_TOPOLOGY top)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	deviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	deviceContext->IASetPrimitiveTopology(top);
}

void Terrain::GenerateHeightMap(ID3D11Device * device)
{
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			// Calculate index
			int index = ((width) * y) + x;
			// Calculate new random y-pos
			float newYPos = RandomIntRange(6, 12);

			// Set the new y-pos
			vertices[index].position.y += newYPos;
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::SmoothVertices(ID3D11Device * device)
{
	// Index of local height map
	int index;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			int adjacentSections = 0;
			float sectionsTotal = 0.0f;
			index = (height * x) + y;

			if ((x - 1) > 0) // Check to left
			{
				sectionsTotal += vertices[x - 1].position.y;
				adjacentSections++;

				if ((y - 1) > 0) // Check up and to the left
				{
					sectionsTotal += vertices[y - 1].position.y;
					adjacentSections++;
				}

				if ((y + 1) < height) // Check down and to the left
				{
					sectionsTotal += vertices[y + 1].position.y;
					adjacentSections++;
				}
			}

			if ((x + 1) < width) // Check to right
			{
				sectionsTotal += vertices[x + 1].position.y;
				adjacentSections++;

				if ((y - 1) > 0) // Check up and to the right
				{
					sectionsTotal += vertices[y - 1].position.y;
					adjacentSections++;
				}

				if ((y + 1) < height) // Check down and to the right
				{
					sectionsTotal += vertices[y + 1].position.y;
					adjacentSections++;
				}
			}

			if ((y - 1) > 0) // Check above
			{
				sectionsTotal += vertices[y - 1].position.y;
				adjacentSections++;
			}

			if ((y + 1) < height) // Check below
			{
				sectionsTotal += vertices[y + 1].position.y;
				adjacentSections++;
			}

			// Smoothe the y-pos based on calculated sections and ajacent sections
			vertices[index].position.x = vertices[index].position.x;
			vertices[index].position.y = (vertices[index].position.y + (sectionsTotal / adjacentSections)) * 0.5f;
			vertices[index].position.z = vertices[index].position.z;
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::Faulting(ID3D11Device * device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		// Calculate random number
		float randomNumber = std::rand();
		// Calculate sin/cos of random number
		float randomSin = sin(randomNumber);
		float randomCos = cos(randomNumber);
		// Calculate distance of terrain
		float distance = sqrt((width * width) + height * height);
		// rand() / RAND_MAX gives a random number between 0 & 1
		// therefore length will be a random number between -distance/2 and distance/2
		float length = ((float)std::rand() / RAND_MAX) * distance - distance / 2.0f;

		for (int z = 0; z < height; z++)
		{
			for (int x = 0; x < width; x++)
			{
				int index = ((width)* z) + x;;

				// If point is on one side of the dividing line
				if ((randomSin * z) + (randomCos * x) - length > 0)
				{
					// Increase height
					vertices[index].position.y += displacement;
				}
				else
				{
					// Decrease height
					vertices[index].position.y -= displacement;
				}
			}
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::RandomCircleAlgorithm(ID3D11Device * device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		// Calculate random x and z positions
		// Calculate random circle size
		int randX = std::rand() % (width + 1);
		int randZ = std::rand() % (height + 1);
		int randCircSize = std::rand() % ((width + height) / 10); // circle diameter

		for (int z = 0; z < height; z++)
		{
			for (int x = 0; x < width; x++)
			{
				// Calculate index
				int index = ((width)* z) + x;

				// pd = distFromCircle*2/size
				float pd = sqrt((randX - x)*(randX - x) + (randZ - z)*(randZ - z)) * 2.0f / randCircSize;

				// If vertex in circle, displace upwards
				if (fabs(pd) <= 1.0f)
				{
					// Calculate the height to displace by
					float diff = displacement / 2.0f + cos(pd*3.14)*displacement / 2.0f;

					// Increase y-pos of vertices
					vertices[index].position.y += diff;
				}
			}
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::CircleAlgorithm(ID3D11Device * device, float displacement, XMFLOAT3& point, int diameter)
{
	for (int z = 0; z < height; z++)
	{
		for (int x = 0; x < width; x++)
		{
			// Calculate index
			int index = ((width)* z) + x;

			// pd = (distance between vertex position and clicked point)*2/size
			float pd = sqrt((point.x - vertices[index].position.x)*(point.x - vertices[index].position.x) + (point.z - vertices[index].position.z)*(point.z - vertices[index].position.z)) * 2.0f / diameter;	// pd = distFromCircle*2/size
			
			// if vertex in circle, displace upwards
			if (fabs(pd) <= 1.0f)
			{
				// Calculate the height to displace by
				float diff = displacement / 2.0f + cos(pd*3.14)*displacement / 2.0f;

				// Increase y-pos of vertices
				vertices[index].position.y += diff;
			}
		}
	}
}

// Midpoint displacement functions
// Adapted from http://stevelosh.com/blog/2016/02/midpoint-displacement/
float Terrain::jitter(XMFLOAT3 & point, float d)
{
	return point.y += RandomIntRange(0, d);
}

int Terrain::midpoint(int index, int index2)
{
	return ((index + index2) / 2);
}

float Terrain::average2(XMFLOAT3 & point, XMFLOAT3 & point2)
{
	return ((point.y + point2.y) / 2.0f);
}

float Terrain::average4(XMFLOAT3 & point, XMFLOAT3 & point2, XMFLOAT3 & point3, XMFLOAT3 & point4)
{
	return ((point.y + point2.y + point3.y + point4.y) / 4.0f);
}

void Terrain::mpdDisplace(int leftX, int rightX, int bottomY, int topY, float spread)
{
	// Functions adapted from http://stevelosh.com/blog/2016/02/midpoint-displacement/

	// lx: x co-ord for left-hand corners
	// rx: x co-ord for right-hand corners
	// by: y co-ord for bottom corners
	// ty: y co-ord for top corners

	// Centre position
	int centreX, centerY;

	// Calculate centre midpoint
	centreX = midpoint(leftX, rightX);
	centerY = midpoint(bottomY, topY);

	// Midpoint of the midpoint x
	int centerMidpointX;

	// Calculate midpoint of the midpoint x
	centerMidpointX = midpoint(centreX, centerY);

	// Get the bottom left, bottom right, top left and top right vertex positions
	XMFLOAT3 bottomLeft, bottomRight, topLeft, topRight;
	bottomLeft = vertices[leftX + (bottomY * width)].position;
	bottomRight = vertices[rightX + (bottomY * width)].position;
	topLeft = vertices[leftX + (topY * width)].position;
	topRight = vertices[rightX + (topY * width)].position;

	// Average all the heights of the positions
	XMFLOAT3 top, left, bottom, right, centre;
	top.y = average2(topLeft, topRight);
	left.y = average2(bottomLeft, topLeft);
	bottom.y = average2(bottomLeft, bottomRight);
	right.y = average2(bottomRight, topRight);
	centre.y = average4(top, left, bottom, right);

	// Set the centre y-pos
	vertices[centreX + (centerY*width)].position.y = jitter(centre, spread);

	// Set the left and right y-pos
	vertices[leftX + (centerY*width)].position.y = jitter(left, spread);
	vertices[rightX + (centerY*width)].position.y = jitter(right, spread);

	// Set the bottom and top y-pos
	vertices[centreX + (bottomY*width)].position.y = jitter(bottom, spread);
	vertices[centreX + (topY*width)].position.y = jitter(top, spread);
}

void Terrain::MidpointDisplacement(ID3D11Device * device, float displacement, float bottomLeftCornerValue, float bottomRightCornerValue, float topLeftCornerValue, float topRightCornerValue, bool currentCornerValues, bool setCornerValues, bool randomCornerValues)
{
	// Functions adapted from http://stevelosh.com/blog/2016/02/midpoint-displacement/

	float d = displacement;

	int leftX, rightX, bottomY, topY;

	// lx: x co-ord for left-hand corners
	// rx: x co-ord for right-hand corners
	// by: y co-ord for bottom corners
	// ty: y co-ord for top corners
	leftX = 0;
	rightX = width - 1;
	bottomY = width * height - width;
	topY = width * height - 1;

	if (currentCornerValues)
	{
		// Set the corners to random Y values
		vertices[leftX].position.y = vertices[leftX].position.y;// bottomLeftCornerValue;// RandomIntRange(0, d);
		// Bottom Right							 
		vertices[rightX].position.y = vertices[rightX].position.y;// bottomRightCornerValue;// RandomIntRange(0, d);
		// Top Left								 
		vertices[bottomY].position.y = vertices[bottomY].position.y;// topLeftCornerValue; //RandomIntRange(0, d);
		// Top Right							 
		vertices[topY].position.y = vertices[topY].position.y;// topRightCornerValue;// RandomIntRange(0, d); 
	}
	else if (setCornerValues)
	{
		// Set the corners to random Y values
		vertices[leftX].position.y = bottomLeftCornerValue;
		// Bottom Right							 
		vertices[rightX].position.y = bottomRightCornerValue;
		// Top Left					
		vertices[bottomY].position.y = topLeftCornerValue;
		// Top Right				
		vertices[topY].position.y = topRightCornerValue;
	}
	else if (randomCornerValues)
	{
		// Set the corners to random Y values
		vertices[leftX].position.y = RandomIntRange(0, d);
		// Bottom Right				
		vertices[rightX].position.y = RandomIntRange(0, d);
		// Top Left					
		vertices[bottomY].position.y = RandomIntRange(0, d);
		// Top Right				
		vertices[topY].position.y = RandomIntRange(0, d); 
	}

	// Calculate the exponent
	int exponent = log2(width - 1);

	float chunks;
	float chunkWidth;
	float tileWidth = width - 1;
	for (int i = 0; i < exponent; i++)
	{
		if (i < exponent)
		{
			// Calculate chunks and chunk width
			chunks = pow(2, i);
			chunkWidth = tileWidth / chunks;
			for (int x = 0; x < chunks; x++)
			{
				for (int y = 0; y < chunks; y++)
				{
					float leftX, rightX, bottomY, topY;
					leftX = x * chunkWidth;
					rightX = leftX + chunkWidth;
					bottomY = y * chunkWidth;
					topY = bottomY + chunkWidth;

					// Calculate the edges (midpoints of corners on each side)
					mpdDisplace(leftX, rightX, bottomY, topY, d);
				}
			}
			d *= 0.5f;
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::SimplexNoiseFunction(ID3D11Device * device, float frequency, float scale)
{
	// Functions adapted from http://staffwww.itn.liu.se/~stegu/simplexnoise/simplexnoise.pdf

	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			// Calculate index
			int index = ((width)* y) + x;
			
			// Calculate the simplex noise value
			double perlin = simplexNoise.noise(frequency * vertices[index].position.x, frequency * vertices[index].position.z);

			// Alter y-height dependant on returned value
			vertices[index].position.y += perlin * scale;
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::FractalBrownianMotion(ID3D11Device * device, float frequency_, float gain_, float amplitude_, float lacunarity_, int octaves_, bool ridged)
{
	// Initialise the values used for fBM
	double simplexValue = 0.0;
	float noiseHeight = 0;
	float frequency = frequency_;
	float gain = gain_;
	float amplitude = amplitude_;
	float lacunarity = lacunarity_;
	int octaves = octaves_;

	for (int i = 0; i < octaves; i++)
	{
		for (int y = 0; y < height; y++)
		{
			for (int x = 0; x < width; x++)
			{
				simplexValue = 0.0;
				noiseHeight = 0;

				// Calculate index
				int index = ((width)* y) + x;

				// Increase simplex value
				simplexValue += simplexNoise.noise(frequency * vertices[index].position.x, frequency * vertices[index].position.z);

				// If not ridged, just alter height
				if (!ridged)
				{
					noiseHeight += simplexValue * amplitude;
				}
				// If ridged, inverse the height
				else if (ridged)
				{
					noiseHeight = simplexValue * amplitude;
					noiseHeight = abs(noiseHeight);
					noiseHeight *= -1;
				}

				// Alter y-pos
				vertices[index].position.y += noiseHeight;
			}
		}
		// Alter amplitude and frequency
		amplitude *= gain;
		frequency *= lacunarity;
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::Voronoi(ID3D11Device * device, int regionCount)
{
	// Vector to hold random points around the terrain
	std::vector<XMFLOAT3> points;

	// Get set of random points across the plane
	for (int i = 0; i < regionCount; i++)
	{
		// Calculate random x,z pos
		int randX = std::rand() % (width + 1);
		int randZ = std::rand() % (height + 1);

		// Seet y = i
		int y = i;

		// Add point to points vector
		points.push_back(XMFLOAT3(randX, y, randZ));
	}

	// Vector to hold the distances
	std::vector<float> d;

	for (float z = 0; z < height; z++)
	{
		for (float x = 0; x < width; x++)
		{
			// Calculate index
			int index = ((width)* z) + x;

			// Check the distance between this point and set of points
			for (int i = 0; i < regionCount; i++)
			{
				float xf = (points[i].x - x);
				float zf = (points[i].z - z);
				float dist = sqrt((xf * xf) + (zf * zf));
				d.push_back(dist);
			}

			// Find the smallest distance
			std::vector<float>::iterator result = std::min_element(std::begin(d), std::end(d));
			
			// Alter y-pos based on distances
			vertices[index].position.y = points[std::distance(std::begin(d), result)].y;
			d.clear();
		}
	}

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

void Terrain::Pick(ID3D11Device * device, XMVECTOR pickRayInWorldSpacePos, XMVECTOR pickRayInWorldSpaceDir, float displacement, int diameter)
{
	// Functions adapted from https://www.braynzarsoft.net/viewtutorial/q16390-24-picking

	//Loop through each triangle in the object
	for (int i = 0; i < indexCount / 3; i++)
	{
		//Triangle's vertices V1, V2, V3
		XMVECTOR tri1V1 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR tri1V2 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR tri1V3 = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		//Temporary 3d floats for each vertex
		XMFLOAT3 tV1, tV2, tV3;

		//Get triangle 
		tV1 = vertices[indices2[(i*3) + 0]].position;
		tV2 = vertices[indices2[(i*3) + 1]].position;
		tV3 = vertices[indices2[(i*3) + 2]].position;

		tri1V1 = XMVectorSet(tV1.x, tV1.y, tV1.z, 0.0f);
		tri1V2 = XMVectorSet(tV2.x, tV2.y, tV2.z, 0.0f);
		tri1V3 = XMVectorSet(tV3.x, tV3.y, tV3.z, 0.0f);
		
		//Find the normal using U, V coordinates (two edges)
		XMVECTOR U = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR V = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		XMVECTOR faceNormal = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		U = tri1V2 - tri1V1;
		V = tri1V3 - tri1V1;

		//Compute face normal by crossing U, V
		faceNormal = XMVector3Cross(U, V);

		faceNormal = XMVector3Normalize(faceNormal);

		//Calculate a point on the triangle for the plane equation
		XMVECTOR triPoint = tri1V1;

		//Get plane equation ("Ax + By + Cz + D = 0") Variables
		float tri1A = XMVectorGetX(faceNormal);
		float tri1B = XMVectorGetY(faceNormal);
		float tri1C = XMVectorGetZ(faceNormal);
		float tri1D = (-tri1A * XMVectorGetX(triPoint) - tri1B * XMVectorGetY(triPoint) - tri1C * XMVectorGetZ(triPoint));

		//Now we find where (on the ray) the ray intersects with the triangles plane
		float ep1, ep2, t = 0.0f;
		float planeIntersectX, planeIntersectY, planeIntersectZ = 0.0f;
		XMVECTOR pointInPlane = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);

		ep1 = (XMVectorGetX(pickRayInWorldSpacePos) * tri1A) + (XMVectorGetY(pickRayInWorldSpacePos) * tri1B) + (XMVectorGetZ(pickRayInWorldSpacePos) * tri1C);
		ep2 = (XMVectorGetX(pickRayInWorldSpaceDir) * tri1A) + (XMVectorGetY(pickRayInWorldSpaceDir) * tri1B) + (XMVectorGetZ(pickRayInWorldSpaceDir) * tri1C);

		//Make sure there are no divide-by-zeros
		if (ep2 != 0.0f)
			t = -(ep1 + tri1D) / (ep2);

		if (t > 0.0f)    //Make sure you don't pick objects behind the camera
		{
			//Get the point on the plane
			planeIntersectX = XMVectorGetX(pickRayInWorldSpacePos) + XMVectorGetX(pickRayInWorldSpaceDir) * t;
			planeIntersectY = XMVectorGetY(pickRayInWorldSpacePos) + XMVectorGetY(pickRayInWorldSpaceDir) * t;
			planeIntersectZ = XMVectorGetZ(pickRayInWorldSpacePos) + XMVectorGetZ(pickRayInWorldSpaceDir) * t;

			pointInPlane = XMVectorSet(planeIntersectX, planeIntersectY, planeIntersectZ, 0.0f);

			//Call function to check if point is in the triangle
			if (PointInTriangle(tri1V1, tri1V2, tri1V3, pointInPlane))
			{
				// Store the clicked point in plane as float 3
				XMFLOAT3 p;
				XMStoreFloat3(&p, pointInPlane);

				// Do circle algorithm based on clicked point in plane
				CircleAlgorithm(device, displacement, p, diameter);
				break;
			}
		}
	}

	// Delete the secondary indices array
	delete[] indices2;
	indices2 = 0;

	// Calculate normals of the terrain
	CalculateNormals();

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	initBuffers(device);
}

bool Terrain::PointInTriangle(XMVECTOR & triV1, XMVECTOR & triV2, XMVECTOR & triV3, XMVECTOR & point)
{
	// Functions adapted from https://www.braynzarsoft.net/viewtutorial/q16390-24-picking

	//To find out if the point is inside the triangle, we will check to see if the point
	//is on the correct side of each of the triangles edges.

	// Calculate the centre points of all the tris and the given point
	XMVECTOR centrePoint = XMVector3Cross((triV3 - triV2), (point - triV2));
	XMVECTOR centrePoint2 = XMVector3Cross((triV3 - triV2), (triV1 - triV2));

	if (XMVectorGetX(XMVector3Dot(centrePoint, centrePoint2)) >= 0)
	{
		// Recalculate the centre points
		centrePoint = XMVector3Cross((triV3 - triV1), (point - triV1));
		centrePoint2 = XMVector3Cross((triV3 - triV1), (triV2 - triV1));

		if (XMVectorGetX(XMVector3Dot(centrePoint, centrePoint2)) >= 0)
		{
			// Recalculate the centre points
			centrePoint = XMVector3Cross((triV2 - triV1), (point - triV1));
			centrePoint2 = XMVector3Cross((triV2 - triV1), (triV3 - triV1));

			if (XMVectorGetX(XMVector3Dot(centrePoint, centrePoint2)) >= 0)
			{
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

void Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	XMFLOAT3* normals;

	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new XMFLOAT3[(height - 1) * (width - 1)];

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j<(height - 1); j++)
	{
		for (i = 0; i<(width - 1); i++)
		{
			index1 = (j * height) + i;
			index2 = (j * height) + (i + 1);
			index3 = ((j + 1) * height) + i;

			// Get three vertices from the face.
			vertex1[0] = vertices[index1].position.x;
			vertex1[1] = vertices[index1].position.y;
			vertex1[2] = vertices[index1].position.z;

			vertex2[0] = vertices[index2].position.x;
			vertex2[1] = vertices[index2].position.y;
			vertex2[2] = vertices[index2].position.z;

			vertex3[0] = vertices[index3].position.x;
			vertex3[1] = vertices[index3].position.y;
			vertex3[2] = vertices[index3].position.z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (height - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for (j = 0; j<height; j++)
	{
		for (i = 0; i<width; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (height - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if ((i < (width - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (height - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (height - 1)))
			{
				index = (j * (height - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if ((i < (width - 1)) && (j < (height - 1)))
			{
				index = (j * (height - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * height) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			vertices[index].normal.x = (-1.0f) * (sum[0] / length);
			vertices[index].normal.y = (-1.0f) * (sum[1] / length);
			vertices[index].normal.z = (-1.0f) * (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;
}

float Terrain::RandomIntRange(int min, int max)
{
	return (min + (std::rand() % (max - min + 1)));
}

