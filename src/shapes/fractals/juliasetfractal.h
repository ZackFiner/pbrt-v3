#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_FRAC_JULIASET_H
#define PBRT_SHAPES_FRAC_JULIASET_H
#pragma once

#include "shapes/raymarcher.h"

#define DEFAULT_JULIA_BAILOUT 10.0f
#define DEFAULT_JULIA_ITERATIONS 20
#define DEFAULT_JULIA_RCONST -0.5f
#define DEFAULT_JULIA_ICONST Vector3f(-0.4f, 0.0f, 0.0f)

namespace pbrt {

	class JuliaSetFractal : public RayMarcher {
	public:
		JuliaSetFractal(const Transform *ObjectToWorld,
			const Transform *WorldToObject,
			bool reverseOrientation, Float normalEPS, Float hitEPS,
			Float maxMarchDist, int maxRaySteps, Float phiMax, Float bailoutRadius,
			int juliaIterations, Float realConst, Vector3f imgConst)
			: RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f,
				-1.0f, 1.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps,
				phiMax),
			bailoutRadius(bailoutRadius),
			juliaIterations(juliaIterations),
			constant()
		{
			constant.w = realConst;
			constant.v = imgConst;
		}
		Float sdf(const Point3f &pos, Vector3f* trap) const;
		Float sdf(const Point3f &pos) const;
		Bounds3f ObjectBound() const;

	private:
		Float bailoutRadius;
		Quaternion constant;
		int juliaIterations;
	};
	std::shared_ptr<Shape> CreateJuliaSetFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params);
}  // namespace pbrt
#endif