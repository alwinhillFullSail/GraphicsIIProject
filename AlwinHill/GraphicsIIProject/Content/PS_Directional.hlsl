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
float3 coneDir = float3(-1, 5.0f, 1.0f);
float lightIntensity;
float lightRatio;
float spotFactor;
float surfaceRatio;
float4 color;
float coneRatio = 0.9;

//Directional Light
if (lightType == 1)
{
	//// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	//// Invert the light direction for calculations.
	//lightDir = -lightDirection;

	//// Calculate the amount of light on this pixel.
	//lightIntensity = saturate(dot(input.normal, lightDir));

	//// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	//color = saturate(diffuseColor * lightIntensity + 0.15f);

	//// Multiply the texture pixel and the final diffuse color to get the final pixel color result.
	//color = color * textureColor;

	lightRatio = clamp(dot(-lightDirection, input.normal), 0, 5);
	color = lightRatio * diffuseColor * textureColor;
}

//Point Light
if (lightType == 2)
{
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	lightDir = normalize(lightDirection - input.position);

	lightRatio = clamp(dot(input.normal, lightDir), 0, 1);

	color = lightRatio * diffuseColor * textureColor;
}

//Spot Light
if (lightType == 3)
{
	textureColor = shaderTexture.Sample(SampleType, input.tex);

	lightDir = normalize(lightDirection - input.position);

	surfaceRatio = clamp(dot(-lightDir, coneDir), 0, 1);

	spotFactor = (surfaceRatio > coneRatio) ? 1 : 0;

	color = spotFactor * lightRatio * diffuseColor * textureColor;
}

return color;
}
