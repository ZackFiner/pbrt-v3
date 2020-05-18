#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "shapes/fractals/mandelbulbfractal.h"
#include "stats.h"

/*********************************************************
 *	FILENAME: shapes/fractals/mandebulbfractal.cpp
 *  AUTHOR: Zackary Finer
 *
 * Description: Provides implementation of mandelbulb for
 * PBRT renderer
 *
 *********************************************************/

namespace pbrt {
	/*******************************************************************************************************************************************************
	 * References:
	 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (V): The Mandelbulb & Different DE Approximations [Blog Post]. Retrieved from
	 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
	 * 
	 * Hart, J. C., Sandin, D. J., & Kauffman, L. H. (1989). Ray tracing deterministic 3-D fractals. ACM SIGGRAPH Computer Graphics, 23(3), 289-296.
	 * doi:10.1145/74334.74363
	 * 
	 * Description:
	 * The mandlebulb, like it's 2d mandlebrot counterpart, is an example of an escape time fractal. Escape time fractals are mathematically
	 * defined by performing some operation on a point (in the case of the mandlebrot set, a complex number) and testing to determine
	 * whether the operation diverges (often via testing to see if it escapes a given radius) or convergese (whether it stays within a given
	 * radius or apporaches a value).
	 *
	 * Unlike concrete shapes like a sphere or a box, an escape time fractal's surface is often infinitly complex, making it impossible for an exact
	 * intersection function to be derived for a point. It is here where the advantages of raymarching come into play, because even though we cannot
	 * derive an intersection function, it is possible to calcualte an approximated distance to surface function.
	 * 
	 * In his article, Christensen describes how a distance estimation formula for the mandelbrot set:
	 * DE = 0.5 * ln(r) * r/dr
	 *
	 * Where r is the escape time length
	 * And dr is the length of the running derivative
	 * 
	 * For example, in the case of the mandelbrot set, we can define the formula
	 * as follows:
	 * fn(c) = fn-1(c)^2 + c, f0(c) = 0 (we square the last result, and then add w/e point we're testing)
	 * then r = |fn(c)| and dr =  |f'n(c)|
	 * With this definition, we can derive dr with respect to c
	 * 
	 * f'n(c) = d/dc(fn-1(c)^2) - d/dc(c)
	 * f'n(c) = 2(fn-1(c))f'n-1(c) + 1 (using chain rule)
	 * f'0(c) = 1, since f0(c) = 0 
	 * now that we know how to calcualte f'n(c), we can use a similar iterative approach
	 * to calcualte dr for our approximation.
	 *
	 * In this class, we implement the sdf for the mandelbulb, but a very similar approach to this
	 * will be taken for the julia set
	 *
	 ********************************************************************************************************************************************************/
	Float MandelbulbFractal::sdf(const Point3f &pos, Vector3f * trap) const {
		Vector3f z = Vector3f(pos);
		const Vector3f vpos = z;
		Float dr = 1.0f;
		Float r = 0.0f;
		const Vector3f point = Vector3f(0.0f, 0.0f, 5.0f);
		*trap = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);

		// for our iterative step, we approximate r and dr
		for (int i = 0; i < mandelIterations; i++) {
			Vector3f last_z = z;
			r = z.Length();
			if (r > bailoutRadius) break; // terminate execution if we escape

			// convert our cartesian position into spherical coordinates
			Float theta = acos(z.z / r);
			Float phi = atan2(z.y, z.x);
			dr = pow(r, power - 1.0f)*power*dr + 1.0f; // our calculation of dr as descirbed above

			// apply the necessary iterative operations to our spherical coordinate (using trig laws, we can express power as a multiple of angle
			// and a power of radius)
			// this is equivalent to fn-1(c)^power
			Float zr = pow(r, power);
			theta = theta * power;
			phi = phi * power;

			// convert it back to cartesian coordiantes so we can add our constant term
			z = zr * Vector3f(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
			z += vpos; // add the constant term + c
			

			// use planar orbit trapping, with a plane on each axis
			trap->x = trap->x > std::abs(z.x) ? std::abs(z.x) : trap->x;
			trap->y = trap->y > std::abs(z.y) ? std::abs(z.y) : trap->y;
			trap->z = trap->z > std::abs(z.z) ? std::abs(z.z) : trap->z;

			
		}

		Float debugLength = Vector3f(pos).Length()*0.5f;


		// use our distance estimation formula to determine distance to surface - we apply a clamp to ensure we don't overstep
		return Clamp(0.5f*log(r)*r / dr, -debugLength, debugLength);
	}

	/*******************************************************************************************************************************************************
	 * References:
	 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (V): The Mandelbulb & Different DE Approximations [Blog Post]. Retrieved from
	 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
	 *
	 * Hart, J. C., Sandin, D. J., & Kauffman, L. H. (1989). Ray tracing deterministic 3-D fractals. ACM SIGGRAPH Computer Graphics, 23(3), 289-296.
	 * doi:10.1145/74334.74363
	 *
	 *******************************************************************************************************************************************************/
	Float MandelbulbFractal::sdf(const Point3f &pos) const {
		Vector3f z = Vector3f(pos);
		const Vector3f vpos = z;
		Float dr = 1.0f;
		Float r = 0.0f;


		// for our iterative step, we approximate r and dr
		for (int i = 0; i < mandelIterations; i++) {

			r = z.Length();
			if (r > bailoutRadius) break; // terminate execution if we escape

			// convert our cartesian position into spherical coordinates
			Float theta = acos(z.z / r);
			Float phi = atan2(z.y, z.x);
			dr = pow(r, power - 1.0f)*power*dr + 1.0f; // our calculation of dr as descirbed above

			// apply the necessary iterative operations to our spherical coordinate (using trig laws, we can express power as a multiple of angle
			// and a power of radius)
			// this is equivalent to fn-1(c)^power
			Float zr = pow(r, power);
			theta = theta * power;
			phi = phi * power;


			// convert it back to cartesian coordiantes so we can add our constant term
			z = zr * Vector3f(sin(theta)*cos(phi), sin(phi)*sin(theta), cos(theta));
			z += vpos; // add the constant term + c
		}
		Float debugLength = Vector3f(pos).Length()*0.5f;


		// use our distance estimation formula to determine distance to surface
		return Clamp(0.5f*log(r)*r / dr, -debugLength, debugLength);
	}

	Vector3f MandelbulbFractal::computeOrbitTrap(const Vector3f& v) const {
		return v;
	}

	std::shared_ptr<Shape> CreateMandelbulbFractalShape(const Transform *o2w,
		const Transform *w2o,
		bool reverseOrientation,
		const ParamSet &params) {

		Float bailoutRadius = params.FindOneFloat("bailoutRadius", DEFAULT_MANDEL_BAILOUT);
		Float power = params.FindOneFloat("power", DEFAULT_MANDEL_POW);
		int mandelIterations = params.FindOneInt("mandelIterations", DEFAULT_MANDEL_ITERATIONS);
		Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
		Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
		Float maxMarchDist =
			params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
		int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
		Float phimax = params.FindOneFloat("phimax", 360.f);
		return std::make_shared<MandelbulbFractal>(
			o2w, w2o, reverseOrientation, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phimax, power, bailoutRadius, mandelIterations);
	}

} // namespace pbrt