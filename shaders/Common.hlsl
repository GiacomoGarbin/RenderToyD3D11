#ifndef COMMON
#define COMMON

#if FIXME
#include "../RenderToyD3D11/shaders/LightingUtils.hlsl"
#else
#include "LightingUtils.hlsl"
#endif

struct MaterialData
{
	float4 diffuse;
	float3 fresnel;
	float  roughness;
	int diffuseTextureIndex;
	int normalTextureIndex;
	float2 padding;
	float4x4 uvTransform;
};

// material buffer, it contains all materials
StructuredBuffer<MaterialData> gMaterialBuffer : register(t0);

Texture2DArray gDiffuseTextures : register(t1);
Texture2DArray gNormalTextures : register(t2);

// SamplerState gSamplerPointWrap        : register(s0);
// SamplerState gSamplerPointClamp       : register(s1);
SamplerState gSamplerLinearWrap       : register(s2);
// SamplerState gSamplerLinearClamp      : register(s3);
// SamplerState gSamplerAnisotropicWrap  : register(s4);
// SamplerState gSamplerAnisotropicClamp : register(s5);
// SamplerComparisonState gSamplerShadow : register(s6);

// cbuffer SkinnedCB : register(b1)
// {
//     float4x4 gBoneTransforms[96];
// };

cbuffer MainPassCB : register(b0)
{
// 	float4x4 gView;
// 	float4x4 gViewInverse;
// 	float4x4 gProj;
// 	float4x4 gProjInverse;
	float4x4 gViewProj;
	float4x4 viewProjInv;
// 	float4x4 gShadowMapTransform;
	float3 gEyePosition;
	float padding1;
// 	float2 gRenderTargetSize;
// 	float2 gRenderTargetSizeInverse;
// 	float gNearPlane;
// 	float gFarPlane;
// 	float gDeltaTime;
// 	float gTotalTime;

	float4 gAmbientLight;

// #ifdef FOG
// 	float4 gFogColor;
// 	float gFogStart;
// 	float gFogRange;
// 	float2 padding2;
// #endif // FOG

	Light gLights[LIGHT_MAX_COUNT];
};

// float3 NormalSampleToWorldSpace(const float3 NormalSample, const float3 UnitNormalW, const float3 TangentW)
// {
// 	// uncompress from [0,1] to [-1,+1]
// 	const float3 NormalT = 2.0f * NormalSample - 1.0f;

// 	// build orthonormal basis
// 	const float3 N = UnitNormalW;
// 	const float3 T = normalize(TangentW - dot(TangentW, N) * N);
// 	const float3 B = cross(N, T);

// 	const float3x3 TBN = float3x3(T, B, N);

// 	// transform from tangent space to world space
// 	const float3 NormalW = mul(NormalT, TBN);

// 	return NormalW;
// }

// float CalculateShadowFactor(float4 position)
// {
//     // complete projection by doing division by w
//     position.xyz /= position.w;

//     // depth in NDC space
//     const float depth = position.z;

//     uint width, height, mips;
//     gShadowMap.GetDimensions(0, width, height, mips);

//     // texel size
//     const float dx = 1.0f / (float)width;

//     float light = 0.0f;
//     const float2 offsets[9] =
//     {
//         float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
//         float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
//         float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
//     };

//     [unroll]
//     for(int i = 0; i < 9; ++i)
//     {
//         light += gShadowMap.SampleCmpLevelZero(gSamplerShadow,
// 											   position.xy + offsets[i],
// 											   depth).r;
//     }
    
//     return light / 9.0f;
// }

#endif // COMMON