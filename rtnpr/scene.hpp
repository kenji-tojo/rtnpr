#pragma once

#include <memory>
#include <vector>

#include "object.hpp"
#include "light.hpp"
#include "camera.hpp"

namespace rtnpr {

class Scene {
public:
    Scene() {
        brdf.push_back(m_phong);
        brdf.push_back(std::make_shared<BRDF>(/*_albedo=*/.2f));
        brdf.push_back(std::make_shared<GlossyBRDF>());
        brdf.push_back(std::make_shared<SpecularBRDF>());

        m_plane->obj_id = 0;
        m_plane->mat_id = 1;
        m_objects.push_back(m_plane);
    }

    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    std::shared_ptr<Light> light = std::make_shared<Light>();

    std::vector<std::shared_ptr<BRDF>> brdf;

    void add(std::shared_ptr<Object> obj) {
        obj->obj_id = m_objects.size();
        m_objects.emplace_back(std::move(obj));
    }

    void clear() { m_objects.resize(1); }

    void ray_cast(const Ray &ray, Hit &hit) const {
        for (const auto &obj: m_objects) {
            assert(obj);
            obj->ray_cast(ray,hit);
        }
    }

    [[nodiscard]] const Object &object(size_t index) const { assert(index < m_objects.size()); return *m_objects[index]; }
    Object &object(size_t index) { assert(index < m_objects.size()); return *m_objects[index]; }

    Plane &plane() { return *m_plane; }
    [[nodiscard]] const Plane &plane() const { return *m_plane; }

    PhongBRDF &phong() { return *m_phong; }

private:
    std::shared_ptr<Plane> m_plane = Plane::create();
    std::shared_ptr<PhongBRDF> m_phong = std::make_shared<PhongBRDF>();

    std::vector<std::shared_ptr<Object>> m_objects;

};


} // namespace rtnpr