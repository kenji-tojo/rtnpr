#pragma once

#include <vector>

#include "rtnpr_math.hpp"


namespace rtnpr {

enum class PixelFormat {
    RGB, RGBA, Alpha
};

template<typename dtype_, PixelFormat fmt_ = PixelFormat::RGB>
class Image {
public:
    using dtype = dtype_;
    static constexpr PixelFormat fmt = fmt_;

    void resize(unsigned int width, unsigned int height) {
        m_width = width;
        m_height = height;
        m_data.resize(pixels()*channels());
    }

    void clear(dtype_ val = 0) {
        auto size = m_data.size();
        m_data.clear();
        m_data.resize(size, val);
    }

    [[nodiscard]] unsigned int width() const { return m_width; }
    [[nodiscard]] unsigned int height() const { return m_height; }
    [[nodiscard]] unsigned int pixels() const { return m_width * m_height; }
    [[nodiscard]] unsigned int size() const { return pixels() * channels();}
    [[nodiscard]] constexpr unsigned int channels() const {
        if constexpr(fmt == PixelFormat::RGB) { return 3; }
        else if (fmt == PixelFormat::RGBA) { return 4; }
        return 1; // PixelFormat::Alpha
    }

    dtype_ operator()(int iw, int ih, int ic = 0) const {
        assert(iw < m_width);
        assert(ih < m_height);
        assert(ic < channels());
        return m_data[(ih*m_width+iw)*channels()+ic];
    }

    dtype_ &operator()(int iw, int ih, int ic = 0) {
        assert(iw < m_width);
        assert(ih < m_height);
        assert(ic < channels());
        return m_data[(ih*m_width+iw)*channels()+ic];
    }

    dtype_ *data() { return m_data.data(); }
    const dtype_ *data() const { return m_data.data(); }

private:
    unsigned int m_width = 0;
    unsigned int m_height = 0;

    std::vector<dtype_> m_data;
};

} // rtnpr