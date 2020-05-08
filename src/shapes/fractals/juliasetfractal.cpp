#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "shapes/fractals/juliasetfractal.h"
#include "stats.h"

namespace pbrt {
	/*
	 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (V): The Mandelbulb & Different DE Approximations [Blog Post]. Retrieved from
	 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
	 *
	 * Also Retrieved from https://www.iquilezles.org/www/articles/juliasets3d/juliasets3d.htm
	 * 
	 * Also https://www.shadertoy.com/view/MsfGRr
	 *
	 * Like the mandelbrot set, the Julia set is an escape time fractal utilizing complex numbers. The formula for the
	 * julia set is similar to the mandelbrot set with one exception: instead of adding our complex term z, we add a constant
	 * term d at each step:
	 * zn = (zn-1)^2 + d
	 * where c (our test point) is only used as the input for z0 = c
	 * Where each d gives a differnt julia set.
	 *
	 * Given this definition, if we define r as
	 * r = |z| = |fn(c)| = |fn-1(c)^2 + d|, f0(c) = c
	 * then dr can be defined as
	 * f'n(c) = 2fn-1(c)f'n-1(c) (the constant term d is eliminated via differentiation)
	 *
	 * We defined squaring operations for our mandelbrot sdf earlier, so we will simply use this same representation
	 * here for julia sets as a test. We'll set d to be some arbitrary constant, and allow the user
	 */

	Quaternion multQuat(const Quaternion& quat0, const Quaternion& quat1) {
		Float q0 = quat0.w, q1 = quat0.v.x, q2 = quat0.v.y, q3 = quat0.v.z;
		Float r0 = quat1.w, r1 = quat1.v.x, r2 = quat1.v.y, r3 = quat1.v.z;
		Quaternion ret;

		ret.w   = r0*q0 - r1*q1 - r2*q2 - r3*q3;
		ret.v.x = r0*q1 + r1*q0 - r2*q3 + r3*q2;
		ret.v.y = r0*q2 + r1*q3 + r2*q0 - r3*q1;
		ret.v.z = r0*q3 - r1*q2 + r2*q1 + r3*q0;

		return ret;
	}

	// http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.101.9183&rep=rep1&type=pdf
	Quaternion sqrQuat2(const Quaternion& q) {
		Float q0 = q.w;
		Float q1 = q.v.x;
		Float q2 = q.v.y;
		Float q3 = q.v.z;
		Quaternion ret;
		ret.w = q0*q0 - q1*q1 - q2*q2 - q3*q3;
		ret.v.x = 2 * q0*q1;
		ret.v.y = 2 * q0*q2;
		ret.v.z = 2 * q0*q3;
		return ret;
	}

	Float quatLength2(const Quaternion& q) {
		Float q0 = q.w;
		Float q1 = q.v.x;
		Float q2 = q.v.y;
		Float q3 = q.v.z;

		return q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3;
	}

	//http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.101.9183&rep=rep1&type=pdf
	Float JuliaSetFractal::sdf(const Point3f &pos, Vector3f *trap) const {
		*trap = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);

		Quaternion z;
		z.v = Vector3f(pos.y, pos.z, 0);
		z.w = pos.x;


		Float dr = 1.0f; 
		Float r = 0.0f; // we set it to z.length ahead
		// for our iterative step, we approximate r and dr
		for (int i = 0; i < juliaIterations; i++) {
			
			r = std::sqrt(quatLength2(z)); // THIS NEEDS OPTIMIZATION, i've seen implementations that avoid using sqrts up until the end
			dr = 2.0f*r*dr;
			z = sqrQuat2(z) + constant;


			trap->x = trap->x > std::abs(z.w) ? std::abs(z.w) : trap->x;
			trap->y = trap->y > std::abs(z.v.x) ? std::abs(z.v.x) : trap->y;
			trap->z = trap->z > std::abs(z.v.y) ? std::abs(z.v.y) : trap->z;


			if (r > bailoutRadius) break; // terminate execution if we escape
		}
		
		return 0.5f*log(r)*r / dr; // use our distance estimation formula to determine distance to surface
		/*
		 * One thing we must take into consideration is how to track the path of the test point,
		 * and feed it to a BSDF, since this will be needed for our algorithmic shading techniques:
		 * We may need to modify the structure of our class, and the surface intersection class
		 * to accomodate these new requirements.
		 *
		 * One technique could be to use the UV coordinates for the surface to pass this information, this would offer atleast 2 floating
		 * point numbers, and would not cause any problems because we have no plans to add texture support to escape time fractals at this time.
		 */
	}

	Float JuliaSetFractal::sdf(const Point3f &pos) const {
		Quaternion z;
		z.v = Vector3f(pos.y, pos.z, 0);
		z.w = pos.x;


		Float dr = 1.0f;
		Float r = 0.0f; // we set it to z.length ahead
		// for our iterative step, we approximate r and dr
		for (int i = 0; i < juliaIterations; i++) {

			r = std::sqrt(quatLength2(z)); // THIS NEEDS OPTIMIZATION, i've seen implementations that avoid using sqrts up until the end
			dr = 2.0f*r*dr;
			z = sqrQuat2(z) + constant;
					   
			if (r > bailoutRadius) break; // terminate execution if we escape
		}

		return 0.5f*log(r)*r / dr; // use our distance estimation formula to determine distance to surface
		/*
		 * One thing we must take into consideration is how to track the path of the test point,
		 * and feed it to a BSDF, since this will be needed for our algorithmic shading techniques:
		 * We may need to modify the structure of our class, and the surface intersection class
		 * to accomodate these new requirements.
		 *
		 * One technique could be to use the UV coordinates for the surface to pass this information, this would offer atleast 2 floating
		 * point numbers, and would not cause any problems because we have no plans to add texture support to escape time fractals at this time.
		 */
	}

	Bounds3f JuliaSetFractal::ObjectBound() const {
		return Bounds3f(Point3f(-20, -20, -20), Point3f(20, 20, 20));
	}

	std::shared_ptr<Shape> CreateJuliaSetFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params) {

		// add parameters for iteration numbers, and possibly for the conditional folding planes and 'focus' point (incase we want to adjust the shape of our fractal)

		Float bailoutRadius = params.FindOneFloat("bailoutRadius", DEFAULT_JULIA_BAILOUT);
		int juliaIterations = params.FindOneInt("juliaIterations", DEFAULT_JULIA_ITERATIONS);
		Float w = params.FindOneFloat("realConstant", DEFAULT_JULIA_RCONST);
		Vector3f imaginary = params.FindOneVector3f("imaginaryConstants", DEFAULT_JULIA_ICONST);


		Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
		Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
		Float maxMarchDist =
			params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
		int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
		Float phimax = params.FindOneFloat("phimax", 360.f);
		return std::make_shared<JuliaSetFractal>(
			o2w, w2o, reverseOrientation, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phimax, bailoutRadius, juliaIterations, w, imaginary);
	}

} // namespace pbrt