cbuffer ModelViewProjectionConstantBuffer : register (b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct VertexShaderInput
{
	float3 position : POSITION;
	float3 uv : UV;
};

struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 uv : UV;
};

PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;

	float4 position = float4(input.position, 1.0);

	// Calculate the position of the vertex against the world, view, and projection matrices.
	position = mul(position, worldMatrix);
	position = mul(position, viewMatrix);
	position = mul(position, projectionMatrix);

	// Send the unmodified position through to the pixel shader.
	output.position = position;
	output.uv = input.position;

	return output;
}
