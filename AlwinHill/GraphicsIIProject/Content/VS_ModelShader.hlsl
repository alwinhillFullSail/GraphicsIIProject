// A constant buffer that stores the three basic column-major matrices for composing geometry.
cbuffer ModelViewProjectionConstantBuffer : register(b0)
{
	matrix model;
	matrix view;
	matrix projection;
};

// Per-vertex data used as input to the vertex shader.
struct VertexShaderInput
{
	float3 pos : POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
	float3 instancePos: UV1;
};

// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
	float3 worldPos : POSITION;
};

// Simple shader to do vertex processing on the GPU.
PixelShaderInput main(VertexShaderInput input)
{
	PixelShaderInput output;
	float4 pos = float4(input.pos, 1.0f);

	// Transform the vertex position into projected space.
	matrix world = model;
	world[3].xyz += input.instancePos.xyz;
	//pos.xyz += input.instancePos.xyz;
	//pos = mul(pos, model);
	pos = mul(pos, world);
	output.worldPos = pos;
	pos = mul(pos, view);
	pos = mul(pos, projection);
	output.pos = pos;
	// Pass the color through without modification.
	output.uv = input.uv;
	output.normal = input.normal;

	return output;
}
