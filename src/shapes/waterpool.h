
#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_WATERPOOL_H
#define PBRT_SHAPES_WATERPOOL_H
#pragma once


#include "shapes/raymarcher.h"
namespace pbrt {

#define DEFAULT_WATERPOOL_LENGTH 5.0f
#define DEFAULT_WATERPOOL_WIDTH 5.0f
#define DEFAULT_WATERPOOL_HEIGHT 5.0f
#define DEFAULT_WATERPOOL_OCTAVE 3
#define DEFAULT_WATERPOOL_AMPLITUDE 0.05f
class WaterPool : public RayMarcher
{
  public:
    WaterPool(const Transform *ObjectToWorld, const Transform *WorldToObject,
              bool reverseOrientation, Float length, Float width, Float height, int octave, Float amplitude, Float normalEPS,
              Float hitEPS, Float maxMarchDist, int maxRaySteps, Float phiMax)
        : RayMarcher(ObjectToWorld, WorldToObject, reverseOrientation, 1.0f, -1.0f, 1.0f, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phiMax),
		length(length),
        width(width),
		height(height),
        octave(octave),
        amplitude(amplitude)
		{}
    Float sdf(const Point3f &pos) const;
    Bounds3f ObjectBound() const;
  private:
    Float length, width, height;
	Float amplitude;
    int octave;
};
std::shared_ptr<Shape> CreateWaterPoolShape(const Transform *o2w,
                                                const Transform *w2o,
                                                bool reverseOrientation,
                                                const ParamSet &params);
}  // namespace PBRT
#endif