
struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 uv : UV;
	//float4 normal : NORMAL
};

textureCUBE env : register(t0);
SamplerState envFilter : register (s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	return env.Sample(envFilter, input.uv);
}
