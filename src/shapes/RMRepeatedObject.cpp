#include "shapes/RMRepeatedObject.h"
#include "efloat.h"
#include "paramset.h"
#include "sampling.h"
#include "stats.h"

namespace pbrt {

Float RMRepeatedObject::repSDF(const Point3f &pos) const {
    Vector2f xz = Vector2f(pos.x, pos.z);
    Vector2f q = Vector2f(xz.Length() - 1.0f, pos.y);
    return q.Length() - 0.2f;
}
Float RMRepeatedObject::sdf(const Point3f &pos) const {
    Vector3f localDim = Vector3f(4.0f, 4.0f, 4.0f);
    Point3f local = Point3f(fmodf(std::abs(pos.x), localDim.x), fmodf(std::abs(pos.y), localDim.y),
                            fmodf(std::abs(pos.z), localDim.z));
    local = local - localDim * 0.5f;
    return repSDF(local);
 
}
Bounds3f RMRepeatedObject::ObjectBound() const {
    Float maxNum = std::numeric_limits<Float>::max();
    return Bounds3f(Point3f(-maxNum, -maxNum, -maxNum),
                    Point3f(maxNum, maxNum, maxNum));

}

std::shared_ptr<Shape> CreateRMRepeatedObjectShape(const Transform *o2w,
                                                   const Transform *w2o,
                                                   bool reverseOrientation,
                                                   const ParamSet &params) {
    Float normalEPS = params.FindOneFloat("normalEPS", DEFAULT_NORMAL_EPS);
    Float hitEPS = params.FindOneFloat("hitEPS", DEFAULT_DIST_THRESHOLD);
    Float maxMarchDist =
        params.FindOneFloat("maxMarchDist", DEFAULT_MAX_DISTANCE);
    int maxRaySteps = params.FindOneInt("maxRaySteps", DEFAULT_MAX_RAY_STEPS);
    Float phimax = params.FindOneFloat("phimax", 360.f);

    return std::make_shared<RMRepeatedObject>(o2w, w2o, reverseOrientation,
                                              normalEPS, hitEPS, maxMarchDist,
                                              maxRaySteps, phimax);
}
}  // namespace pbrt