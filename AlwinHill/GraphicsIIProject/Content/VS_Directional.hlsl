//cbuffer PerFrame : register(b0)
//{
//	matrix ViewProjectionMatrix;
//}
//
//struct AppData
//{
//	// Per-vertex data
//	float3 Position : POSITION;
//	float3 Normal   : NORMAL;
//	float2 TexCoord : TEXCOORD;
//	// Per-instance data
//	matrix Matrix   : WORLDMATRIX;
//	matrix InverseTranspose : INVERSETRANSPOSEWORLDMATRIX;
//};
//
//VertexShaderOutput InstancedVertexShader(AppData IN)
//{
//	VertexShaderOutput OUT;
//
//	matrix MVP = mul(ViewProjectionMatrix, IN.Matrix);
//
//	OUT.Position = mul(MVP, float4(IN.Position, 1.0f));
//	OUT.PositionWS = mul(IN.Matrix, float4(IN.Position, 1.0f));
//	OUT.NormalWS = mul((float3x3)IN.InverseTranspose, IN.Normal);
//	OUT.TexCoord = IN.TexCoord;
//
//	return OUT;
//}

cbuffer MatrixBuffer
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexInputType
{
	float4 position : POSITION;
	float2 tex : UV;
	float3 normal : NORMAL;
	float3 instancePos: UV1;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : UV;
	float3 normal : NORMAL;
};

PixelInputType main(VertexInputType input)
{
	PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
	input.position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only.
	output.normal = mul(input.normal, (float3x3)worldMatrix);

	// Normalize the normal vector.
	output.normal = normalize(output.normal);

	return output;
}
