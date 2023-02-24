#pragma once

#include <array>

#include "rtnpr_math.hpp"


namespace rtnpr {

class ToneMapper {
public:
    enum class MapMode {
        Sigmoid = 0, Reinhard = 1, Cel = 2
    };

    float cel_step = 4.f;


    struct ColorTheme {
        Eigen::Vector3f lo_rgb;
        Eigen::Vector3f hi_rgb;
    };
    const std::array<ColorTheme, 2> themes = {
            ColorTheme{Eigen::Vector3f::Zero(),
                       Eigen::Vector3f::Ones()},
            ColorTheme{Eigen::Vector3f{165.f/255.f,206.f/255.f,239.f/255.f},
                       Eigen::Vector3f{250.f/255.f,210.f/255.f,219.f/255.f}}
    };

    int theme_id = 0;

    [[nodiscard]] const ColorTheme &theme() const {
        assert(theme_id < themes.size()); return themes[theme_id];
    }


    [[nodiscard]] Eigen::Vector3f map(float c, MapMode mode = MapMode::Reinhard) const
    {
        switch (mode) {
            case MapMode::Sigmoid: c = math::sigmoid(c) * 1.2f; break;
            case MapMode::Reinhard: c = math::tone_map_Reinhard(c, 4.f); break;
            case MapMode::Cel: {
                c = math::tone_map_Reinhard(c, 4.f);
                c = std::floor(c*cel_step)/cel_step;
                break;
            }
        }
        c = math::clip(c, 0.f, 1.f);
        return (1.f-c) * theme().lo_rgb + c * theme().hi_rgb;
    }

    [[nodiscard]] Eigen::Vector3f map3(Eigen::Vector3f c, MapMode mode = MapMode::Reinhard) const
    {
        switch (mode) {
            case MapMode::Sigmoid: math::sigmoid3(c); break;
            case MapMode::Reinhard: math::Reinhard3(c, 4.f); break;
            case MapMode::Cel: {
                math::Reinhard3(c, 4.f);
                c *= cel_step;
                math::floor3(c);
                c /= cel_step;
                break;
            }
        }
        math::clip3(c, 0.f, 1.f);
        using namespace Eigen;
        return (Vector3f::Ones()-c).cwiseProduct(theme().lo_rgb) + c.cwiseProduct(theme().hi_rgb);
    }
private:

};

} // namespace rtnpr