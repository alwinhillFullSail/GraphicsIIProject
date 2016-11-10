
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float3 uv : UV;
};

textureCUBE env : register(t0);
SamplerState envFilter : register (s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return env.Sample(envFilter, input.uv);
}
