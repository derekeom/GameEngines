// We want to use row major matrices
#pragma pack_matrix(row_major)

struct PointLightData
{
	float3 mDiffuseColor;
	float3 mSpecularColor;
	float3 mPosition;
	float mSpecularPower;
	float mInnerRadius;
	float mOuterRadius;
	bool mEnabled;
};

// Per-camera constants
cbuffer PerCameraConstants : register(b0)
{
	float4x4 mViewProjection;
	float3 mWorldCameraPosition;
}

// Per-object constants
cbuffer PerObjectConstants : register(b1)
{
	float4x4 mWorldTransform;
}

// Lighting constants
cbuffer LightingConstants : register(b2)
{
	PointLightData mPointLights[8];
	float3 mAmbientLight;
}

// For sampling the texture
SamplerState DefaultSampler : register(s0);

// Texture
Texture2D DiffuseTexture : register(t0);