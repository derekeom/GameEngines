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
	float3 mWorldNormal : NORMAL0;
	float3 mWorldPos : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output;

	// world space position
	output.mWorldPos = mul(float4(input.mPos, 1.0f), mWorldTransform).xyz;

	// normalized device space position
	output.mPos = mul(float4(output.mWorldPos, 1.0f), mViewProjection);

	// world space normal
	output.mWorldNormal = mul(float4(input.mNormal, 0.0f), mWorldTransform).xyz;

	// texture coordinates
	output.mTex = input.mTex;

	return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	// Normalize normal vector,
	// because interpolation may have messed up the normalization
	input.mWorldNormal = normalize(input.mWorldNormal);

	// Phong Lighting Equation
	// I = ambient + sumOfAllLights(diffuse * (N dot L) + specular * (R dot V) ^ a)
	float3 Phong = mAmbientLight;
	for(int i = 0; i < 8; i++)
	{
		// If point light isn't enabled, skip it
		if (!mPointLights[i].mEnabled) continue;

		// Compute the normalized L and V vectors.
		float3 L = normalize(mPointLights[i].mPosition - input.mWorldPos);
		float3 V = normalize(mWorldCameraPosition - input.mWorldPos);
		float3 R = reflect(-L, input.mWorldNormal);

		// if (N dot L) > 0, light should affect this pixel
		float NdotL = dot(input.mWorldNormal, L);
		if (NdotL > 0)
		{
			// Smooth falloff
			float falloff = lerp(1.0f, 0.0f, smoothstep(
				mPointLights[i].mInnerRadius,
				mPointLights[i].mOuterRadius,
				distance(mPointLights[i].mPosition, input.mWorldPos)
			));

			// Diffuse light
			Phong += mPointLights[i].mDiffuseColor * (NdotL * falloff);

			// Specular light
			Phong += mPointLights[i].mSpecularColor * (pow(max(0.0f, dot(R, V)), mPointLights[i].mSpecularPower) * falloff);
		}
	}

	// Clamp Phong lighting to [0.0f, 1.0f]
	Phong = saturate(Phong);

	// Return color sampled from texture
	return DiffuseTexture.Sample(DefaultSampler, input.mTex) * float4(Phong, 1.0f);
}
