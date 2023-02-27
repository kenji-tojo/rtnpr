#pragma once

#include <vector>

#include "rtnpr_math.hpp"


namespace rtnpr {

template<typename Scalar_, unsigned int channels_>
class Image {
public:
    using Scalar = Scalar_;
    static constexpr unsigned int channels = channels_;

    Image() : m_shape{0,0,channels_} {}

    Image(unsigned int width, unsigned int height)
            : m_shape{width, height, channels_} {
        m_data.resize(size());
    }

    [[nodiscard]] unsigned int size() const { return m_shape[0]*m_shape[1]*m_shape[2]; }
    [[nodiscard]] unsigned int shape(unsigned int index) { assert(index<3); return m_shape[index]; }
    [[nodiscard]] unsigned int ndim() const { return m_shape.size(); }

    void resize(unsigned int width, unsigned int height) {
        m_shape[0] = height;
        m_shape[1] = width;
        m_data.resize(size());
    }

    void clear(Scalar_ val = 0) {
        std::fill(m_data.begin(), m_data.end(), val);
    }

    Scalar_ operator()(unsigned int iw, unsigned int ih, unsigned int ic = 0) const {
        const auto width    = m_shape[0];
        const auto height   = m_shape[1];
        const auto channels = m_shape[2];
        assert(iw<width && ih<height && ic<channels);
        return m_data[(ih*width+iw)*channels+ic];
    }

    Scalar_ &operator()(unsigned int iw, unsigned int ih, unsigned int ic = 0) {
        const auto width    = m_shape[0];
        const auto height   = m_shape[1];
        const auto channels = m_shape[2];
        assert(iw<width && ih<height && ic<channels);
        return m_data[(ih*width+iw)*channels+ic];
    }

    Scalar_ *data() { return m_data.data(); }
    const Scalar_ *data() const { return m_data.data(); }

private:
    std::vector<unsigned int> m_shape;
    std::vector<Scalar_> m_data;
};

} // rtnpr