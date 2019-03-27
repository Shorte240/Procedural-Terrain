#include "TessellatedPlane.h"
#include <time.h>
#include <cmath>

// https://stackoverflow.com/questions/17805912/how-can-i-create-draw-render-gridplane-using-directx
// Practical rendering chapter nines

TessellatedPlane::TessellatedPlane(ID3D11Device* device, ID3D11DeviceContext* deviceContext, int _scale, int _patchSize) : scale(_scale), patchSize(_patchSize)
{
	int numberOfPatches = scale * scale;

	vertexCount = numberOfPatches + (2 * scale) + 1;

	vertices = new VertexType[vertexCount];

	FlatPlane();
	initBuffers(device);
}


TessellatedPlane::~TessellatedPlane()
{
	delete[] vertices;
	vertices = 0;
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}

void TessellatedPlane::initBuffers(ID3D11Device * device)
{
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	int numberOfPatches = scale * scale;

	indexCount = 4 * numberOfPatches;

	unsigned long* indices = new unsigned long[indexCount];

	int indexTrack = 0;

	for (int y = 0; y < scale; y++) 
	{
		for (int x = 0; x < scale; x++) 
		{
			indices[indexTrack + 0] = (y  + 0) + (x + 0) * (scale + 1);
			indices[indexTrack + 1] = (y  + 1) + (x + 0) * (scale + 1);
			indices[indexTrack + 2] = (y  + 1) + (x + 1) * (scale + 1);
			indices[indexTrack + 3] = (y  + 0) + (x + 1) * (scale + 1);

			indexTrack += 4;
		}
	}

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

void TessellatedPlane::FlatPlane()
{
	for (int y = 0; y < scale + 1; y++) 
	{
		for (int x = 0; x < scale + 1; x++) 
		{
			XMFLOAT2 patchPosition;
			int vertexIdentity = ((scale + 1) * y) + x;

			patchPosition.x = (-scale / 2) + (patchSize * x);
			patchPosition.y = (-scale / 2) + (patchSize * y);

			// Load the vertex array with data - setting the top left position of every patch
			vertices[vertexIdentity].position = XMFLOAT3(patchPosition.x, 0.0f, patchPosition.y);
			vertices[vertexIdentity].texture = XMFLOAT2((float)x / scale, (float)y / scale);
			//vertices[vertexIdentity].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
		}
	}

	CalculateNormals();
}

void TessellatedPlane::CalculateNormals()
{
	for (int y = 0; y < scale + 1; y++) {
		for (int x = 0; x < scale + 1; x++) {
			int vertexIdentity = ((scale + 1) * y) + x;
			int vertexIdentityAbove = ((scale + 1) * (y + 1)) + x;
			int vertexIdentityBelow = ((scale + 1) * (y - 1)) + x;

			XMVECTOR A = XMLoadFloat3(&vertices[vertexIdentity + 1].position);

			XMVECTOR B = XMLoadFloat3(&vertices[vertexIdentity - 1].position);

			XMVECTOR AB = XMVector3Normalize(B - A);

			XMVECTOR C = XMLoadFloat3(&vertices[vertexIdentityAbove].position);

			XMVECTOR D = XMLoadFloat3(&vertices[vertexIdentityBelow].position);

			XMVECTOR CD = XMVector3Normalize(D - C);

			XMVECTOR vertexNormal = XMVector3Normalize(XMVector3Cross(AB, CD));

			XMStoreFloat3(&vertices[vertexIdentity].normal, vertexNormal);
		}
	}
}

void TessellatedPlane::sendData(ID3D11DeviceContext* deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
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

void TessellatedPlane::GenerateHeightMap(ID3D11Device* device)
{
	for (int y = 0; y < scale + 1; y++) 
	{
		for (int x = 0; x < scale + 1; x++) 
		{
			int vertexIdentity = ((scale + 1) * y) + x;
			float newYPos = RandomIntRange(6, 12);

			// Load the vertex array with data - setting the top left position of every patch
			vertices[vertexIdentity].position.y = newYPos;
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void TessellatedPlane::SmoothVertices(ID3D11Device* device)
{
	VertexType* tempVertices = new VertexType[vertexCount];
	tempVertices = vertices;

	for (int j = 0; j < scale + 1; j++)
	{
		for (int i = 0; i < scale + 1; i++)
		{
			float averageHeight = 0.0f;
			int index;

			for (int xMod = -1; xMod < 2; xMod++) 
			{
				for (int yMod = -1; yMod < 2; yMod++) 
				{
					if (xMod == 0 && yMod == 0) 
					{
						continue;
					}
					index = ((scale + 1 + yMod) * j) + i + xMod;

					averageHeight += vertices[index].position.y;

				}
			}

			index = ((scale + 1) * j) + i;
			averageHeight /= 8.0f;

			tempVertices[index].position.y /= 1.1f;
		}
	}

	vertices = tempVertices;

	CalculateNormals();

	initBuffers(device);
}

void TessellatedPlane::Faulting(ID3D11Device* device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		float v = std::rand();
		float a = sin(v);
		float b = cos(v);
		float d = sqrt(scale*scale + scale * scale);
		// rand() / RAND_MAX gives a random number between 0 & 1
		// therefore cwill be a random number between -d/2 and d/2
		float c = ((float)std::rand() / RAND_MAX) * d - d / 2.0f;

		for (int z = 0; z < scale + 1; z++)
		{
			for (int x = 0; x < scale + 1; x++)
			{
				int index;

				if ((a * z) + (b * x) - c > 0)
				{
					index = ((scale + 1) * z) + x;

					vertices[index].position.y += displacement;
				}
				else
				{
					index = ((scale + 1) * z) + x;

					vertices[index].position.y -= displacement;
				}
			}
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void TessellatedPlane::CircleAlgorithm(ID3D11Device * device, float displacement)
{
	for (int i = 0; i < 100; i++)
	{
		int randX = std::rand() % (scale + 1);
		int randZ = std::rand() % (scale + 1);
		int randCircSize = std::rand() % ((scale + scale) / 10); // circle diameter

		for (int z = 0; z < scale + 1; z++)
		{
			for (int x = 0; x < scale + 1; x++)
			{
				int index;

				float pd = sqrt((randX - x)*(randX - x) + (randZ - z)*(randZ - z)) * 2.0f / randCircSize;	// pd = distFromCircle*2/size

				if (fabs(pd) <= 1.0f)	// if vertex in circle, displace upwards
				{
					index = ((scale + 1) * z) + x;

					float diff = displacement / 2.0f + cos(pd*3.14)*displacement / 2.0f;

					vertices[index].position.y += diff;
				}
			}
		}
	}

	CalculateNormals();

	initBuffers(device);
}

void TessellatedPlane::MidpointDisplacement(ID3D11Device * device, float displacement)
{

}

float TessellatedPlane::RandomIntRange(int min, int max)
{
	return (min + (std::rand() % (max - min + 1)));
}