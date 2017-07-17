#include "Constants.hlsl"

// Input structs for vertex and pixel shader
struct VS_INPUT
{
	float3 mPos : POSITION;
	float3 mNormal : NORMAL0;
	float2 mTex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 mPos : SV_POSITION;
	float2 mTex: TEXCOORD0;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	// outPos = inPos * worldTransform * viewProjection
	output.mPos = mul(mul(float4(input.mPos, 1.0f), mWorldTransform), mViewProjection);

	output.mTex = input.mTex;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	// Return color sampled from texture
	return DiffuseTexture.Sample(DefaultSampler, input.mTex);
}
