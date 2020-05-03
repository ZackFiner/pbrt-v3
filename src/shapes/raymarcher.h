
/*
    pbrt source code is Copyright(c) 1998-2016
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

// Modification to Sphere Shape to Implement RayMarching
// by Kevin M. Smith 3-2-2019
//

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_SHAPES_RAYMARCHER_H
#define PBRT_SHAPES_RAYMARCHER_H

// shapes/sphere.h*
#include "shape.h"

namespace pbrt {

#define DEFAULT_MAX_RAY_STEPS 1000
#define DEFAULT_DIST_THRESHOLD .01f
#define DEFAULT_MAX_DISTANCE 100.0f
#define DEFAULT_NORMAL_EPS .01f
// Done.
// Sphere Declarations
class RayMarcher : public Shape {
  public:
    // Sphere Public Methods
    RayMarcher(const Transform *ObjectToWorld, const Transform *WorldToObject,
           bool reverseOrientation, Float radius, Float zMin, Float zMax,
		   Float normalEPS, Float hitEPS, Float maxMarchDist, int maxRaySteps,
           Float phiMax)
        : Shape(ObjectToWorld, WorldToObject, reverseOrientation),
          radius(radius),
          zMin(Clamp(std::min(zMin, zMax), -radius, radius)),
          zMax(Clamp(std::max(zMin, zMax), -radius, radius)),
          thetaMin(std::acos(Clamp(std::min(zMin, zMax) / radius, -1, 1))),
          thetaMax(std::acos(Clamp(std::max(zMin, zMax) / radius, -1, 1))),
          phiMax(Radians(Clamp(phiMax, 0, 360))),
          normalEPS(normalEPS),
          hitEPS(hitEPS),
          maxMarchDist(maxMarchDist),
          maxRaySteps(maxRaySteps) {}

    Bounds3f ObjectBound() const;
    bool Intersect(const Ray &ray, Float *tHit, SurfaceInteraction *isect,
                   bool testAlphaTexture) const;
    Vector3f GetNormalRM(const Point3f &pos, float eps,
                             const Vector3f &defaultNormal) const;

    virtual Float sdf(const Point3f &pos) const;
	virtual Float sdf(const Point3f &pos, Vector3f * trap) const {
		return sdf(pos); // use our non-trap type if we have no implementation details for this version
	}
	virtual Vector3f computeOrbitTrap(const Vector3f& v) const {
		return v;
	}

    Float Area() const;
    Interaction Sample(const Point2f &u, Float *pdf) const;
    Interaction Sample(const Interaction &ref, const Point2f &u,
                       Float *pdf) const;


  private:
    // RayMarcher Private Data
    const Float radius;
    const Float zMin, zMax;
    const Float normalEPS, hitEPS, maxMarchDist;
    const int maxRaySteps;
    const Float thetaMin, thetaMax, phiMax;
};

std::shared_ptr<Shape> CreateRayMarcherShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params);

}  // namespace pbrt

#endif  // PBRT_SHAPES_RAYMARCHER_H
