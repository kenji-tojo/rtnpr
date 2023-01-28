#pragma once

#include "ray.hpp"
#include "hit.hpp"

namespace rtnpr {

class Object {
public:
    int obj_id = 0;
    int mat_id = 0;

    virtual void ray_cast(const Ray &ray, Hit &hit) const = 0;
};

} // namespace rtnpr
