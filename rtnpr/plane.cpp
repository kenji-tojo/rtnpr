#include "plane.h"

namespace rtnpr {

Plane::Plane(): Object("plane") {}

Plane::~Plane() = default;

void Plane::ray_cast(const Ray &ray, Hit &hit) const
{
    using namespace std;
    using namespace Eigen;

    if (!this->visible) { return; }

    auto nrm = m_normal;
    if (ray.dir.dot(nrm) > 0.f) { nrm *= -1.f; }

    float ddn = -ray.dir.dot(nrm);
    if (ddn < 1e-8f) {
        // plane is perpendicular to the ray
        return;
    }

    float dist = nrm.dot(ray.org-m_center) / ddn;
    if (dist < 1e-8f) {
        // plane is behind the ray origin
        return;
    }
    if (dist >= hit.dist) { return; }

    const auto pos = ray.org + dist * ray.dir + 1e-4f * nrm;
    float w = m_b1.dot(pos-m_center)/m_width+.5f;
    float h = m_b2.dot(pos-m_center)/m_height+.5f;
    if (w < 0.f || w > 1.f) { return; }
    if (h < 0.f || h > 1.f) { return; }

    hit.dist = dist;
    hit.prim_id = 0;
    if (this->checkerboard) { hit.prim_id = floor(w*float(check_res)) + floor(h*float(check_res)); }
    hit.nrm = nrm;
    hit.pos = pos;
    hit.wo = -ray.dir;
    hit.obj_id = this->obj_id;
    hit.mat_id = this->mat_id;
}

void Plane::apply_transform()
{
    using namespace Eigen;
    m_center = this->transform->shift;
    const auto rot = this->transform->rot();
    m_normal = rot * Vector3f::UnitZ();
    m_b1 = rot * Vector3f::UnitX();
    m_b2 = rot * Vector3f::UnitY();
    m_width = this->transform->scale;
    m_height = this->transform->scale;
}


} // namespace rtnpr