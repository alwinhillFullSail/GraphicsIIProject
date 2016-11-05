//// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
	float3 normal : NORMAL;
};

texture2D env : register(t1);
SamplerState envFilter : register(s1);

float4 main(PixelShaderInput input) : SV_TARGET
{
	//return env.Sample(envFilter, icolor);
	return env.Sample(envFilter, input.uv.xy);
	//return float4(input.uv, 1.0f);
}
