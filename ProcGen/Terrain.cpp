// Terrain.cpp
// Contains all the setup for the terrain class and functions to manipulate it
#include "Terrain.h"

Terrain::Terrain(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int _width, int _height, XMFLOAT2 quadScale)
{
	width = _width;
	height = _height;
	size_ = quadScale;

	// Vertex count is width and height
	vertexCount = height * width;

	vertices = new VertexType[vertexCount];

	GeneratePlane(device);
}

Terrain::~Terrain()
{
	delete[] vertices;
	vertices = 0;
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
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

	indexCount = total_quads * 6;

	// for all the quads to be rendered
	int quadIndex = 0;
	int vertRef = 0;
	int lineCheck = quad_x;
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
	XMFLOAT3 topLeft = XMFLOAT3(-(float)quad_x*size_.x / 2.0f, 0.0f, (float)quad_z*size_.y / 2.0f);

	// Create all the vers based on the top left position 
	int index = 0;

	for (int l = 0; l < height; l++)
	{
		for (int w = 0; w < width; w++)
		{

			VertexType vert;

			vert.position = XMFLOAT3(topLeft.x + ((float)w * size_.x), 0.0f, topLeft.z - ((float)l * size_.y));
			vert.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			// Calculate the texture coordinate
			vert.texture = XMFLOAT2((float)w / width, (float)l / height);

			// Store the vert position 
			vertices[index] = vert;

			// keep track of where in the array
			index++;

		}
	}

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
			int vertexIdentity = ((width) * y) + x;
			float newYPos = RandomIntRange(6, 12);

			// Load the vertex array with data - setting the top left position of every patch
			vertices[vertexIdentity].position.y += newYPos;
		}
	}

	CalculateNormals();

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

			vertices[index].position.x = vertices[index].position.x;
			vertices[index].position.y = (vertices[index].position.y + (sectionsTotal / adjacentSections)) * 0.5f;
			vertices[index].position.z = vertices[index].position.z;
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void Terrain::Faulting(ID3D11Device * device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		float v = std::rand();
		float a = sin(v);
		float b = cos(v);
		float d = sqrt((width * width) + height * height);
		// rand() / RAND_MAX gives a random number between 0 & 1
		// therefore cwill be a random number between -d/2 and d/2
		float c = ((float)std::rand() / RAND_MAX) * d - d / 2.0f;

		for (int z = 0; z < height; z++)
		{
			for (int x = 0; x < width; x++)
			{
				int index;

				if ((a * z) + (b * x) - c > 0)
				{
					index = ((width) * z) + x;

					vertices[index].position.y += displacement;
				}
				else
				{
					index = ((width) * z) + x;

					vertices[index].position.y -= displacement;
				}
			}
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void Terrain::CircleAlgorithm(ID3D11Device * device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		int randX = std::rand() % (width + 1);
		int randZ = std::rand() % (height + 1);
		int randCircSize = std::rand() % ((width + height) / 10); // circle diameter

		for (int z = 0; z < height; z++)
		{
			for (int x = 0; x < width; x++)
			{
				int index;

				float pd = sqrt((randX - x)*(randX - x) + (randZ - z)*(randZ - z)) * 2.0f / randCircSize;	// pd = distFromCircle*2/size

				if (fabs(pd) <= 1.0f)	// if vertex in circle, displace upwards
				{
					index = ((width) * z) + x;

					float diff = displacement / 2.0f + cos(pd*3.14)*displacement / 2.0f;

					vertices[index].position.y += diff;
				}
			}
		}
	}

	CalculateNormals();

	initBuffers(device);
}

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

void Terrain::mpdDisplace(int lx, int rx, int by, int ty, float spread)
{
	// lx: x co-ord for left-hand corners
	// rx: x co-ord for right-hand corners
	// by: y co-ord for bottom corners
	// ty: y co-ord for top corners

	int cx, cy;

	cx = midpoint(lx, rx);
	cy = midpoint(by, ty);

	int cmx;

	cmx = midpoint(cx, cy);

	XMFLOAT3 bottomLeft, bottomRight, topLeft, topRight;
	bottomLeft = vertices[lx + (by * width)].position;
	bottomRight = vertices[rx + (by * width)].position;
	topLeft = vertices[lx + (ty * width)].position;
	topRight = vertices[rx + (ty * width)].position;

	XMFLOAT3 top, left, bottom, right, centre;
	top.y = average2(topLeft, topRight);
	left.y = average2(bottomLeft, topLeft);
	bottom.y = average2(bottomLeft, bottomRight);
	right.y = average2(bottomRight, topRight);
	centre.y = average4(top, left, bottom, right);

	vertices[cx + (cy*width)].position.y = jitter(centre, spread);

	vertices[lx + (cy*width)].position.y = jitter(left, spread);
	vertices[rx + (cy*width)].position.y = jitter(right, spread);

	vertices[cx + (by*width)].position.y = jitter(bottom, spread);
	vertices[cx + (ty*width)].position.y = jitter(top, spread);
}

void Terrain::MidpointDisplacement(ID3D11Device * device, float displacement, float bottomLeftCornerValue, float bottomRightCornerValue, float topLeftCornerValue, float topRightCornerValue, bool currentCornerValues, bool setCornerValues, bool randomCornerValues)
{
	float d = displacement;

	int lx, rx, by, ty;

	// lx: x co-ord for left-hand corners
	// rx: x co-ord for right-hand corners
	// by: y co-ord for bottom corners
	// ty: y co-ord for top corners
	lx = 0;
	rx = width - 1;
	by = width * height - width;
	ty = width * height - 1;

	if (currentCornerValues)
	{
		// Set the corners to random Y values
		vertices[lx].position.y = vertices[lx].position.y;// bottomLeftCornerValue;// RandomIntRange(0, d);
		// Bottom Right							 
		vertices[rx].position.y = vertices[rx].position.y;// bottomRightCornerValue;// RandomIntRange(0, d);
		// Top Left								 
		vertices[by].position.y = vertices[by].position.y;// topLeftCornerValue; //RandomIntRange(0, d);
		// Top Right							 
		vertices[ty].position.y = vertices[ty].position.y;// topRightCornerValue;// RandomIntRange(0, d); 
	}
	else if (setCornerValues)
	{
		// Set the corners to random Y values
		vertices[lx].position.y = bottomLeftCornerValue;
		// Bottom Right							 
		vertices[rx].position.y = bottomRightCornerValue;
		// Top Left					
		vertices[by].position.y = topLeftCornerValue;
		// Top Right				
		vertices[ty].position.y = topRightCornerValue;
	}
	else if (randomCornerValues)
	{
		// Set the corners to random Y values
		vertices[lx].position.y = RandomIntRange(0, d);
		// Bottom Right				
		vertices[rx].position.y = RandomIntRange(0, d);
		// Top Left					
		vertices[by].position.y = RandomIntRange(0, d);
		// Top Right				
		vertices[ty].position.y = RandomIntRange(0, d); 
	}

	int exponent = log2(width - 1);

	float chunks;
	float chunkWidth;
	float tileWidth = width - 1;
	for (int i = 0; i < exponent; i++)
	{
		if (i < exponent)
		{
			chunks = pow(2, i);
			//chunkWidth = chunks / (width);
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

	CalculateNormals();

	initBuffers(device);
}

void Terrain::SimplexNoiseFunction(ID3D11Device * device, float frequency, float scale)
{
	for (int y = 0; y < height; y++) 
	{
		for (int x = 0; x < width; x++) 
		{
			int index;
			index = ((width)* y) + x;
			double perlin = simplexNoise.noise(frequency * vertices[index].position.x, frequency * vertices[index].position.z);
			vertices[index].position.y += perlin * scale;
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void Terrain::FractalBrownianMotion(ID3D11Device * device, float frequency_, float gain_, float amplitude_, float lacunarity_, int octaves_, float scale_, bool ridged)
{
	//for each pixel, get the value total = 0.0f; frequency = 1.0f/(float)hgrid; amplitude = gain;
	
	for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{
			double simplexValue = 0.0;
			float noiseHeight = 0;
			float frequency = frequency_;
			float gain = gain_;
			float amplitude = amplitude_;
			float lacunarity = lacunarity_;
			int octaves = octaves_;
			float scale = scale_;

			int index;
			index = ((width)* y) + x;
			for (int i = 0; i < octaves; i++) 
			{
				float sampleX = x / scale * frequency;
				float sampleY = y / scale * frequency;

				simplexValue += simplexNoise.noise(sampleX, sampleY);
				if (!ridged)
				{
					noiseHeight += simplexValue * amplitude;
				}
				else if (ridged)
				{
					noiseHeight = simplexValue * amplitude;
					noiseHeight = abs(noiseHeight);
					noiseHeight *= -1;
				}

				amplitude *= gain;
				frequency *= lacunarity;
			}
			vertices[index].position.y += noiseHeight;
		}
	}

	/*for (int y = 0; y < height; y++)
	{
		for (int x = 0; x < width; x++)
		{

		}
	}*/

	//now that we have the value, put it in map[x][y]=total;

	CalculateNormals();

	initBuffers(device);
}

void Terrain::Voronoi(ID3D11Device * device, int regionCount)
{
	std::vector<XMFLOAT3> points;

	// Get set of random points across the plane
	for (int i = 0; i < regionCount; i++)
	{
		int randX = std::rand() % (width + 1);
		int y = i;
		int randZ = std::rand() % (height + 1);
		points.push_back(XMFLOAT3(randX, y, randZ));
	}

	std::vector<float> d;
	for (float z = 0; z < height; z++)
	{
		for (float x = 0; x < width; x++)
		{
			int index;

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

			index = ((width)* z) + x;

			vertices[index].position.y = points[std::distance(std::begin(d), result)].y;
			d.clear();
		}
	}

	CalculateNormals();

	initBuffers(device);
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
			vertices[index].normal.x = (-1) * (sum[0] / length);
			vertices[index].normal.y = (-1) * (sum[1] / length);
			vertices[index].normal.z = (-1) * (sum[2] / length);
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


