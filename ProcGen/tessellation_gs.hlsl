// Tessellation Geometry Shader
// Shades the geometry yo

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
    //float4 worldPosition : TEXCOORD1;
};

[maxvertexcount(3)]
void main(triangle InputType input[3], inout TriangleStream<OutputType> triStream)
{
    OutputType output;

	// Append plane tri

	for (int i = 0; i < 3; i++)
	{
		//output.worldPosition = float4(mul(input[i].position, worldMatrix).xyz, 1.0f);
		
		output.position = input[i].position;
		output.tex = input[i].tex;
		output.normal = input[i].normal;
		triStream.Append(output);
	}

    triStream.RestartStrip();
}