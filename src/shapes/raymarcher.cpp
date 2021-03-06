
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


// shapes/sphere.cpp*
#include "shapes/raymarcher.h"
#include "sampling.h"
#include "paramset.h"
#include "efloat.h"
#include "stats.h"

namespace pbrt {

// Sphere Method Definitions
Bounds3f RayMarcher::ObjectBound() const {
    return Bounds3f(Point3f(-boundsnX, -boundsnY, -boundsnZ),
                    Point3f(boundsX, boundsY, boundsZ));
}




//  Template Method
//
bool RayMarcher::Intersect(const Ray &r, Float *tHit,
                                  SurfaceInteraction *isect,
                                  bool testAlphaTexture) const {
    // Transform _Ray_ to object space
    Vector3f oErr, dErr;
    Ray ray = (*WorldToObject)(r, &oErr, &dErr);
    Vector3f dir = Normalize(ray.d);  // ray direction vectors are not
                                      // normalized in PBRT by default (KMS) 
	bool hit = false;
    Point3f origin = ray.o;
    //static int counter = 0;
    //printf("%d", counter);
    Float t = 0.0f;
    Float lastDist;
	int i;
	Vector3f orbitTrapVec;
	if (sdf(origin) >= 0.0f)
    for (i = 0; i < maxRaySteps; i++) {  // ray marching happens here
        lastDist = sdf(origin + dir * t);
        if (t < 0.0f || std::abs(t) > maxMarchDist) { 
                break;
        } else if (std::abs(lastDist) < hitEPS) {
            hit = true;
            break;
        } else {
            t += lastDist;
		}
	}



	if (hit && tHit != nullptr && isect != nullptr) {
		
		// This where you return your SurfaceInteraction structure and your tHit
		// Important Note: You must check for null pointer as Intersect is called 
		// by IntersectP() with null values for these parameters.
        
		
		auto pHit = origin + dir * t;
                auto pError = Vector3f(hitEPS * 10.0f, hitEPS * 10.0f, hitEPS * 10.0f);
                auto aproximatedNorm =
                    GetNormalRM(pHit, normalEPS, Vector3f(0.0f, 0.0f, 1.0f));
		Point3f orbitTestPoint = Point3f(pHit + aproximatedNorm * 2.0f);
	    sdf(orbitTestPoint, &orbitTrapVec);
        Vector3f dpdu, dpdv;
		CoordinateSystem(aproximatedNorm, &dpdu, &dpdv); // cordinate system will generate a coordinate sytem using our normal
        *isect = (*ObjectToWorld)(SurfaceInteraction( // we've been working in local space this entire time (notice how we don't store pos or rot params in this class)
            pHit, pError, Point2f(0, 0), -ray.d, dpdu, dpdv,
            Normal3f(0.0f, 0.0f, 0.0f), Normal3f(0.0f, 0.0f, 0.0f), ray.time,
            this));
		isect->rayMarchSteps = i;
		isect->orbitTrap = computeOrbitTrap(orbitTrapVec);

		*tHit = t; // we set our distance to point of incidence to be the distance we found while marching the ray
	}
    return hit;
}


//  Template Method
//
Float RayMarcher::sdf(const Point3f &pos) const { 
	Float length = (pos-Point3f(0.0f,0.0f,0.0f)).Length(); // the subtraction is to 'cast' our point to vec3 so we can take the length
    return length - radius; // we will assume our radius is 1.0 for now
}



// Get Normal using Gradient (Finite Distances Methods )  - See class slides.
//  Note if the normal you calculate has zero length, return the defaultNormal
//
Vector3f RayMarcher::GetNormalRM( const Point3f &p, float eps, const Vector3f &defaultNormal) const {
    Float dp = sdf(p);
    Vector3f AproxNorm = Vector3f(dp - sdf(Point3f(p.x - eps, p.y, p.z)),
                                  dp - sdf(Point3f(p.x, p.y - eps, p.z)),
                                  dp - sdf(Point3f(p.x, p.y, p.z - eps)));
    return AproxNorm.LengthSquared() > 0.0f ? AproxNorm : defaultNormal; // if our normal has a zero length, return the default
}


Float RayMarcher::Area() const { return phiMax * radius * (zMax - zMin); }

// These functions are stubbed
//
Interaction RayMarcher::Sample(const Point2f &u, Float *pdf) const {
    LOG(FATAL) << "RayMarcher::Sample not implemented.";
    return Interaction();
}

Interaction RayMarcher::Sample(const Interaction &ref, const Point2f &u,
                           Float *pdf) const {
    LOG(FATAL) << "RayMarcher::Sample not implemented.";
    return Interaction();
}

std::shared_ptr<Shape> CreateRayMarcherShape(const Transform *o2w,
                                         const Transform *w2o,
                                         bool reverseOrientation,
                                         const ParamSet &params) {
    Float radius = params.FindOneFloat("radius", 1.f); 
    Float zmin = params.FindOneFloat("zmin", -radius);
    Float zmax = params.FindOneFloat("zmax", radius);
    Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
    Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
    Float maxMarchDist = params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
    int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
    Float phimax = params.FindOneFloat("phimax", 360.f);
    
	return std::make_shared<RayMarcher>(o2w, w2o, reverseOrientation, radius, zmin,
                                    zmax, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phimax);
}

}  // namespace pbrt
