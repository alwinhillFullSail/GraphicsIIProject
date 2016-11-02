//// Per-pixel color data passed through the pixel shader.
struct PixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 uv : UV;
};
//
//
//// A pass-through function for the (interpolated) color data.
//float4 main(PixelShaderInput input) : SV_TARGET
//{
//	return float4(input.uv, 1.0f);
//}

//texture2D baseTexture : register(t0); //1st texture
//texture2D detailTexture : register(t1); //2nd texture
//SamplerState filters[2] : register(s0); //filter using CLAMP, filter 1 using WRAP
//
//
//float4 main(float2 baseUV : TEXCOORD0, float2 detailUV : TEXCOORD1, float4 modulate : COLOR ) : SV_TARGET
//{
//	float4 baseColor = baseTexture.Sample(filters[0], baseUV) * modulate; //Get base color
//	float4 detailColor = detailTexture.Sample(filters[1], detailUV); //detail effect
//	float4 finalColor = float4(lerp(baseColor.rgb, detailColor.rgb, detailColor.a), baseColor.a);
//	return finalColor;
//}

texture2D env : register(t0);
SamplerState envFilter : register(s0);

float4 main(PixelShaderInput input) : SV_TARGET
{
	//return env.Sample(envFilter, icolor);
	return env.Sample(envFilter, input.uv.xy);
	//return float4(input.uv, 1.0f);
}
