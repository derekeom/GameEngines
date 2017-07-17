#pragma once
#include "Component.h"
#include "PointLightData.h"

class PointLightComponent : public Component
{
	DECL_COMPONENT(PointLightComponent, Component);
public:
	PointLightComponent(Actor& owner);

	void Register() override;
	void Unregister() override;

	void OnUpdatedTransform() override;

	const PointLightData& GetData() const { return mData; }

	// Getter/setter functions for PointLightData

	const Vector3& GetDiffuse() const { return mData.mDiffuse; }
	void SetDiffuse(const Vector3 &diffuse) { mData.mDiffuse = diffuse; }

	const Vector3& GetSpecular() const { return mData.mSpecular; }
	void SetSpecular(const Vector3 &specular) { mData.mSpecular = specular; }

	const Vector3& GetPosition() const { return mData.mPosition; }
	void SetPosition(const Vector3 &position) { mData.mPosition = position; }

	float GetSpecularPower() const { return mData.mSpecularPower; }
	void SetSpecularPower(const float specularPower) { mData.mSpecularPower = specularPower; }

	float GetInnerRadius() const { return mData.mInnerRadius; }
	void SetInnerRadius(const float innerRadius) { mData.mInnerRadius = innerRadius; }

	float GetOuterRadius() const { return mData.mOuterRadius; }
	void SetOuterRadius(const float outerRadius) { mData.mOuterRadius = outerRadius; }

	int IsEnabled() const { return mData.mEnabled; }
	void SetEnabled(const int enabled) { mData.mEnabled = enabled; }

	void SetProperties(const rapidjson::Value& properties) override;
private:
	PointLightData mData;
};

DECL_PTR(PointLightComponent);
