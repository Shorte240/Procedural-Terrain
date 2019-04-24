// SizeableQuad.cpp
// Used in generation of the l-system
#include "SizableQuad.h"

SizableQuad::SizableQuad(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float width_, float height_, XMFLOAT2 quadScale)
{
	// Set width, height and scale based on passed in parameters
	width = width_;
	height = height_;
	scale_ = quadScale;

	// Vertex count is width and height
	vertexCount = height * width;

	// Initialise vertices based on vertex count
	vertices = new VertexType[vertexCount];

	// Initialise the buffers
	initBuffers(device);
}

SizableQuad::~SizableQuad()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();

	delete[] vertices;
	vertices = 0;
}


void SizableQuad::initBuffers(ID3D11Device * device)
{
	// index count calculation 
	// How many quads
	int quad_x = width - 1;
	int quad_z = height - 1;

	// Position offsets
	XMFLOAT3 topLeft = XMFLOAT3(-(float)quad_x*scale_.x, 0.0f, (float)quad_z*scale_.y);

	// Create all the vers based on the top left position 
	int index = 0;

	for (int l = 0; l < height; l++)
	{
		for (int w = 0; w < width; w++)
		{
			VertexType vert;

			// Set position and normal of quad
			vert.position = XMFLOAT3(topLeft.x + ((float)w * scale_.x), 0.0f, topLeft.z - ((float)l * scale_.y));
			vert.normal = XMFLOAT3(0.0f, 1.0f, 0.0f);

			// Calculate the texture coordinate
			vert.texture = XMFLOAT2((float)w / width, (float)l / height);

			// Store the vert position 
			vertices[index] = vert;

			// keep track of where in the array
			index++;

		}
	}

	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	
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

void SizableQuad::sendData(ID3D11DeviceContext * deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
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