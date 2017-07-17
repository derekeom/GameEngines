#include "ITPEnginePCH.h"

IMPL_COMPONENT(BoxComponent, CollisionComponent, 100);

BoxComponent::BoxComponent(Actor& owner)
	:CollisionComponent(owner)
	,mScale(1.0f, 1.0f, 1.0f)
{

}

bool BoxComponent::Intersects(CollisionComponentPtr other)
{
	if (IsA<BoxComponent>(other))
	{
		auto box = Cast<BoxComponent>(other);
		return Collision::Intersects(mWorldSpaceBounds, box->mWorldSpaceBounds);
	}
	// Ignore any other components for now
	return false;
}

bool BoxComponent::SegmentCast(const Collision::LineSegment& segment, Vector3& outPoint)
{
	return Collision::SegmentCast(segment, mWorldSpaceBounds, outPoint);
}

void BoxComponent::BoxFromMesh(MeshPtr mesh)
{
	mModelSpaceBounds = mesh->GetBoundingBox();
	OnUpdatedTransform();
}

void BoxComponent::BoxFromBox(const Collision::AxisAlignedBox& box)
{
	mModelSpaceBounds = box;
}

void BoxComponent::OnUpdatedTransform()
{
	// Update the world space box from our owning component

	// Scale the box by the owner's scale, as well as the scale vector
	mWorldSpaceBounds.mMin = mModelSpaceBounds.mMin * mOwner.GetScale() * mScale;
	mWorldSpaceBounds.mMax = mModelSpaceBounds.mMax * mOwner.GetScale() * mScale;

	// Translate by owner's translation
	mWorldSpaceBounds.mMin += mOwner.GetPosition();
	mWorldSpaceBounds.mMax += mOwner.GetPosition();
}

void BoxComponent::SetProperties(const rapidjson::Value& properties)
{
	Super::SetProperties(properties);

	// Setup the model space bounds for BoxComponent.
	std::string meshName;
	if (GetStringFromJSON(properties, "mesh", meshName))
	{
		BoxFromMesh(mOwner.GetGame().GetAssetCache().Load<Mesh>(meshName));
	}

	// Set scale
	GetVectorFromJSON(properties, "scale", mScale);

	// Since the scale might've updated, update the world bounds
	OnUpdatedTransform();
}
