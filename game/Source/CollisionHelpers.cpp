#include "ITPEnginePCH.h"

namespace Collision
{

	bool Intersects(const Sphere& a, const Sphere& b)
	{
		Vector3 diff = a.mCenter - b.mCenter;
		float DistSq = diff.LengthSq();
		float sumRadiiSq = (a.mRadius + b.mRadius) * (a.mRadius + b.mRadius);
		if (DistSq <= sumRadiiSq)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	bool Intersects(const AxisAlignedBox & a, const AxisAlignedBox & b)
	{
		return !(
			a.mMin.x > b.mMax.x || b.mMin.x > a.mMax.x ||
			a.mMin.y > b.mMax.y || b.mMin.y > a.mMax.y ||
			a.mMin.z > b.mMax.z || b.mMin.z > a.mMax.z
		);
	}

	bool SegmentCast(const LineSegment& segment, const AxisAlignedBox& box, Vector3& outPoint)
	{
		Vector3 d = segment.mEnd - segment.mStart;
		float tmin = 0.0f;
		float tmax = 1.0f;

		// For x slab
		if (fabs(d.x) < FLT_MIN)
		{
			// Ray is parallel to slab. No hit if origin not within slab.
			if (segment.mStart.x < box.mMin.x || segment.mStart.x > box.mMax.x)
			{
				return false;
			}
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / d.x;
			float t1 = (box.mMin.x - segment.mStart.x) * ood;
			float t2 = (box.mMax.x - segment.mStart.x) * ood;

			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2)
			{
				// swap t1 and t2
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			// Compute the intersection of slab intersection intervals
			tmin = max(tmin, t1);
			tmax = min(tmax, t2);

			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}

		// For y slab
		if (fabs(d.y) < FLT_MIN)
		{
			// Ray is parallel to slab. No hit if origin not within slab.
			if (segment.mStart.y < box.mMin.y || segment.mStart.y > box.mMax.y)
			{
				return false;
			}
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / d.y;
			float t1 = (box.mMin.y - segment.mStart.y) * ood;
			float t2 = (box.mMax.y - segment.mStart.y) * ood;

			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2)
			{
				// swap t1 and t2
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			// Compute the intersection of slab intersection intervals
			tmin = max(tmin, t1);
			tmax = min(tmax, t2);

			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax) return false;
		}

		// For z slab
		if (fabs(d.z) < FLT_MIN)
		{
			// Ray is parallel to slab. No hit if origin not within slab.
			if (segment.mStart.z < box.mMin.z || segment.mStart.z > box.mMax.z)
			{
				return false;
			}
		}
		else
		{
			// Compute intersection t value of ray with near and far plane of slab
			float ood = 1.0f / d.z;
			float t1 = (box.mMin.z - segment.mStart.z) * ood;
			float t2 = (box.mMax.z - segment.mStart.z) * ood;

			// Make t1 be intersection with near plane, t2 with far plane
			if (t1 > t2)
			{
				// swap t1 and t2
				float temp = t1;
				t1 = t2;
				t2 = temp;
			}

			// Compute the intersection of slab intersection intervals
			tmin = max(tmin, t1);
			tmax = min(tmax, t2);

			// Exit with no collision as soon as slab intersection becomes empty
			if (tmin > tmax)
			{
				return false;
			}
		}

		// Ray intersects all 3 slabs. Return point of intersection.
		outPoint = segment.mStart + d * tmin;
		return true;
	}

} // namespace
