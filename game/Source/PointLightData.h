#pragma once
#include "Math.h"

const size_t MAX_POINT_LIGHTS = 8;

struct PointLightData
{
	PointLightData();
	
	Vector3 mDiffuse;
	float mPadding;

	Vector3 mSpecular;
	float mPadding2;

	Vector3 mPosition;
	float mSpecularPower;

	float mInnerRadius;
	float mOuterRadius;
	int mEnabled;
	float mPadding3;
};
