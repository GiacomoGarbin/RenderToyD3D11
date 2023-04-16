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

struct DefaultVSIn
{
	float3 position : POSITION;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD;
	float3 tangent  : TANGENT;
};

struct DefaultVSOut
{
	float4 position : SV_POSITION;
	float3 world    : POSITION0;
	float3 normal   : NORMAL;
	float2 uv       : TEXCOORD;
	float3 tangent  : TANGENT;
};

struct DefaultPSOut
{
	float4 colour  : SV_Target0;
	float4 gbuffer : SV_Target1; // xyz: world normal, w: material index
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

	return vout;
}

DefaultPSOut DefaultImpl(const DefaultVSOut pin, const int materialIndex)
{
	const MaterialData material = gMaterialBuffer[materialIndex];

	float4 diffuse = material.diffuse;
	if (material.diffuseTextureIndex != -1)
	{
		diffuse *= gDiffuseTextures.Sample(gSamplerLinearWrap, float3(pin.uv, material.diffuseTextureIndex));
	}

#if ALPHA_TEST
	clip(diffuse.a - 0.1f);
#endif // ALPHA_TEST

#if NORMAL_MAPPING
	const float4 normalTexel = gNormalTextures.Sample(gSamplerLinearWrap, float3(pin.uv, material.normalTextureIndex));
	const float3 normal = NormalSampleToWorldSpace(normalTexel.rgb, normalize(pin.normal), pin.tangent);
#else // NORMAL_MAPPING
	const float3 normal = normalize(pin.normal);
#endif // NORMAL_MAPPING

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

// #if SHADOW || 1
//     // only the first light casts a shadow
//     float3 ShadowFactor = float3(1.0f, 1.0f, 1.0f);
// 	ShadowFactor[0] = CalculateShadowFactor(pin.ShadowPositionH);
// #else // SHADOW
	const float3 shadow = 1.0f;
// #endif // SHADOW

	const float4 direct = ComputeLighting(gLights, lightMaterial, pin.world, normal, toEye, shadow);
	
	DefaultPSOut result;

	result.colour = ambient + direct;

	// specular reflections
	{
		const float3 r = reflect(-toEye, normal);
		// const float4 reflection = gCubeMap.Sample(gSamplerLinearWrap, r);
		const float3 fresnel = SchlickFresnel(material.fresnel, normal, r);
		// result.rgb += shininess * fresnel * reflection.rgb;
		result.colour.rgb += shininess * fresnel;
	}

// #if FOG
// 	const float FogAmount = saturate((DistToEye - gFogStart) / gFogRange);
// 	result = lerp(result, gFogColor, FogAmount);
// #endif // FOG

	result.colour.a = diffuse.a;

	result.gbuffer.xyz = normal;
	result.gbuffer.w = materialIndex;

	return result;
}

DefaultPSOut DefaultPS(const DefaultVSOut pin)
{
	return DefaultImpl(pin, gMaterialIndex);
}

#endif // DEFAULT