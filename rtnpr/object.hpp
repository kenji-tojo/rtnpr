#pragma once

#include <memory>

#include "ray.hpp"
#include "hit.hpp"
#include "transform.hpp"

namespace rtnpr {

class Object {
public:
    bool visible = true;

    int obj_id = 0;
    int mat_id = 0;

    std::shared_ptr<Transform> transform = std::make_shared<Transform>();

    virtual void ray_cast(const Ray &ray, Hit &hit) const = 0;
    virtual void apply_transform() = 0;
};

} // namespace rtnpr
