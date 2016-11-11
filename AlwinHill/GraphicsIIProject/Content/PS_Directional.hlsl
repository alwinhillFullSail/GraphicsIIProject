Texture2D shaderTexture : register(t1);
Texture2D normalTex : register (t2);
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
//float3 coneDir = float3(-1, 3.0f, 1.0f);
float3 coneDir = float3(0.0f, 2.0f, 1.0f);

float lightIntensity;
float lightRatio;
float spotFactor;
float surfaceRatio;
float4 color;
float coneRatio = 0.8;
float alpha;

float4 normal = normalTex.Sample(SampleType, input.tex.xy);
input.normal = float4(normalize(input.normal.xyz + normal.xyz), 1);

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

	color = spotFactor * lightRatio * diffuseColor * textureColor * 1.7f;
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
