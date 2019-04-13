#include "RiverQuad.h"


RiverQuad::RiverQuad(ID3D11Device * device, ID3D11DeviceContext * deviceContext, float width_, float height_, XMFLOAT3 initialPos_, XMFLOAT3 translatedPos_)
{
	width = width_;
	height = height_;
	initialPos = initialPos_;
	translatedPos = translatedPos_;
	initBuffers(device);

	// Pass in pos of last top of quad
	// Base pos from bottom vertex off of passed in pos
	/// Decrease width by current point in shape
	/// Translate by height of next quad
}

RiverQuad::~RiverQuad()
{
	// Run parent deconstructor
	BaseMesh::~BaseMesh();
}


void RiverQuad::initBuffers(ID3D11Device * device)
{
	D3D11_SUBRESOURCE_DATA vertexData, indexData;

	vertexCount = 4;
	indexCount = 6;

	VertexType* vertices = new VertexType[vertexCount];
	unsigned long* indices = new unsigned long[indexCount];

	///X-Z Plane
	vertices[0].position = XMFLOAT3(width, 0.0f, height);  // Top right.
	vertices[0].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);

	vertices[1].position = XMFLOAT3(-width, 0.0f, height);  // Top left.
	vertices[1].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[1].texture = XMFLOAT2(0.0f, 0.0f);

	vertices[2].position = XMFLOAT3(-width, 0.0f, 0.0f);  // bottom left.
	vertices[2].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[2].texture = XMFLOAT2(1.0f, 0.0f);

	vertices[3].position = XMFLOAT3(width, 0.0f, 0.0f);  // bottom right.
	vertices[3].normal = XMFLOAT3(0.0f, 1.0f, 0.0f);
	vertices[3].texture = XMFLOAT2(1.0f, 1.0f);

	// Load the index array with data.
	indices[0] = 1;  // Top left
	indices[1] = 2;  // Bottom left.
	indices[2] = 0;  // Top right.

	indices[3] = 0;  // Top right
	indices[4] = 2;  // Bottom left.
	indices[5] = 3;  // Bottom right.

	D3D11_BUFFER_DESC vertexBufferDesc = { sizeof(VertexType) * vertexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, 0, 0, 0 };
	vertexData = { vertices, 0 , 0 };
	device->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc = { sizeof(unsigned long) * indexCount, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, 0, 0, 0 };
	indexData = { indices, 0, 0 };
	device->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;
	delete[] indices;
	indices = 0;
}

void RiverQuad::sendData(ID3D11DeviceContext * deviceContext, D3D_PRIMITIVE_TOPOLOGY top)
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