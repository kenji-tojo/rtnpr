/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_OPENGL_NEW_DRWER_MSHTRI_H
#define DFM2_OPENGL_NEW_DRWER_MSHTRI_H

#include <cstdio>
#include <vector>

#include "delfem2/dfm2_inline.h"
#include "delfem2/opengl/new/funcs.h" // CGL4_VAO_Mesh
#include "delfem2/color.h"

// -------------------------------------

namespace delfem2::opengl {

class CShader_TriMesh{
public:
  void InitGL();

  template <typename REAL>
  void Initialize(
      std::vector<REAL>& aXYZd,
      unsigned int ndim,
      std::vector<unsigned int>& aTri);

  template <typename REAL>
  void UpdateVertex(
      std::vector<REAL>& aXYZd,
      unsigned int ndim,
      std::vector<unsigned int>& aTri);

  void Draw(const float mat4_projection[16],
            const float mat4_modelview[16]) const;
  
public:
  VertexArrayObject vao; // gl4
  int shaderProgram;
  int Loc_MatrixProjection;
  int Loc_MatrixModelView;
  int Loc_Color;
  delfem2::CColor color_face = delfem2::CColor(1.0,0.0,0.0,0.0);
  float line_width = 1.0;
};

}

#ifndef DFM2_STATIC_LIBRARY
  #include "delfem2/opengl/new/drawer_mshtri.cpp"
#endif

#endif /* DFM2_OPENGL_NEW_DRWER_MSHTRI_H */
