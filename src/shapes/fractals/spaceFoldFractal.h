#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_FRAC_SPACEFOLD_H
#define PBRT_SHAPES_FRAC_SPACEFOLD_H
#pragma once

#include "shapes/raymarcher.h"
#define DEFAULT_FOLD_ITERATIONS 20

/*********************************************************
 *	FILENAME: shapes/fractals/spaceFoldFractal.h
 *  AUTHOR: Zackary Finer
 *
 * Description: Provides implementation of seirpinski
 * pyramid fractals for PBRT
 *
 *********************************************************/

namespace pbrt {

class SpaceFoldFractal : public RayMarcher {
  public:
    SpaceFoldFractal(const Transform *ObjectToWorld,
                     const Transform *WorldToObject,
              bool reverseOrientation, Float normalEPS, Float hitEPS,
              Float maxMarchDist, int maxRaySteps, Float phiMax, int foldIterations)
        : RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f,
                     -1.0f, 1.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps,
                     phiMax),
		foldIterations(foldIterations)
	{}
    Float sdf(const Point3f &pos) const;
    //Bounds3f ObjectBound() const;

  private:
	  int foldIterations;
};
std::shared_ptr<Shape> CreateSpaceFoldFractalShape(const Transform *o2w,
                                            const Transform *w2o,
                                            bool reverseOrientation,
                                            const ParamSet &params);
}  // namespace pbrt
#endif