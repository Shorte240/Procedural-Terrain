// Manipulation vertex shader
// For manipulating vertexes

Texture2D heightTex : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
	float time;
	float height;
	float frequency;
	float speed;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

OutputType main(InputType input)
{
	OutputType output;

	float4 new_pos = input.position;


	//// offset position based on sine wave
	//input.position.x = input.position.x + (height * length(input.normal) * (sin(((input.position.z * frequency) + (time * speed)))));
	new_pos.y = new_pos.y + (height * length(input.normal) * (sin(((new_pos.z * frequency) + (time * speed)))));
	//new_pos.y = sin(new_pos.x + time);
	//input.position.z = input.position.z + (height * length(input.normal) * (sin(((input.position.y * frequency) + (time * speed)))));
	new_pos.w /= 2;

	//// modify normals
	/*input.normal.x = 1 - cos(input.position.x + time);
	input.normal.y = abs(cos(input.position.x + time));*/

	// Sample the texture. Use colour to alter height of plane.
	/*float4 textureColour = heightTex.SampleLevel(sampler0, input.tex, 0, 0);
	input.position.y = textureColour.r * height;*/

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(new_pos, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}