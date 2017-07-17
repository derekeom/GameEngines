#include "ITPEnginePCH.h"

LevelLoader::LevelLoader(Game& game)
	:mGame(game)
{
	SetupSpawnMaps();
}

void LevelLoader::SetupSpawnMaps()
{
	// Actor spawn map
	mActorSpawnMap.emplace("Actor", &Actor::SpawnWithProperties);
	mActorSpawnMap.emplace("KillVolume", &KillVolume::SpawnWithProperties);
	mActorSpawnMap.emplace("Player", &Player::SpawnWithProperties);

	// Component spawn map
	mCompSpawnMap.emplace("BoxComponent", 
		ComponentInfo(BoxComponent::StaticType(), &BoxComponent::CreateWithProperties));
	mCompSpawnMap.emplace("CameraComponent",
		ComponentInfo(CameraComponent::StaticType(), &CameraComponent::CreateWithProperties));
	mCompSpawnMap.emplace("CharacterMoveComponent",
		ComponentInfo(CharacterMoveComponent::StaticType(), &CharacterMoveComponent::CreateWithProperties));
	mCompSpawnMap.emplace("MeshComponent", 
		ComponentInfo(MeshComponent::StaticType(), &MeshComponent::CreateWithProperties));
	mCompSpawnMap.emplace("PointLightComponent",
		ComponentInfo(PointLightComponent::StaticType(), &PointLightComponent::CreateWithProperties));
	mCompSpawnMap.emplace("SkeletalMeshComponent",
		ComponentInfo(SkeletalMeshComponent::StaticType(), &SkeletalMeshComponent::CreateWithProperties));
}

void LevelLoader::Load(const char* fileName)
{
	std::ifstream file(fileName);

	if (!file.is_open())
	{
		SDL_Log("Level file %s not found", fileName);
		return;
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		SDL_Log("Level file %s is not valid JSON", fileName);
		DbgAssert(false, "Level file is not valid JSON!");
		return;
	}

	std::string str = doc["metadata"]["type"].GetString();
	int ver = doc["metadata"]["version"].GetInt();

	// Check the metadata
	if (!doc["metadata"].IsObject() ||
		str != "itplevel" ||
		ver != 1)
	{
		SDL_Log("Level %s is not a know level file format", fileName);
		return;
	}

	// Step 1: Setup any overall world properties
	Vector3 ambientLight;
	GetVectorFromJSON(doc["world"], "ambientLight", ambientLight);
	mGame.GetRenderer().SetAmbientLight(ambientLight);

	// Step 2: Setup the actors (and each of their components)
	const rapidjson::Value& actors = doc["actors"];
	if (actors.IsArray())
	{
		for (rapidjson::SizeType i = 0; i < actors.Size(); i++)
		{
			ActorPtr actorPtr;
			const rapidjson::Value& actorType = actors[i]["type"];
			if (actorType.IsString())
			{
				// Spawn with properties
				const rapidjson::Value& actorProperties = actors[i]["properties"];
				actorPtr = mActorSpawnMap[actorType.GetString()](mGame, actorProperties);

				// Set actor properties
				actorPtr->SetProperties(actorProperties);
				
				
				// Loop through updated components
				const rapidjson::Value& updatedComponents = actors[i]["updatedComponents"];
				if (updatedComponents.IsArray())
				{
					for (rapidjson::SizeType j = 0; j < updatedComponents.Size(); j++)
					{
						// Spawn actor component and set component properties
						const rapidjson::Value& componentType = updatedComponents[j]["type"];
						if (componentType.IsString())
						{
							ComponentPtr componentPtr = actorPtr->GetComponentFromType(
								mCompSpawnMap.at(componentType.GetString()).mType);

							const rapidjson::Value& componentProperties = updatedComponents[j]["properties"];
							componentPtr->SetProperties(componentProperties);
						}
					}
				}

				// Loop through new components
				const rapidjson::Value& newComponents = actors[i]["newComponents"];
				if (newComponents.IsArray())
				{
					for (rapidjson::SizeType j = 0; j < newComponents.Size(); j++)
					{
						std::string componentType;
						std::string update;
						if (GetStringFromJSON(newComponents[j], "type", componentType)
							&& GetStringFromJSON(newComponents[j], "update", update))
						{
							// Get update type
							Component::UpdateType updateType;
							if (update == "PreTick")
								updateType = Component::PreTick;
							else if (update == "PostTick")
								updateType = Component::PostTick;

							// Get properties
							const rapidjson::Value& componentProperties = newComponents[j]["properties"];

							// Create with properties
							mCompSpawnMap.at(componentType).mFunc(
								*actorPtr,
								updateType,
								componentProperties
							);
						}
					}
				}

				// Begin play on actor
				actorPtr->BeginPlay();
			}
		}
	}
}

// Global helper functions
bool GetFloatFromJSON(const rapidjson::Value& inObject, const char* inProperty, float& outFloat)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;
	if (!property.IsDouble())
	{
		return false;
	}

	outFloat = property.GetDouble();
	return true;
}

bool GetIntFromJSON(const rapidjson::Value& inObject, const char* inProperty, int& outInt)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;
	if (!property.IsInt())
	{
		return false;
	}

	outInt = property.GetInt();
	return true;
}

bool GetStringFromJSON(const rapidjson::Value& inObject, const char* inProperty, std::string& outStr)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;
	if (!property.IsString())
	{
		return false;
	}

	outStr = property.GetString();
	return true;
}

bool GetBoolFromJSON(const rapidjson::Value& inObject, const char* inProperty, bool& outBool)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;
	if (!property.IsBool())
	{
		return false;
	}

	outBool = property.GetBool();
	return true;
}

bool GetVectorFromJSON(const rapidjson::Value& inObject, const char* inProperty, Vector3& outVector)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;
	if (!property.IsArray() || property.Size() != 3)
	{
		return false;
	}

	for (rapidjson::SizeType i = 0; i < 3; i++)
	{
		if (!property[i].IsDouble())
		{
			return false;
		}
	}

	outVector.x = property[0].GetDouble();
	outVector.y = property[1].GetDouble();
	outVector.z = property[2].GetDouble();

	return true;
}

bool GetQuaternionFromJSON(const rapidjson::Value& inObject, const char* inProperty, Quaternion& outQuat)
{
	auto itr = inObject.FindMember(inProperty);
	if (itr == inObject.MemberEnd())
	{
		return false;
	}

	auto& property = itr->value;

	for (rapidjson::SizeType i = 0; i < 4; i++)
	{
		if (!property[i].IsDouble())
		{
			return false;
		}
	}

	outQuat.x = property[0].GetDouble();
	outQuat.y = property[1].GetDouble();
	outQuat.z = property[2].GetDouble();
	outQuat.w = property[3].GetDouble();

	return true;
}
