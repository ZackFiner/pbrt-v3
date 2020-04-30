#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "shapes/fractals/spaceFoldFractal.h"
#include "stats.h"

namespace pbrt {



Vector3f foldAxis(const Vector3f& pos, const Vector3f& n) {
    Float dot = Dot(pos, n);
	if (dot < 0.0f)
		return pos - (2.0f * dot * n);
    return pos;
}
/*
 * Pyramid SDF retrieved from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 */
Float pyramidSDF(const Vector3f &pos, float h) {
	Vector3f p = pos;
	Float m2 = h * h + 0.25f;
	
	p.x = std::abs(p.x);
	p.z = std::abs(p.z);

	Float temp = p.x;
	p.x = (p.z > p.x) ? p.z : p.x;
	p.z = (p.z > p.x) ? temp : p.z;

	p.x -= 0.5f;
	p.z -= 0.5f;

	Vector3f q = Vector3f(p.z, h*p.y - 0.5f*p.x, h*p.x + 0.5f*p.y);
	Float s = std::max(-q.x, 0.0f);
	Float t = Clamp((q.y-0.5f*p.z)/(m2+0.25f), 0.0f, 1.0f);

	Float a = m2 * (q.x + s)*(q.x + s) + q.y*q.y;
	Float b = m2 * (q.x + 0.5f*t)*(q.x + 0.5f*t) + (q.y - m2 * t)*(q.y - m2 * t);

	Float d2 = std::min(q.y, -q.x*m2 - q.y*0.5f) > 0.0 ? 0.0 : std::min(a, b);
	return sqrtf((d2 + q.z*q.z) / m2)*(std::max(q.z,-p.y) < 0.0f ? -1.0f : 1.0f);
}

/*
 * Octahedron SDF retrieved from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 */
Float sdOctahedron(const Vector3f &pos, float s) {
	Vector3f p = Abs(pos);
	return (p.x + p.y + p.z - s)*0.57735027;
}

/*
 * Octahedron SDF retrieved from https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
 */
Float sdBox(const Vector3f &pos, const Vector3f &b) {
	Vector3f q = Abs(pos) - b;
	return (Max(q, Vector3f(0,0,0))).Length() + std::min(std::max(q.x, std::max(q.y, q.z)), 0.0f);
}

Float sdSubtract(Float d1, Float d2) {
	return std::max(-d1, d2);
}
/*
 * The Sierpinski is an excellent example of how spacial manipulation via
 * folding can simplify fractal rendering. Using the same techniques Christensen
 * outlines in the cited article below [1], We use 4 spacial folds along with an
 * iterative uniform scale to render a Sierpinski pyramid with a quadrulateral
 * base.
 *
 * Christensen, M. (September 20, 2011). Distance Estimated 3D Fractals (III): Folding Space [Blog Post]. Retrieved from
 * http://blog.hvidtfeldts.net/index.php/2011/09/distance-estimated-3d-fractals-v-the-mandelbulb-different-de-approximations/
 *
 */
Float SpaceFoldFractal::sdf(const Point3f &pos) const {
    Float r = 1.0f;
    Vector3f z = Vector3f(pos);
    Float Scale = 2.0f; // uniform scale by a factor of 2

	Vector3f f1 = Normalize(Vector3f(1, 1, 0));
    Vector3f f2 = Normalize(Vector3f(-1, 1, 0));
    Vector3f f3 = Normalize(Vector3f(0, 1, 1));
    Vector3f f4 = Normalize(Vector3f(0, 1, -1));
    Vector3f focusPt = Vector3f(0, 1, 0); // note that Norm(f1 + f2 + f3 + f4) is equal to this; this is why I call this the focus point
    int n = 0;
	while (n < foldIterations) {
		z = foldAxis(z, f1); // fold 1
		z = foldAxis(z, f2); // fold 2
		z = foldAxis(z, f3); // fold 3
		z = foldAxis(z, f4); // fold 4
		
        /*
		By scaling up our iteration point, we've effectively scaled down everything else
		in the scene. The focus point is the scaling center: meaning that we want it to
		represent the origin of the respective space we're in (in this case, one of our 
		"pyramids") because we will be using the length of z to determine signed distance
		(it's the same sdf as a sphere, hence why i've placed pyramid in quotes).

		Once we've performed this operation, we are now in the new subspace, and are ready for
		the next iteration.
		*/
        z = z * Scale - focusPt * (Scale - 1.0f); 
		
        n++;
	}
    return sdOctahedron(z, r) * pow(Scale, (float)(-n)); // we use simple spherical collision once we've adjusted z to the level we desire
	// then we scale the result down by the same amount we scaled it up during iteration
}

//Float SpaceFoldFractal::sdf(const Point3f& pos) const {
//	Vector3f z = Vector3f(pos);
//	Float r = 1.0f;
//	Float scale = 2.0f;
//
//	int n = 0;
//	while (n < 6) {
//		if (z.y < 0.5f) { // base section, slice along y = 0 and 0.5f
//			z = foldAxis(z, Vector3f(0.707106781187f, 0, 0.707106781187f));
//			z = foldAxis(z, Vector3f(-0.707106781187f, 0, 0.707106781187f));
//
//			z = z * scale - Vector3f(0, 0, 1);
//		}
//		else {
//			z = z * scale - Vector3f(0, 1, 0);
//		}
//		n++;
//	}
//	Vector3f zrot = z;
//	zrot = Vector3f(zrot.x*0.707106781187f - zrot.z*0.707106781187f, zrot.y, zrot.x*0.707106781187f + zrot.z*0.707106781187f) - (Vector3f(0,2,0) + Vector3f(1, 0, -1)*pow(scale, (float)(n)));
//	Vector3f boxDim = Vector3f(1, 0, 1)*pow(scale, (float)(n)) + Vector3f(0, 1, 0);
//	return sdSubtract(sdBox(z+Vector3f(0,1,0), Vector3f(2,1,2)) ,std::min(sdOctahedron(z, r), sdBox(zrot, boxDim))) * pow(scale, (float)(-n));
//}


Bounds3f SpaceFoldFractal::ObjectBound() const {
    return Bounds3f(Point3f(-20, -20, -20), Point3f(20, 20, 20));
}

std::shared_ptr<Shape> CreateSpaceFoldFractalShape(const Transform *o2w,
                                            const Transform *w2o,
                                            bool reverseOrientation,
                                            const ParamSet &params) {

	// add parameters for iteration numbers, and possibly for the conditional folding planes and 'focus' point (incase we want to adjust the shape of our fractal)
	int foldIterations = params.FindOneInt("foldIterations", DEFAULT_FOLD_ITERATIONS);
    Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
    Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
    Float maxMarchDist =
        params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
    int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
    Float phimax = params.FindOneFloat("phimax", 360.f);
    return std::make_shared<SpaceFoldFractal>(
        o2w, w2o, reverseOrientation, normalEPS, hitEPS, maxMarchDist, maxRaySteps, phimax, foldIterations);
}

}  // namespace pbrt