#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "shapes/fractals/juliasetfractal.h"
#include "stats.h"

/*********************************************************
 *	FILENAME: shapes/fractals/juliasetfractal.cpp
 *  AUTHOR: Zackary Finer
 *
 * Description: provides an implementation of a julia set
 * for PBRT renderer
 *
 *********************************************************/

namespace pbrt {


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

   /*
	*Odegaard, T., &Wennergren, J. (2007).Raytracing 4D fractals, visualizing the four dimensional properties of the Julia set.
	*/
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

	/*******************************************************************************************************************************************************
	 * References:
	 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (V): The Mandelbulb & Different DE Approximations [Blog Post]. Retrieved from
	 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
	 *
	 * Odegaard, T., & Wennergren, J. (2007). Raytracing 4D fractals, visualizing the four dimensional properties of the Julia set.
	 *
	 * Hart, J. C., Sandin, D. J., & Kauffman, L. H. (1989). Ray tracing deterministic 3-D fractals. ACM SIGGRAPH Computer Graphics, 23(3), 289-296.
	 * doi:10.1145/74334.74363
	 *
	 * Descrption:
	 * Like the mandelbrot set, the Julia set is an escape time fractal utilizing complex numbers. The formula for the
	 * julia set is similar to the mandelbrot set with one exception: instead of adding our complex term z, we add a constant
	 * term d at each step:
	 * zn = (zn-1)^2 + d
	 * where c (our test point) is only used as the input for z0 = c
	 * Each d gives a differnt julia set, and serves as a kind of "seed" for our shape.
	 *
	 * Given this definition, if we define r as
	 * r = |z| = |fn(c)| = |fn-1(c)^2 + d|, f0(c) = c
	 * then dr can be defined as
	 * f'n(c) = 2fn-1(c)f'n-1(c) (the constant term d is eliminated via differentiation)
	 *
	 * The question then becomes how we can represent the squaring operation. We could use the same approach as the mandelbulb (with the double angle,
	 * multiply the radius interpretation), but a common approach is to represent our test point as a quaternion and use quaternion multiplication to square
	 * the point each iteration (See Hart, Christensen). The question then becomes how we intialize our starting quaternion using the cartesian point pos.
	 *
	 * For this render, we will set the real component of the quaternion to the x component of pos (Odegaard, 2007), and set the imaginary components
	 * x and y to pos.y and pos.z respectively. Since quaternions are 4 dimensional, and we are rendering something in 3 dimension space, we are only
	 * rendering a "slice" of a 4 dimensional object. We can adjust the position of the slice by adjusting the initial imaginary z component of starting
	 * quaternion
	 *
	 *******************************************************************************************************************************************************/
	Float JuliaSetFractal::sdf(const Point3f &pos, Vector3f *trap) const {
		*trap = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);

		Quaternion z;
		z.v = Vector3f(pos.y, pos.z, 0);
		z.w = pos.x;
		Float debugLength = std::sqrt(quatLength2(z))*0.5f;

		Float dr = 1.0f; 
		Float r = std::sqrt(quatLength2(z));

		// for our iterative step, we approximate r and dr
		for (int i = 0; i < juliaIterations; i++) {
			
			dr = 2.0f*r*dr;
			z = sqrQuat2(z) + constant;
			r = std::sqrt(quatLength2(z));


			trap->x = trap->x > std::abs(z.w) ? std::abs(z.w) : trap->x;
			trap->y = trap->y > std::abs(z.v.x) ? std::abs(z.v.x) : trap->y;
			trap->z = trap->z > std::abs(z.v.y) ? std::abs(z.v.y) : trap->z;


			if (r > bailoutRadius) break; // terminate execution if we escape
		}
		
		return Clamp(0.5f*log(r)*r / dr, -debugLength, debugLength); // use our distance estimation formula to determine distance to surface
	}

	/*******************************************************************************************************************************************************
	 * References:
	 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (V): The Mandelbulb & Different DE Approximations [Blog Post]. Retrieved from
	 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
	 *
	 * Odegaard, T., & Wennergren, J. (2007). Raytracing 4D fractals, visualizing the four dimensional properties of the Julia set.
	 *
	 * Hart, J. C., Sandin, D. J., & Kauffman, L. H. (1989). Ray tracing deterministic 3-D fractals. ACM SIGGRAPH Computer Graphics, 23(3), 289-296.
	 * doi:10.1145/74334.74363
	 *
	 *******************************************************************************************************************************************************/
	Float JuliaSetFractal::sdf(const Point3f &pos) const {
		Quaternion z;
		z.v = Vector3f(pos.y, pos.z, zSlice);
		z.w = pos.x;
		Float debugLength = std::sqrt(quatLength2(z))*0.5f;

		Float dr = 1.0f;
		Float r = 0.0f; // we set it to z.length ahead
		r = std::sqrt(quatLength2(z)); // THIS NEEDS OPTIMIZATION, i've seen implementations that avoid using sqrts up until the end

		// for our iterative step, we approximate r and dr
		for (int i = 0; i < juliaIterations; i++) {

			dr = 2.0f*r*dr;
			z = sqrQuat2(z) + constant;
			r = std::sqrt(quatLength2(z)); // THIS NEEDS OPTIMIZATION, i've seen implementations that avoid using sqrts up until the end

			if (r > bailoutRadius) break; // terminate execution if we escape
		}
		
		return Clamp(0.5f*log(r)*r / dr, -debugLength, debugLength); // use our distance estimation formula to determine distance to surface
	}
	
	Bounds3f JuliaSetFractal::ObjectBound() const {
		return Bounds3f(Point3f(-5, -5, -5), Point3f(5, 5, 5));
	}
	
	std::shared_ptr<Shape> CreateJuliaSetFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params) {

		Float bailoutRadius = params.FindOneFloat("bailoutRadius", DEFAULT_JULIA_BAILOUT);
		int juliaIterations = params.FindOneInt("juliaIterations", DEFAULT_JULIA_ITERATIONS);
		Float w = params.FindOneFloat("realConstant", DEFAULT_JULIA_RCONST);
		Vector3f imaginary = params.FindOneVector3f("imaginaryConstants", DEFAULT_JULIA_ICONST);
		Float zSlice = params.FindOneFloat("juliaZSlice", DEFAULT_JULIA_ZSLICE);

		Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
		Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
		Float maxMarchDist =
			params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
		int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
		Float phimax = params.FindOneFloat("phimax", 360.f);
		return std::make_shared<JuliaSetFractal>(
			o2w, w2o, reverseOrientation, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phimax, bailoutRadius, juliaIterations, zSlice, w, imaginary);
	}

} // namespace pbrt