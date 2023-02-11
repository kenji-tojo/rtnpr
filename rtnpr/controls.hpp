#pragma once

#include <memory>
#include <vector>
#include <functional>

#include "rtnpr_math.hpp"

namespace rtnpr {

class Controls {
public:
    bool enabled = true;
    virtual void update() {};
    virtual void on_horizontal_cursor_move(float movement, float speed) {};
    virtual void on_vertical_cursor_move(float movement, float speed) {};
    virtual void on_mouse_wheel(float movement, float speed) {};
};


template<typename Object3D_, bool look_at_ = true>
class SphereControls: public Controls {
public:
    float radius = 1.f;
    float z = 0.f;
    float phi = 0.f;
    Eigen::Vector3f center = Eigen::Vector3f::Zero();

    void set_object(std::shared_ptr<Object3D_> object) {
        m_object = std::move(object);

        using namespace Eigen;

        Vector3f rp = m_object->position - center;
        radius = math::max(1e-2f, rp.norm());
        rp /= radius;

        z = m_up.dot(rp);
        rp -= z * m_up;
        float y = m_e1.cross(rp).dot(m_up);
        float x = m_e1.dot(rp);
        phi = atan2(y, x);
        phi += phi < 0 ? 2.f*float(M_PI) : 0;

#if !defined(NDEBUG)
        const Vector3f pos = m_object->position;
        update_position();
        assert((pos-m_object->position).norm() < 1e-5f);
        m_object->position = pos;
#endif

        update();
    }

    template<typename Vector3f_>
    void set_up(Vector3f_ &&up) {
        m_up = up.normalized();
        math::create_local_frame(m_up, m_e1, m_e2);
    }

    void update() override {
        if (!m_object) { return; }
        update_position();
        if constexpr(look_at_) { m_object->look_at(center); }
    }

    void on_horizontal_cursor_move(float movement, float speed) override {
        if (!enabled) { return; }
        movement *= -1.f * speed;
        float twopi = 2.f * float(M_PI);
        phi += movement * twopi;
        while (phi > twopi) { phi -= twopi; }
        while (phi < 0.f) { phi += twopi; }
        update();
    }

    void on_vertical_cursor_move(float movement, float speed) override {
        if (!enabled) { return; }
        movement *= -1.f * speed;
        z = math::clip(z+movement,-.99f,.99f);
        update();
    }

    void on_mouse_wheel(float movement, float speed) override {
        if (!enabled) { return; }
        movement *= -1.f * speed;
        radius = math::max(1e-2f, radius+movement);
        update();
    }

private:
    std::shared_ptr<Object3D_> m_object;

    Eigen::Vector3f m_up = Eigen::Vector3f::UnitZ();
    Eigen::Vector3f m_e1 = Eigen::Vector3f::UnitX();
    Eigen::Vector3f m_e2 = Eigen::Vector3f::UnitY();

    void update_position() {
        assert(m_object);
        float rxy = std::sqrt(math::max(0.f,1.f-z*z));
        const auto p = z*m_up + rxy*std::cos(phi)*m_e1 + rxy*std::sin(phi)*m_e2;
        m_object->position = radius * p;
    }

};


template<typename Object3D_, bool look_at_ = true>
class UnitDiscControls: public Controls {
public:
    float phi = 0.f;
    Eigen::Vector3f center = Eigen::Vector3f::Zero();

    void set_object(std::shared_ptr<Object3D_> object) {
        m_object = std::move(object);

        using namespace Eigen;

        Vector3f rp = m_object->position - center;
        m_up = m_e1.cross(rp);
        set_up(m_up, m_e1);

        float y = m_e1.cross(rp).dot(m_up);
        float x = m_e1.dot(rp);
        phi = atan2(y, x);
        phi += phi < 0 ? 2.f*float(M_PI) : 0;

#if !defined(NDEBUG)
        const Vector3f pos = m_object->position;
        update_position();
        assert((pos.normalized()-m_object->position).norm()<1e-5f);
        m_object->position = pos;
#endif

        update();
    }

    template<typename Vector3f_>
    void set_up(Vector3f_ &&up) {
        m_up = up.normalized();
        math::create_local_frame(m_up, m_e1, m_e2);
    }

    template<typename Vector3f_>
    void set_up(Vector3f_ &&up, Vector3f_ &&e1) {
        m_up = up.normalized();
        m_e2 = m_up.cross(e1).normalized();
        m_e1 = m_e2.cross(m_up);
    }

    void update() override {
        if (!m_object) { return; }
        update_position();
        if constexpr(look_at_) { m_object->look_at(center); }
    }

    void on_horizontal_cursor_move(float movement, float speed) override {
        if (!enabled) { return; }
        movement *= -1.f * speed;
        float twopi = 2.f * float(M_PI);
        phi += movement * twopi;
        while (phi > twopi) { phi -= twopi; }
        while (phi < 0.f) { phi += twopi; }
        update();
    }

private:
    std::shared_ptr<Object3D_> m_object;

    Eigen::Vector3f m_up = Eigen::Vector3f::UnitZ();
    Eigen::Vector3f m_e1 = Eigen::Vector3f::UnitX();
    Eigen::Vector3f m_e2 = Eigen::Vector3f::UnitY();

    void update_position() {
        assert(m_object);
        m_object->position = std::cos(phi) * m_e1 + std::sin(phi) * m_e2;
    }

};

} // namespace rtnpr