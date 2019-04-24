// Terrain_ps

Texture2D grassTexture : register(t0);
Texture2D slopeTexture : register(t1);
Texture2D rockTexture  : register(t2);
SamplerState Sampler0 : register(s0);

cbuffer LightBuffer : register(b0)
{
	float4 ambient;
	float4 diffuse;
	float4 direction;
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
	// float4s for grass, rock and slope textures
	float4 grassColor;
	float4 slopeColor;
	float4 rockColor;

	// slope variable used in texture setting
	float slope;

	// blend amount to determine how much to blend
	float blendAmount;

	// texture colour to hold final texture colours
	float4 textureColor;

	// general lighting colour
	float4 color = (0.0f, 0.0f, 0.0f, 1.0f);

	// Sample the grass color from the texture using the sampler at this texture coordinate location.
	grassColor = grassTexture.Sample(Sampler0, input.tex);

	// Sample the slope color from the texture using the sampler at this texture coordinate location.
	slopeColor = slopeTexture.Sample(Sampler0, input.tex);

	// Sample the rock color from the texture using the sampler at this texture coordinate location.
	rockColor = rockTexture.Sample(Sampler0, input.tex);

	// Calculate the slope of this point.
	slope = 1.0f - input.normal.y;
	//Since we have the slope we can now use it in some if statements and determine which texture to use based on the slope of the pixel.To make things look smooth we do a linear interpolation between the textures so the transition between each one isn't a sharp line in the terrain.

	// Determine which texture to use based on height.
	if (slope < 0.2)
	{
		blendAmount = slope / 0.2f;
		textureColor = lerp(grassColor, slopeColor, blendAmount);
	}

	if ((slope < 0.7) && (slope >= 0.2f))
	{
		blendAmount = (slope - 0.2f) * (1.0f / (0.7f - 0.2f));
		textureColor = lerp(slopeColor, rockColor, blendAmount);
	}

	if (slope >= 0.7)
	{
		textureColor = rockColor;
	}
	//Now do the regular lighting and add the lighting value to the texture value to get the final output color.

	//color += ambient;

	color += calculateLighting(-direction.xyz, input.normal, diffuse);

	// Saturate the final light color.
	//color = saturate(color);

	// Multiply the texture color and the final light color to get the result.
	color = color * textureColor;

	return color;
}