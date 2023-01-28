#ifndef DFM2_OPENGL_NEW_SHDR_MSHTEX_H
#define DFM2_OPENGL_NEW_SHDR_MSHTEX_H

#include <cstdio>
#include <vector>
#include <set>
#include <climits>

#include "delfem2/opengl/new/funcs.h" // CGL4_VAO_Mesh
#include "delfem2/msh_topology_uniform.h"
#include "delfem2/dfm2_inline.h"

// -------------------------------------

namespace delfem2::opengl {

class Drawer_MeshTex {
public:
  void SetElement(
      std::vector<unsigned int> &elem_vtx,
      int gl_primitive_type);

  template <typename REAL>
  void SetCoords(
      std::vector<REAL> &vtx_coords,
      unsigned int ndim);

  template <typename REAL>
  void SetTexUV(
      std::vector<REAL> &aTex);

  virtual void InitGL();

  void Draw(const float mat4_projection[16],
            const float mat4_modelview[16]) const;

public:
  VertexArrayObject vao; // gl4
  int shaderProgram = -1;
  int loc_projection_matrix = -1;
  int loc_modelview_matrix = -1;
  int loc_texture = -1;
};

// ----------------------------------

class Drawer_RectangleTex : public Drawer_MeshTex {
public:
  Drawer_RectangleTex(float xmin, float xmax, float ymin, float ymax, float z=0.f)
  : xmin(xmin), xmax(xmax), ymin(ymin), ymax(ymax), z(z){}
  
  Drawer_RectangleTex(float half_size)
  : xmin(-half_size), xmax(+half_size), ymin(-half_size), ymax(+half_size){}
  
  Drawer_RectangleTex() = default;
  
  // --------------------
  
  void InitGL() override {
    Drawer_MeshTex::InitGL();
    std::vector<float> aPos3d = {
      xmin, ymin, z,
      xmax, ymin, z,
      xmax, ymax, z,
      xmin, ymax, z,
    };
    std::vector<unsigned int> aTri = {
      0,1,2,
      0,2,3,
    };
    std::vector<float> aTex2d = {
      0.0, 0.0,
      1.0, 0.0,
      1.0, 1.0,
      0.0, 1.0
    };
    Drawer_MeshTex::SetCoords(aPos3d,3);
    Drawer_MeshTex::SetTexUV(aTex2d);
    Drawer_MeshTex::SetElement( aTri, GL_TRIANGLES);
  }
private:
  float xmin = -1;
  float xmax = +1;
  float ymin = -1;
  float ymax = +1;
  float z = 0.0;
};

class Drawer_MeshMixTwoTextures
    : public Drawer_MeshTex{
 public:
  void InitGL();
  void Draw(
      const float mat4_projection[16],
      const float mat4_modelview[16],
      float ratio) const;
 public:
  unsigned int loc_ratio;
  unsigned int loc_overlaytex;
};


}  // delfem2::opengl


#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/opengl/new/drawer_mshtex.cpp"
#endif



#endif  // DFM2_OPENGL_NEW_SHDR_MSHTEX_H
