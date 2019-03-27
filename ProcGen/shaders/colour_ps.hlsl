// Colour pixel/fragment shader
// Basic fragment shader outputting a colour

struct InputType
{
	float4 position : SV_POSITION;
	float4 colour : COLOR;
};


float4 main(InputType input) : SV_TARGET
{
	/*float4 red = float4(1.f, 0.f, 0.f, 1.f);

	return red;*/
	return input.colour;
}