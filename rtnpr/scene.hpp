#pragma once

#include <memory>
#include <vector>

#include "object.hpp"
#include "light.hpp"

namespace rtnpr {

class Scene {
public:
    Scene() {
        brdf.emplace_back(std::make_unique<PhongBRDF>());
        brdf.emplace_back(std::make_unique<BRDF>(/*albedo*/.2f));
        brdf.emplace_back(std::make_unique<GlossyBRDF>());
        brdf.emplace_back(std::make_unique<SpecularBRDF>());
    }

    static std::shared_ptr<Scene> create() { return std::make_shared<Scene>(); }

    std::unique_ptr<Light> light = std::make_unique<Light>();
    std::vector<std::unique_ptr<BRDF>> brdf;

    void add(std::shared_ptr<Object> obj)
    {
        obj->obj_id = m_objects.size();
        m_objects.emplace_back(std::move(obj));
    }

    void clear() { m_objects.resize(1); }

    void ray_cast(const Ray &ray, Hit &hit) const
    {
        for (const auto &obj: m_objects) {
            assert(obj);
            obj->ray_cast(ray,hit);
        }
    }

    Object &object(size_t index) { assert(index < m_objects.size()); return *m_objects[index]; }
    Plane &plane() { return *m_plane; }

private:
    std::shared_ptr<Plane> m_plane = Plane::create();
    std::vector<std::shared_ptr<Object>> m_objects{m_plane};

};


} // namespace rtnpr