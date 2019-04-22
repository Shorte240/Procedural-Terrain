// Manipulation pixel shader
// For manipulating pixels

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambient;
	float4 diffuse;
	float3 direction;
	float padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 ldiffuse)
{
	float intensity = saturate(dot(normal, lightDirection));
	float4 colour = saturate(ldiffuse * intensity);
	return colour;
}

float4 main(InputType input) : SV_TARGET
{
	float4 lightColour = 0.f;

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
	float4 textureColour = texture0.Sample(sampler0, input.tex);

	// Calculate the light colour.
	lightColour = calculateLighting(-direction, input.normal, diffuse);

	return lightColour * textureColour;
}