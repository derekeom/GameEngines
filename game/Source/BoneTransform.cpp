#include "ITPEnginePCH.h"

Matrix4 BoneTransform::ToMatrix() const
{
	return Matrix4::CreateFromQuaternion(mRotation) * Matrix4::CreateTranslation(mTranslation);
}

SimdMatrix4 BoneTransform::ToSimdMatrix() const
{
	SimdMatrix4 retMat;
	retMat.LoadTransform(1.0f, mTranslation, mRotation);
	return retMat;
}

BoneTransform Interpolate(const BoneTransform& a, const BoneTransform& b, float f)
{
	BoneTransform retVal = BoneTransform();
	
	// Lerp rotation
	const float dotResult = 
		a.mRotation.x * b.mRotation.x +
		a.mRotation.y * b.mRotation.y +
		a.mRotation.z * b.mRotation.z +
		a.mRotation.w * b.mRotation.w;
	float bias = -1.0f;
	if (dotResult >= 0.0f) { bias = 1.0f; }
	retVal.mRotation.x = b.mRotation.x * f + a.mRotation.x * bias * (1.0f - f);
	retVal.mRotation.y = b.mRotation.y * f + a.mRotation.y * bias * (1.0f - f);
	retVal.mRotation.z = b.mRotation.z * f + a.mRotation.z * bias * (1.0f - f);
	retVal.mRotation.w = b.mRotation.w * f + a.mRotation.w * bias * (1.0f - f);
	retVal.mRotation.Normalize();

	// Lerp translation
	retVal.mTranslation = Lerp(a.mTranslation, b.mTranslation, f);

	return retVal;
}
