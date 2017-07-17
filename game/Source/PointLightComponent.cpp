#include "ITPEnginePCH.h"

IMPL_COMPONENT(PointLightComponent, Component, MAX_POINT_LIGHTS);

PointLightComponent::PointLightComponent(Actor& owner)
	:Component(owner)
{
	mData.mEnabled = 1;
	mData.mPosition = owner.GetPosition();
}

void PointLightComponent::Register()
{
	Super::Register();
	mOwner.GetGame().GetRenderer().AddPointLight(ThisPtr());
}

void PointLightComponent::Unregister()
{
	Super::Unregister();
	mOwner.GetGame().GetRenderer().RemovePointLight(ThisPtr());
}

void PointLightComponent::OnUpdatedTransform()
{
	mData.mPosition = mOwner.GetPosition();
}

void PointLightComponent::SetProperties(const rapidjson::Value& properties)
{
	Super::SetProperties(properties);

	Vector3 diffuseColor;
	if (GetVectorFromJSON(properties, "diffuseColor", diffuseColor))
		SetDiffuse(diffuseColor);

	Vector3 specularColor;
	if (GetVectorFromJSON(properties, "specularColor", specularColor))
		SetSpecular(specularColor);

	float specularPower;
	if (GetFloatFromJSON(properties, "specularPower", specularPower))
		SetSpecularPower(specularPower);

	float innerRadius;
	if (GetFloatFromJSON(properties, "innerRadius", innerRadius))
		SetInnerRadius(innerRadius);

	float outerRadius;
	if (GetFloatFromJSON(properties, "outerRadius", outerRadius))
		SetOuterRadius(outerRadius);
}

