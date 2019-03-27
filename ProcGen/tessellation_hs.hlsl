// Tessellation Hull Shader
// Prepares control points for tessellation

cbuffer TessellationBuffer : register(b0)
{
    float tessFactor;
    float3 cameraPosition;
}

struct InputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID)
{    
    ConstantOutputType output;

    float3 avgPos = inputPatch[0].position;
    avgPos += inputPatch[1].position;
    avgPos += inputPatch[2].position;
    avgPos += inputPatch[3].position;

    avgPos /= 4;

    float distance = pow(cameraPosition.x - avgPos.x, 2) + /*pow(cameraPosition.y - avgPos.y, 2) + */pow(cameraPosition.z - avgPos.z, 2);
    distance = sqrt(distance);
	distance /= 2;
    // Set the tessellation factors for the three edges of the triangle.
	output.edges[0] = tessFactor;// / distance;
	output.edges[1] = tessFactor;// / distance;
    output.edges[2] = tessFactor;// / distance;
    output.edges[3] = tessFactor;// / distance;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside[0] = tessFactor;// / distance;
    output.inside[1] = tessFactor;// / distance;

    return output;
}


[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PatchConstantFunction")]
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input colour as the output colour.
    output.tex = patch[pointId].tex;

	output.normal = patch[pointId].normal;

    return output;
}