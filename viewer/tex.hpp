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

template<typename Image>
class CTexRGB {
 public:
    Image image;
    unsigned int id_tex = 0;

 public:
  CTexRGB() = default;

  virtual void Initialize(unsigned int w0, unsigned int h0)
  {
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
      if( image.channels() ==  3 ) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                       static_cast<int>(image.width()),
                       static_cast<int>(image.height()),
                       0, GL_RGB, GL_UNSIGNED_BYTE,
                       image.data());
      }
      else if( image.channels() ==  4 ) {
          glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                       static_cast<int>(image.width()),
                       static_cast<int>(image.height()),
                       0, GL_RGBA, GL_UNSIGNED_BYTE,
                       image.data());
      }
      glBindTexture(GL_TEXTURE_2D, 0);
  }

  virtual void UpdateGL(int xo, int yo, int w0, int h0)
  {
      if (id_tex == 0) { return; }
      glBindTexture(GL_TEXTURE_2D, id_tex);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
      if (image.channels() == 3) {
          glTexSubImage2D(GL_TEXTURE_2D, 0,
                          static_cast<int>(xo),
                          static_cast<int>(yo),
                          static_cast<int>(w0),
                          static_cast<int>(h0),
                          GL_RGB, GL_UNSIGNED_BYTE,
                          image.data());
      }
      else if (image.channels() == 4) {
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

// ----------------------------------

template<typename Image>
class CTexRGB_Rect2D : public CTexRGB<Image> {
 public:
  CTexRGB_Rect2D() : CTexRGB<Image>() {}
  virtual ~CTexRGB_Rect2D() = default;

  void Initialize(unsigned int w, unsigned int h) override
  {
      CTexRGB<Image>::Initialize(w, h);
      this->min_x = 0.0;
      this->max_x = (double) w;
      this->min_y = 0.0;
      this->max_y = (double) h;
  }

  [[nodiscard]] std::vector<double> MinMaxAABB() const {
    return {this->min_x, this->min_y, z,
            this->max_x, this->max_y, z};
  }

  void SetMinMaxXY(const std::vector<double> &mmxy) {
    if (mmxy.size() < 4) { return; }
    this->min_x = mmxy[0];
    this->max_x = mmxy[1];
    this->min_y = mmxy[2];
    this->max_y = mmxy[3];
    z = (mmxy[4] + mmxy[5]) * 0.5;
  }
 public:
  // this is a coordinate for OpenGL image plane (after ModelView and Projection)
  double min_x = -1, max_x = +1;
  double min_y = -1, max_y = +1;
  double z = -1;
};

} // namespace viewer

#if defined(_MSC_VER)
#pragma warning( pop )
#elif defined(__GNUC__) || defined(__clang__)
#pragma GCC diagnostic pop
#endif

