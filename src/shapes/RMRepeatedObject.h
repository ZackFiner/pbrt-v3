
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_RMRepeatedObject_H
#define PBRT_SHAPES_RMRepeatedObject_H
#pragma once

#include "shapes/raymarcher.h"
namespace pbrt {

#define DEFAULT_WATERPOOL_LENGTH 5.0f
#define DEFAULT_WATERPOOL_WIDTH 5.0f
#define DEFAULT_WATERPOOL_HEIGHT 5.0f
#define DEFAULT_WATERPOOL_OCTAVE 3
#define DEFAULT_WATERPOOL_AMPLITUDE 0.05f
class RMRepeatedObject : public RayMarcher {
  public:
    RMRepeatedObject(const Transform *ObjectToWorld, const Transform *WorldToObject,
              bool reverseOrientation, Float normalEPS, Float hitEPS,
              Float maxMarchDist, int maxRaySteps, Float phiMax)
        : RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f,
                     -1.0f, 100000.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps,
                     phiMax) {}
    Float repSDF(const Point3f &pos) const;
    Float sdf(const Point3f &pos) const;
    Bounds3f ObjectBound() const;

};
std::shared_ptr<Shape> CreateRMRepeatedObjectShape(const Transform *o2w,
                                            const Transform *w2o,
                                            bool reverseOrientation,
                                            const ParamSet &params);
}  // namespace pbrt
#endif