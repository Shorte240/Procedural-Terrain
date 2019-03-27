// colour vertex shader
// Simple geometry pass

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : POSITION;
	float4 colour : COLOR;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};

OutputType main(InputType input)
{
	OutputType output;

	float4 new_pos = input.position;

	/*new_pos.x *= 2;
	new_pos.y *= 2;
	new_pos.z *= 2;*/

	new_pos.w /= 2;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(new_pos, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.colour = input.colour;

	return output;
}