#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_FRAC_JULIASET_H
#define PBRT_SHAPES_FRAC_JULIASET_H
#pragma once

#define DEFAULT_JULIA_POW 2.0f
#define DEFAULT_JULIA_BAILOUT 10.0f
#define DEFAULT_JULIA_ITERATIONS 20

#include "shapes/raymarcher.h"
namespace pbrt {

	class JuliaSetFractal : public RayMarcher {
	public:
		JuliaSetFractal(const Transform *ObjectToWorld,
			const Transform *WorldToObject,
			bool reverseOrientation, Float normalEPS, Float hitEPS,
			Float maxMarchDist, int maxRaySteps, Float phiMax, Float power, Float bailoutRadius,
			int juliaIterations)
			: RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f,
				-1.0f, 1.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps,
				phiMax),
			power(power),
			bailoutRadius(bailoutRadius),
			juliaIterations(juliaIterations)
		{}
		Float sdf(const Point3f &pos, Vector3f* trap) const;
		Float sdf(const Point3f &pos) const;
		Bounds3f ObjectBound() const;

	private:
		Float power, bailoutRadius;
		int juliaIterations;
	};
	std::shared_ptr<Shape> CreateJuliaSetFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params);
}  // namespace pbrt
#endif