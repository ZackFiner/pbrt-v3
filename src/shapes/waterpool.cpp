#include "shapes/waterpool.h"
#include "sampling.h"
#include "paramset.h"
#include "efloat.h"
#include "stats.h"

namespace pbrt {

	Float WaterPool::sdf(const Point3f &pos) const {
		// here is where we actually perform our perlin noise calculation
            Float freq = 1.0f;
            Float _amp = amplitude;
            Float noise = _amp * pbrt::Noise(pos.x * freq, 0.0f, pos.z * freq);
			// we want to establish a limit on the size of our water pool, so we'll first do a bounds check
            //if (std::abs(pos.x) < width * 0.5f && std::abs(pos.z) < length * 0.5f) {
            
				for (int i = 0; i < octave - 1; i++) {
					freq *= 2.0f;
					_amp *= 0.5f;
					noise += _amp * pbrt::Noise(pos.x * freq, 0.0f, pos.z * freq);
				}
				Float diff = pos.y - noise;
				return diff; // > -0.5f ? diff : -diff-0.5f;
            /*} else { // use minimum distance to finite plane
                Float halfwidth = width * 0.5f;
                Float halflength = length * 0.5f;
                Float max_x =
                    std::max(-halfwidth - pos.x, std::max(0.0f, pos.x - halfwidth));
                Float max_z =
                    std::max(-halflength - pos.x, std::max(0.0f, pos.x - halflength));
                max_x = std::abs(max_x) - halfwidth;
                max_z = std::abs(max_z) - halfwidth;
                return sqrtf(max_x * max_x + max_z * max_z + pos.y * pos.y);
			}*/
	}

	Bounds3f WaterPool::ObjectBound() const {
            return Bounds3f(Point3f(-20, -amplitude, -20),
                            Point3f(20, amplitude, 20));
	}

	std::shared_ptr<Shape> CreateWaterPoolShape(const Transform *o2w,
												 const Transform *w2o,
												 bool reverseOrientation,
												 const ParamSet &params) {
		Float length = params.FindOneFloat("length", DEFAULT_WATERPOOL_LENGTH);
		Float width = params.FindOneFloat("width", DEFAULT_WATERPOOL_WIDTH);
		Float height = params.FindOneFloat("height", DEFAULT_WATERPOOL_HEIGHT);
		int octave = params.FindOneInt("octave", DEFAULT_WATERPOOL_OCTAVE);
        Float amplitude = params.FindOneFloat("amplitude", DEFAULT_WATERPOOL_AMPLITUDE);

		Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
		Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
		Float maxMarchDist =
			params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
		int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
		Float phimax = params.FindOneFloat("phimax", 360.f);
		return std::make_shared<WaterPool>(o2w, w2o, reverseOrientation, length, width, height, octave, amplitude, normalEPS, hitEPS,
											maxMarchDist, maxRaySteps, phimax);
	}

}
