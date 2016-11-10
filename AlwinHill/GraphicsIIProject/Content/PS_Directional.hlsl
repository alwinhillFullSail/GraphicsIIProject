//struct PixelShaderInput
//{
//	float4 pos : SV_POSITION;
//	float3 uv : UV;
//	float3 normal : NORMAL;
//};
//
//float3 gLightDir = (0.1, 2, 3);
//float3 gAmbientColor = (0.2, 0.1, 0.01);
//float4 main(PixelShaderInput input) : SV_TARGET
//{
//	float3 ldir = -normalize(gLightDir);
//	float3 wnrm = normalize(input.normal);
//	float3 result = saturate((dot(ldir, wnrm) * input.uv) + (input.uv * gAmbientColor));
//
//	return (result, 1);
//}

Texture2D shaderTexture : register(t1);;
SamplerState SampleType : register(s1);

cbuffer LightBuffer
{
	float4 diffuseColor;
	float3 lightDirection;
	float lightType;
};

struct PixelInputType
{
	float4 position : SV_POSITION;
	float2 tex : UV;
	float3 normal : NORMAL;
};


float4 main(PixelInputType input) : SV_TARGET
{

float4 textureColor;
float3 lightDir;
float3 coneDir = float3(-1, 3.0f, 1.0f);
float lightIntensity;
float lightRatio;
float spotFactor;
float surfaceRatio;
float4 color;
float coneRatio = 0.8;
float alpha;

//Directional Light
if (lightType == 1)
{
	//// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	alpha = textureColor.w;
	// Invert the light direction for calculations.
	lightDir = -lightDirection;

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, lightDir));

	lightRatio = clamp(dot(-lightDirection, input.normal), 0, 1);
	lightIntensity = 4.5;

	color = lightRatio * diffuseColor * textureColor * lightIntensity;
}

//Point Light
if (lightType == 2)
{
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	alpha = textureColor.w;

	lightDir = normalize(lightDirection - input.position);

	lightRatio = clamp(dot(input.normal, lightDir), 0, 1);

	color = lightRatio * diffuseColor * textureColor * 2.2;
}

//Spot Light
if (lightType == 3)
{
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	alpha = textureColor.w;

	lightDir = normalize(lightDirection - input.position);

	surfaceRatio = clamp(dot(-lightDir, coneDir), 0, 1);

	spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;

	lightRatio = clamp(dot(lightDirection, input.normal), 0, 1);

	color = spotFactor * lightRatio * diffuseColor * textureColor;
}

//Ambient Light
if (lightType == 4)
{
	textureColor = shaderTexture.Sample(SampleType, input.tex);
	alpha = textureColor.w;

	color = diffuseColor * textureColor * 0.8f;
}

return float4(color.xyz, alpha);
}
