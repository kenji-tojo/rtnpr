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

#ifndef DFM2_OPENGLSTB_GLYPH_H
#define DFM2_OPENGLSTB_GLYPH_H

#include <vector>
#include <string>
#include <filesystem>
#include <map>

#include "delfem2/dfm2_inline.h"

namespace delfem2 {
namespace openglstb {

class CGlyph{
public:
  struct SData {
    int x;
    int y;
    int width;
    int height;
    int xadvance;
  };
public:
  explicit CGlyph(const std::filesystem::path& fpath);
  void InitGL();
  void DrawEntireGlyph() const;
  void ParseGlyphInfo(const std::filesystem::path& fpath);
  double DrawCharAt(char c, double scale, double px, double py);
  void DrawStringAt(const std::string& str, double scale, double px, double py);
public:
  std::vector<unsigned char> aRGBA;
  int width = 0;
  int height = 0;
  unsigned int texName = 0;
  std::map<char,SData> mapData;
};

}
}

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/stb_opengl/glyph.cpp"
#endif

#endif
