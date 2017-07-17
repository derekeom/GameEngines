#include "ITPEnginePCH.h"

IMPL_COMPONENT(SkeletalMeshComponent, MeshComponent, 32);

SkeletalMeshComponent::SkeletalMeshComponent(Actor& owner)
	:MeshComponent(owner)
{

}

void SkeletalMeshComponent::Draw(class Renderer& render)
{
	if (mMesh)
	{
		render.DrawSkeletalMesh(mMesh->GetVertexArray(), mMesh->GetTexture(mTextureIndex),
			mOwner.GetWorldTransform(), mPalette);
	}
}

void SkeletalMeshComponent::Tick(float deltaTime)
{
	Super::Tick(deltaTime);

	if (mAnimation)
	{
		mAnimationTime += deltaTime * mAnimationPlayRate;

		if (mPrevAnimation && mAnimationTime >= min(mBlendTime, mAnimation->GetLength()))
		{
			// Animation blending transition is finished
			mPrevAnimation = nullptr;
		}

		if (mAnimationTime >= mAnimation->GetLength())
		{
			mAnimationTime -= mAnimation->GetLength();
		}

		ComputeMatrixPalette();
	}
}

float SkeletalMeshComponent::PlayAnimation(AnimationPtr anim, float playRate /*= 1.0f*/, float blendTime /*= 0.0f*/)
{
	DbgAssert(mSkeleton != nullptr, "Can't play an animation without a skeleton!");
	DbgAssert(mSkeleton->GetNumBones() == anim->GetNumBones(), 
		"Skeleton and animation have a different number of bones!");

	// Set previous animation for blending
	if (mAnimation)
	{
		mPrevAnimation = mAnimation;
		mPrevAnimationTime = mAnimationTime;
		mBlendTime = blendTime;
	}

	// Set animation
	mAnimation = anim;
	mAnimationPlayRate = playRate;
	mAnimationTime = 0.0f;

	// Return length of the animation
	return anim->GetLength();
}

void SkeletalMeshComponent::SetProperties(const rapidjson::Value& properties)
{
	Super::SetProperties(properties);

	std::string skeleton;
	if (GetStringFromJSON(properties, "skeleton", skeleton))
	{
		mSkeleton = mOwner.GetGame().GetAssetCache().Load<Skeleton>(skeleton);
	}
}

void SkeletalMeshComponent::ComputeMatrixPalette()
{
	// Get global inverse bind poses
	std::vector<SimdMatrix4> globalInvBindPose = mSkeleton->GetGlobalInvBindPoses();

	// Get global poses
	std::vector<SimdMatrix4> animationPose;
	if (mPrevAnimation)
	{
		mAnimation->GetBlendedGlobalPoseAtTime(animationPose, mSkeleton, mPrevAnimation, mAnimationTime, mPrevAnimationTime, mBlendTime);
	}
	else
	{
		mAnimation->GetGlobalPoseAtTime(animationPose, mSkeleton, mAnimationTime);
	}

	// Multiply and add to matrix palette
	for (unsigned int i = 0; i < mAnimation->GetNumBones(); ++i)
	{
		mPalette.mMatrixPalette[i] = globalInvBindPose[i];
		mPalette.mMatrixPalette[i].Mul(animationPose[i]);
	}
}
