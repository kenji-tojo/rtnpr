/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_OPENGL_NEW_DRAWER_POINTS_H
#define DFM2_OPENGL_NEW_DRAWER_POINTS_H

#include <cstdio>
#include <vector>
#include <array>

#include "delfem2/opengl/new/funcs.h"
#include "delfem2/dfm2_inline.h"

// -------------------------------------

namespace delfem2::opengl {

class Drawer_Coords{
public:
  void InitGL();

  template <typename REAL>
  void SetRawArray(
      REAL *vtx_coords,
      size_t num_vtx_,
      unsigned int ndim);

  template <typename T, unsigned int ndim, class VEC>
  void SetVectors(const std::vector<VEC> &vectors){
    std::vector<T> vtx_coords;
    vtx_coords.reserve(vectors.size()*ndim);
    for(const auto& vec : vectors){
      for(unsigned int idim=0;idim<ndim;++idim){
        vtx_coords.push_back(vec[idim]);
      }
    }
    this->SetRawArray(vtx_coords.data(), vectors.size(), ndim);
  }

  void Draw(
      GLenum gl_primitive_type,
      const float mat4_projection[16],
      const float mat4_modelview[16]) const;
public:
  VertexArrayObject vao; // gl4
  int shaderProgram = -1;
  int Loc_MatrixProjection = -1;
  int Loc_MatrixModelView = -1;
  int Loc_Color = -1;
  unsigned int num_vtx = 0;
  std::array<float,4> color = {0.f, 0.f, 0.f, 1.f};
};

}

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/opengl/new/drawer_points.cpp"
#endif

#endif /* gl4_msh_hpp */
