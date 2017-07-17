#pragma once
#include "Math.h"

const size_t MAX_SKELETON_BONES = 96;

struct MatrixPalette
{
	SimdMatrix4 mMatrixPalette[MAX_SKELETON_BONES];
};
