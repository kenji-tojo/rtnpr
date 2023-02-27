/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * - this code should be compilable for both modern and legacy OpenGL.
 * - This code should not have any dependencies.
 */

#pragma once

#include <memory>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <cassert>
#include <cstdlib>

#if defined(_MSC_VER)
#  pragma warning( push )
// C4996 (lev3): Your code uses a function, class member, variable, or typedef that's marked deprecated.
#  pragma warning( disable : 4996 )
#elif defined(__GNUC__) || defined(__clang__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined(_WIN32) // windows
#  include <windows.h>
#endif

#if defined(__APPLE__) && defined(__MACH__) // mac
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif


namespace viewer {

template<typename Image_>
class CTexRGB {
 public:
    using Image = Image_;

    Image_ image;
    unsigned int id_tex = 0;

 public:
  CTexRGB() = default;

  virtual void Initialize(unsigned int w0, unsigned int h0) {
      image.resize(w0,h0);
      id_tex = 0;
  }

  virtual void InitGL()
  {
      if (id_tex == 0) { ::glGenTextures(1, &id_tex); }
      glBindTexture(GL_TEXTURE_2D, id_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      if constexpr(Image_::channels == 3) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                       static_cast<int>(image.shape(0)),
                       static_cast<int>(image.shape(1)),
                       0, GL_RGB, GL_UNSIGNED_BYTE,
                       image.data());
      }
      else if(Image_::channels == 4) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                       static_cast<int>(image.shape(0)),
                       static_cast<int>(image.shape(1)),
                       0, GL_RGBA, GL_UNSIGNED_BYTE,
                       image.data());
      }
      glBindTexture(GL_TEXTURE_2D, 0);
  }

  virtual void UpdateGL(
          unsigned int xo,
          unsigned int yo,
          unsigned int w0,
          unsigned int h0)
  {
      if (id_tex == 0) { return; }
      glBindTexture(GL_TEXTURE_2D, id_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      if constexpr(Image_::channels == 3) {
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          static_cast<int>(xo),
                          static_cast<int>(yo),
                          static_cast<int>(w0),
                          static_cast<int>(h0),
                          GL_RGB, GL_UNSIGNED_BYTE,
                          image.data());
      }
      else if (Image_::channels == 4) {
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          static_cast<int>(xo),
                          static_cast<int>(yo),
                          static_cast<int>(w0),
                          static_cast<int>(h0),
                          GL_RGBA, GL_UNSIGNED_BYTE,
                          image.data());
      }
      glBindTexture(GL_TEXTURE_2D, 0);
  }
};

} // namespace viewer

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

