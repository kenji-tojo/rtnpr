#pragma once

#include <array>

#include "rtnpr_math.hpp"


namespace rtnpr {

class ToneMapper {
public:
    enum Mode {
        Reinhard = 0,
        Linear   = 1,
        Raw      = 2
    };

    int mode = int(Reinhard);

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

    [[nodiscard]] const ColorTheme &theme(unsigned int theme_id) const {
        assert(theme_id < themes.size()); return themes[theme_id];
    }


    [[nodiscard]] Eigen::Vector3f map(float c, unsigned int theme_id) const {
        using namespace Eigen;
        switch (mode) {
            case Linear: break;
            case Reinhard: c = math::tone_map_Reinhard(c, 4.f); break;
            case Raw:
            default:
                return Vector3f{c,c,c};
        }

        c = math::clip(c, 0.f, 1.f);
        return (1.f-c) * theme(theme_id).lo_rgb + c * theme(theme_id).hi_rgb;
    }

    [[nodiscard]] Eigen::Vector3f map3(Eigen::Vector3f c, unsigned int theme_id) const {
        using namespace Eigen;
        switch (mode) {
            case Linear: break;
            case Reinhard: math::Reinhard3(c, 4.f); break;
            case Raw:
            default:
                return c;
        }

        math::clip3(c, 0.f, 1.f);
        using namespace Eigen;
        return (Vector3f::Ones()-c).cwiseProduct(theme(theme_id).lo_rgb) + c.cwiseProduct(theme(theme_id).hi_rgb);
    }
private:

};

} // namespace rtnpr