#include "ITPEnginePCH.h"
#include <SDL/SDL_log.h>

Animation::Animation(class Game& game)
	:Asset(game)
{

}

bool Animation::Load(const char* fileName, class AssetCache* cache)
{
	std::ifstream file(fileName);
	if (!file.is_open())
	{
		SDL_Log("File not found: Animation %s", fileName);
		return false;
	}

	std::stringstream fileStream;
	fileStream << file.rdbuf();
	std::string contents = fileStream.str();
	rapidjson::StringStream jsonStr(contents.c_str());
	rapidjson::Document doc;
	doc.ParseStream(jsonStr);

	if (!doc.IsObject())
	{
		SDL_Log("Animation %s is not valid json", fileName);
		return false;
	}

	std::string str = doc["metadata"]["type"].GetString();
	int ver = doc["metadata"]["version"].GetInt();

	// Check the metadata
	if (!doc["metadata"].IsObject() ||
		str != "itpanim" ||
		ver != 2)
	{
		SDL_Log("Animation %s unknown format", fileName);
		return false;
	}

	const rapidjson::Value& sequence = doc["sequence"];
	if (!sequence.IsObject())
	{
		SDL_Log("Animation %s doesn't have a sequence.", fileName);
		return false;
	}

	const rapidjson::Value& frames = sequence["frames"];
	const rapidjson::Value& length = sequence["length"];
	const rapidjson::Value& bonecount = sequence["bonecount"];

	if (!frames.IsUint() || !length.IsDouble() || !bonecount.IsUint())
	{
		SDL_Log("Sequence %s has invalid frames, length, or bone count.", fileName);
		return false;
	}

	mNumFrames = frames.GetUint();
	mLength = length.GetDouble();
	mNumBones = bonecount.GetUint();

	mTracks.resize(mNumBones);

	const rapidjson::Value& tracks = sequence["tracks"];

	if (!tracks.IsArray())
	{
		SDL_Log("Sequence %s missing a tracks array.", fileName);
		return false;
	}

	for (rapidjson::SizeType i = 0; i < tracks.Size(); i++)
	{
		if (!tracks[i].IsObject())
		{
			SDL_Log("Animation %s: Track element %d is invalid.", fileName, i);
			return false;
		}

		size_t boneIndex = tracks[i]["bone"].GetUint();

		const rapidjson::Value& transforms = tracks[i]["transforms"];
		if (!transforms.IsArray())
		{
			SDL_Log("Animation %s: Track element %d is missing transforms.", fileName, i);
			return false;
		}

		BoneTransform temp;

		if (transforms.Size() != mNumFrames)
		{
			SDL_Log("Animation %s: Track element %d has fewer frames than expected.", fileName, i);
			return false;
		}

		for (rapidjson::SizeType j = 0; j < transforms.Size(); j++)
		{
			const rapidjson::Value& rot = transforms[j]["rot"];
			const rapidjson::Value& trans = transforms[j]["trans"];

			if (!rot.IsArray() || !trans.IsArray())
			{
				SDL_Log("Skeleton %s: Bone %d is invalid.", fileName, i);
				return false;
			}

			temp.mRotation.x = rot[0].GetDouble();
			temp.mRotation.y = rot[1].GetDouble();
			temp.mRotation.z = rot[2].GetDouble();
			temp.mRotation.w = rot[3].GetDouble();

			temp.mTranslation.x = trans[0].GetDouble();
			temp.mTranslation.y = trans[1].GetDouble();
			temp.mTranslation.z = trans[2].GetDouble();

			mTracks[boneIndex].emplace_back(temp);
		}
	}

	return true;
}

void Animation::GetGlobalPoseAtTime(std::vector<SimdMatrix4>& outPoses, SkeletonPtr inSkeleton, float inTime)
{
	float durationPerFrame = mLength / static_cast<float> (mNumFrames - 1);

	// Closest frames before and after inTime
	int frame = static_cast<int> (inTime / durationPerFrame);

	// Percent between frameA and frameB
	float f = (inTime - static_cast<float> (frame) * durationPerFrame) / durationPerFrame;

	// Global pose for first element
	outPoses.emplace_back(Interpolate(mTracks[0][frame], mTracks[0][frame + 1], f).ToSimdMatrix());

	// Global pose for all other elements
	std::vector<SimdMatrix4> globalInvBindPoses = inSkeleton->GetGlobalInvBindPoses();
	for (unsigned int i = 1; i < mNumBones; ++i)
	{
		// If bone doesn't change during the course of the animation, use global bind pose
		if (mTracks[i].empty())
		{
			SimdMatrix4 bindPoseMatrix = globalInvBindPoses[i];
			bindPoseMatrix.Invert();
			outPoses.emplace_back(bindPoseMatrix);
			continue;
		}

		// Local pose matrix
		SimdMatrix4 mat = Interpolate(mTracks[i][frame], mTracks[i][frame + 1], f).ToSimdMatrix();

		// Global pose matrix
		mat.Mul(outPoses[inSkeleton->GetBone(i).mParent]);
		outPoses.emplace_back(mat);
	}
}

void Animation::GetBlendedGlobalPoseAtTime(std::vector<SimdMatrix4>& outPoses, SkeletonPtr inSkeleton, std::shared_ptr<Animation> inPrevAnim, float inTime, float inPrevTime, float inBlendTime)
{
	// Clip A
	float clipA_durationPerFrame = inPrevAnim->mLength / static_cast<float> (inPrevAnim->mNumFrames - 1);
	int clipA_frame = static_cast<int> (inPrevTime / clipA_durationPerFrame);
	float clipA_f = (inPrevTime - static_cast<float> (clipA_frame) * clipA_durationPerFrame) / clipA_durationPerFrame;

	// ClipB
	float clipB_durationPerFrame = mLength / static_cast<float> (mNumFrames - 1);
	int clipB_frame = static_cast<int> (inTime / clipB_durationPerFrame);
	float clipB_f = (inTime - static_cast<float> (clipB_frame) * clipB_durationPerFrame) / clipB_durationPerFrame;

	// Blend factor
	float f = inTime / min(inBlendTime, mLength);
	float blendFactor = f * f * (3 - 2 * f);

	// Global pose for first element
	outPoses.emplace_back(Interpolate(
		Interpolate(inPrevAnim->mTracks[0][clipA_frame], inPrevAnim->mTracks[0][clipA_frame + 1], clipA_f),
		Interpolate(mTracks[0][clipB_frame], mTracks[0][clipB_frame + 1], clipB_f), blendFactor).ToSimdMatrix());

	// Global pose for all other elements
	std::vector<SimdMatrix4> globalInvBindPoses = inSkeleton->GetGlobalInvBindPoses();
	for (unsigned int i = 1; i < mNumBones; ++i)
	{
		// If bone doesn't change during the course of the animation, use global bind pose
		if (mTracks[i].empty())
		{
			SimdMatrix4 bindPoseMatrix = globalInvBindPoses[i];
			bindPoseMatrix.Invert();
			outPoses.emplace_back(bindPoseMatrix);
			continue;
		}

		// Local pose
		BoneTransform boneTransform = Interpolate(
			Interpolate(inPrevAnim->mTracks[i][clipA_frame], inPrevAnim->mTracks[i][clipA_frame + 1], clipA_f),
			Interpolate(mTracks[i][clipB_frame], mTracks[i][clipB_frame + 1], clipB_f), blendFactor);

		// Global pose matrix
		SimdMatrix4 globalPoseMatrix = boneTransform.ToSimdMatrix();
		globalPoseMatrix.Mul(outPoses[inSkeleton->GetBone(i).mParent]);
		outPoses.emplace_back(globalPoseMatrix);
	}
}