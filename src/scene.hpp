#pragma once

#include <memory>
#include <vector>

#include "object.hpp"

namespace rtnpr {

class Scene {
public:
    void add(std::shared_ptr<Object> obj)
    {
        obj->obj_id = m_objects.size();
        m_objects.push_back(std::move(obj));
    }

    void clear()
    {
        m_objects.clear();
    }

    void ray_cast(const Ray &ray, Hit &hit)
    {
        for (const auto &obj: m_objects) {
            obj->ray_cast(ray,hit);
        }
    }
private:
    std::vector<std::shared_ptr<Object>> m_objects;
};


} // namespace rtnpr