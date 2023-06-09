#ifndef DEFAULT
#define DEFAULT

#if FIXME
#include "../RenderToyD3D11/shaders/Common.hlsl"
#else
#include "Common.hlsl"
#endif

cbuffer ObjectCB : register(b1)
{
	float4x4 gWorld;
	float4x4 uvTransform;
	uint gMaterialIndex;
	float3 padding;
};

#if WATER_NORMAL_MAPPING
cbuffer WavesCB : register(b2)
{
	float4x4 gWavesNormalMapTransform0;
	float4x4 gWavesNormalMapTransform1;
};

Texture2D gWavesNormalMap0 : register(t9);
Texture2D gWavesNormalMap1 : register(t10);
#endif // WATER_NORMAL_MAPPING

struct DefaultVSIn
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float3 tangent  : TANGENT;
};

struct DefaultVSOut
{
	float4 position : SV_POSITION;
	float3 world    : POSITION0;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD0;
	float3 tangent  : TANGENT;
	
#if WATER_NORMAL_MAPPING
	float2 wavesNormalMapTexCoord0 : TEXCOORD1;
	float2 wavesNormalMapTexCoord1 : TEXCOORD2;
#endif // WATER_NORMAL_MAPPING
};

DefaultVSOut DefaultVS(DefaultVSIn vin)
{
	DefaultVSOut vout;

	const MaterialData material = gMaterialBuffer[gMaterialIndex];
	
// #ifdef SKINNED
//     float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
//     weights[0] = vin.BoneWeights.x;
//     weights[1] = vin.BoneWeights.y;
//     weights[2] = vin.BoneWeights.z;
//     weights[3] = 1.0f - weights[0] - weights[1] - weights[2];

//     float3 PositionL = float3(0.0f, 0.0f, 0.0f);
//     float3 NormalL = float3(0.0f, 0.0f, 0.0f);
//     float3 TangentL = float3(0.0f, 0.0f, 0.0f);

//     for(int i = 0; i < 4; ++i)
//     {
// 		const float4x4 BoneTransform = gBoneTransforms[vin.BoneIndices[i]];

//         PositionL += weights[i] * mul(float4(vin.PositionL, 1.0f), BoneTransform).xyz;
//         NormalL += weights[i] * mul(vin.NormalL, (float3x3)(BoneTransform));
//         TangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3)(BoneTransform));
//     }

//     vin.PositionL = PositionL;
//     vin.NormalL = NormalL;
//     vin.TangentL.xyz = TangentL;
// #endif // SKINNED

	vout.world = mul(gWorld, float4(vin.position, 1.0f)).xyz;
	vout.position = mul(gViewProj, float4(vout.world, 1.0f));

	vout.normal = mul((float3x3)(gWorld), vin.normal);
	vout.tangent = mul((float3x3)(gWorld), vin.tangent);

	vout.uv = mul(uvTransform, float4(vin.uv, 0.0f, 0.0f)).xy;
	// vout.uv = mul(material.uvTransform, float4(vout.uv, 0.0f, 0.0f)).xy;

	// vout.uv = vin.uv;
	//vout.uv = uv;

	// // projective tex-coords to project shadow map onto scene
	// vout.ShadowPositionH = mul(PositionW, gShadowMapTransform);

#if WATER_NORMAL_MAPPING
	vout.wavesNormalMapTexCoord0 = mul(gWavesNormalMapTransform0, float4(vin.uv, 0, 1)).xy;
	vout.wavesNormalMapTexCoord1 = mul(gWavesNormalMapTransform1, float4(vin.uv, 0, 1)).xy;
#endif // WATER_NORMAL_MAPPING

	return vout;
}

void GetDiffuseAndNormal(const DefaultVSOut pin,
						 const MaterialData material,
						 inout float4 diffuse,
						 inout float3 normal)
{
	diffuse = material.diffuse;
	if (material.diffuseTextureIndex != -1)
	{
		// diffuse *= gDiffuseTextures.Sample(gSamplerLinearWrap, float3(pin.uv, material.diffuseTextureIndex));
		diffuse *= gDiffuseTextures.Sample(gSamplerLinearWrap, pin.uv);
	}

#if ALPHA_TEST
	clip(diffuse.a - 0.1f);
#endif // ALPHA_TEST

#if WATER_NORMAL_MAPPING
	const float3 n = normalize(pin.normal);

	float4 n0 = gWavesNormalMap0.Sample(gSamplerLinearWrap, pin.wavesNormalMapTexCoord0);
	n0.xyz = NormalSampleToWorldSpace(n0, n, pin.tangent);

	float4 n1 = gWavesNormalMap1.Sample(gSamplerLinearWrap, pin.wavesNormalMapTexCoord1);
	n1.xyz = NormalSampleToWorldSpace(n1, n, pin.tangent);

	normal = normalize(n0.xyz + n1.xyz);
#else // WATER_NORMAL_MAPPING
	if (material.normalTextureIndex != -1)
	{
		// const float4 normalTexel = gNormalTextures.Sample(gSamplerLinearWrap, float3(pin.uv, material.normalTextureIndex));
		const float4 normalTexel = gNormalTextures.Sample(gSamplerLinearWrap, pin.uv);
		normal = NormalSampleToWorldSpace(normalTexel, normalize(pin.normal), pin.tangent);
	}
	else
	{
		normal = normalize(pin.normal);
	}
#endif // WATER_NORMAL_MAPPING
}

float4 DefaultImpl(const DefaultVSOut pin, const int materialIndex)
{
	const MaterialData material = gMaterialBuffer[materialIndex];

	float4 diffuse;
	float3 normal;
	GetDiffuseAndNormal(pin, material, diffuse, normal);
	
// #if WATER_NORMAL_MAPPING
// 	return float4(normal,1);
// #endif // WATER_NORMAL_MAPPING

#if FAKE_NORMAL
	const float3 e0 = ddx(pin.world);
	const float3 e1 = ddy(pin.world);
	normal = normalize(cross(e0, e1));
#endif // FAKE_NORMAL

	float3 toEye = gEyePosition - pin.world;
	const float distToEye = length(toEye);
	toEye /= distToEye;

// 	// indirect lighting
// #if AMBIENT_OCCLUSION || 1
// 	const float2 TexCoord = pin.PositionH.xy * gRenderTargetSizeInverse;
// 	const float AmbientAccess = gAmbientOcclusionMap.Sample(gSamplerLinearClamp, TexCoord, 0.0f).r;
// 	const float4 ambient = gAmbientLight * DiffuseAlbedo * AmbientAccess;
// #else // AMBIENT_OCCLUSION
	const float4 ambient = gAmbientLight * diffuse;
// #endif // AMBIENT_OCCLUSION
	
	// direct lighting
#if NORMAL_MAPPING
	const float shininess = (1.0f - material.roughness) * normalTexel.a;
#else // NORMAL_MAPPING
	const float shininess = 1.0f - material.roughness;
#endif // NORMAL_MAPPING

	const Material lightMaterial = { diffuse, material.fresnel, shininess };

	float3 shadow = 1.0f;
#if SHADOW_MAPPING
    // only the first light casts a shadow
	shadow[0] = gShadowResolve.Load(uint3(pin.position.xy, 0));
	// shadow[0] = CalculateShadowFactor(pin.ShadowPositionH);
#endif // SHADOW_MAPPING

	const float4 direct = ComputeLighting(gLights, lightMaterial, pin.world, normal, toEye, shadow);
	
	float4 result;

	result = ambient + direct;

	// specular reflections
	{
		const float3 r = reflect(-toEye, normal);
		const float3 fresnel = SchlickFresnel(material.fresnel, normal, r);
		float4 reflection = 0;
#if REFLECTIVE_SURFACE
		// const float4 reflection = gCubeMap.Sample(gSamplerLinearWrap, r);
#if WATER_NORMAL_MAPPING
		const float3 rFlat = reflect(-toEye, float3(0,1,0));
		float2 perturbation; 
		perturbation.x = 0.0f;
		perturbation.y = r.y - rFlat.y;

		float2 uv = pin.position.xy * gOneOverResolution;
		uv += perturbation * 0.05f;

		reflection = gReflectionResolve.Sample(gSamplerLinearWrap, uv);
#else // WATER_NORMAL_MAPPING
		reflection = gReflectionResolve.Load(uint3(pin.position.xy, 0));
#endif // WATER_NORMAL_MAPPING
		reflection.rgb = lerp(1, reflection.rgb, reflection.a);
#endif // REFLECTIVE_SURFACE
		result.rgb += shininess * fresnel * reflection.rgb;
	}

// #if FOG
// 	const float FogAmount = saturate((DistToEye - gFogStart) / gFogRange);
// 	result = lerp(result, gFogColor, FogAmount);
// #endif // FOG

#if FAKE_NORMALS
	// result.rgb = normal;
#endif // FAKE_NORMALS

	result.a = diffuse.a;

	return result;
}

float4 DefaultPS(const DefaultVSOut pin) : SV_Target0
{
	return DefaultImpl(pin, gMaterialIndex);
}

#endif // DEFAULT