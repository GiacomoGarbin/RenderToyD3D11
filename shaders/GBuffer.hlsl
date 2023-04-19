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
	
#if FAKE_NORMALS
	const float3 e0 = ddx(pin.world);
	const float3 e1 = ddy(pin.world);
	normal = normalize(cross(e0, e1));
#endif // FAKE_NORMALS

    float4 result;
    result.xyz = normal;
    result.w = gMaterialIndex;

	return result;
}

#endif // GBUFFER