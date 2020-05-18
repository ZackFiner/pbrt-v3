#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_FRAC_MANDELEBULB_H
#define PBRT_SHAPES_FRAC_MANDELEBULB_H
#pragma once

#define DEFAULT_MANDEL_POW 8.0f
#define DEFAULT_MANDEL_BAILOUT 10.0f
#define DEFAULT_MANDEL_ITERATIONS 20

#include "shapes/raymarcher.h"

/*********************************************************
 *	FILENAME: shapes/fractals/mandebulbfractal.h
 *  AUTHOR: Zackary Finer
 * 
 * Description: Provides implementation of mandelbulb for
 * PBRT renderer
 * 
 *********************************************************/
namespace pbrt {

	class MandelbulbFractal : public RayMarcher {
	public:
		MandelbulbFractal(const Transform *ObjectToWorld,
			const Transform *WorldToObject,
			bool reverseOrientation, Float normalEPS, Float hitEPS,
			Float maxMarchDist, int maxRaySteps, Float phiMax, Float power, Float bailoutRadius,
			int mandelIterations)
			: RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f,
				-2.0f, 2.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps,
				phiMax),
			power(power),
			bailoutRadius(bailoutRadius),
			mandelIterations(mandelIterations)
		{}
		Float sdf(const Point3f &pos, Vector3f * trap) const;
		Float sdf(const Point3f &pos) const;
		Vector3f computeOrbitTrap(const Vector3f& v) const;
		
		//Bounds3f ObjectBound() const;

	private:
		Float power, bailoutRadius;
		int mandelIterations;
	};
	std::shared_ptr<Shape> CreateMandelbulbFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params);
}  // namespace pbrt
#endif