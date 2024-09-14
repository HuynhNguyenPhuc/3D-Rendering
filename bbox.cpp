#include "BBox.h"
#include "geometry.h"
#include <algorithm>
#include <limits>

BoundingBox::BoundingBox()
    : min(Vec3(std::numeric_limits<float>::max())),
      max(Vec3(std::numeric_limits<float>::lowest())) {}

BoundingBox::BoundingBox(const Vec3& min, const Vec3& max) : min(min), max(max) {}

BoundingBox BoundingBox::expand(const BoundingBox& other) const {
    Vec3 newMin(std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z));
    Vec3 newMax(std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z));
    return BoundingBox(newMin, newMax);
}

void BoundingBox::expand(const Vec3& p) {
    min.x = std::min(min.x, p.x);
    min.y = std::min(min.y, p.y);
    min.z = std::min(min.z, p.z);

    max.x = std::max(max.x, p.x);
    max.y = std::max(max.y, p.y);
    max.z = std::max(max.z, p.z);
}

bool BoundingBox::intersect(const Ray& ray, float& t_min, float& t_max) const {
    t_min = -std::numeric_limits<float>::infinity();
    t_max = std::numeric_limits<float>::infinity();

    for (int i = 0; i < 3; ++i) {
        float invD = 1.0f / ray.direction[i];
        float t0 = (min[i] - ray.origin[i]) * invD;
        float t1 = (max[i] - ray.origin[i]) * invD;

        if (invD < 0.0f) std::swap(t0, t1);

        t_min = std::max(t_min, t0);
        t_max = std::min(t_max, t1);

        if (t_max <= t_min) return false;
    }

    return true;
}