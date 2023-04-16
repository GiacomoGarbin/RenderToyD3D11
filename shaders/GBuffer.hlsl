#ifndef GBUFFER
#define GBUFFER

#if FIXME
#include "../RenderToyD3D11/shaders/Default.hlsl"
#else
#include "Default.hlsl"
#endif

float4 GBufferPS(const DefaultVSOut pin) : SV_Target0
{
	const MaterialData material = gMaterialBuffer[gMaterialIndex];

	float4 diffuse;
	float3 normal;
	GetDiffuseAndNormal(pin, material, diffuse, normal);

    float4 result;
    result.xyz = normal;
    result.w = gMaterialIndex;

	return result;
}

#endif // GBUFFER