/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @brief functions for mesh export/import
 */

#ifndef DFM2_MSH_IO_OBJ_H
#define DFM2_MSH_IO_OBJ_H

#include <cstdio>
#include <vector>
#include <string>
#include <filesystem>

#include "delfem2/dfm2_inline.h"

namespace delfem2 {

template <typename REAL, typename INT>
DFM2_INLINE void Write_Obj_UniformMesh(
    const std::string &file_path,
    const REAL *vtx_xyz,
    size_t num_xyz,
    const INT *elem_vtx,
    size_t num_elem,
    size_t num_node);

/**
 * write obj file for the mesh the elemenet is a jagged array (tris and quads are mixed).
 * @param str
 * @param aXYZ
 * @param aElemInd
 * @param aElem
 */
DFM2_INLINE void Write_Obj_ElemJArray(
    const std::string &str, // mixed elem
    const std::vector<double> &aXYZ,
    const std::vector<int> &aElemInd,
    const std::vector<int> &aElem);


/**
 * to open the obj file with Blender, select the option "Split by Group".
 * @param pathf
 * @param aXYZ
 * @param aTri
 * @param aFlgTri
 */
DFM2_INLINE void Write_Obj_TriFlag(
    const std::string &pathf,
    std::vector<double> &aXYZ,
    std::vector<unsigned int> &aTri,
    std::vector<unsigned int> &aFlgTri);

DFM2_INLINE void Write_Obj(
    const std::string &str,
    const std::vector<std::pair<std::vector<double>, std::vector<unsigned int> > > &aMesh);

DFM2_INLINE void Write_WavefrontObj(
    const std::filesystem::path &path_file,
    const std::vector<double> &vtx_xyz,
    const std::vector<double> &vtx_tex,
    const std::vector<double> &vtx_nrm,
    const std::vector<unsigned int> &tri_vtx_xyz,
    const std::vector<unsigned int> &tri_vtx_tex,
    const std::vector<unsigned int> &tri_vtx_nrm,
    const std::vector<std::string> &group_names,
    const std::vector<unsigned int> &group_elem_index);


// above: write obj
// -------------------------
// below: read obj

DFM2_INLINE void Read_Obj(
    std::vector<double> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path);

/**
 * Read Obj file for quad-only mesh
 * @param fname
 * @param vtx_xyz
 * @param quad_vtx
 */
DFM2_INLINE void Read_Obj_MeshQuad3(
    std::vector<double> &vtx_xyz,
    std::vector<unsigned int> &quad_vtx,
    const std::filesystem::path &file_path);

DFM2_INLINE void Read_Obj2(
    const std::string &fname,
    std::vector<double> &aXYZ,
    std::vector<unsigned int> &aTri);

template <typename REAL>
void Read_Obj3(
    std::vector<REAL> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path);

// --------------------------
// below: obj with surface attributes

/**
 *
 * @param[out] fname_mtl filen ame of material
 * @param[out] vtx_xyz
 * @param[out] vtx_tex
 * @param[out] vtx_nrm
 * @param[out] elem_vtx_index jagged array index for elem-vtx array
 * @param[out] elem_vtx_xyz
 * @param[out] elem_vtx_tex
 * @param[out] elem_vtx_nrm
 * @param[out] group_names
 * @param[out] group_elem_index jagged array index for group-elem array
 * @param[in] file_path input file path
 * @return return false if failed to open the file
 */
 template <typename T>
DFM2_INLINE bool Read_WavefrontObjWithMaterialMixedElem(
    std::string &fname_mtl,
    std::vector<T> &vtx_xyz,
    std::vector<T> &vtx_tex,
    std::vector<T> &vtx_nrm,
    std::vector<unsigned int> &elem_vtx_index,
    std::vector<unsigned int> &elem_vtx_xyz,
    std::vector<unsigned int> &elem_vtx_tex,
    std::vector<unsigned int> &elem_vtx_nrm,
    std::vector<std::string>& group_names,
    std::vector<unsigned int>& group_elem_index,
    const std::filesystem::path &file_path);

class MaterialWavefrontObj{
public:
  std::string name_mtl;
  float Kd[4]{0};
  float Ka[4]{0};
  float Ks[4]{0};
  float Ke[4]{0};
  float Ns = 0;
  int illum = -1;
  std::string map_Kd;
};

void Read_WavefrontMaterial(
    const std::filesystem::path &file_path,
    std::vector<MaterialWavefrontObj>& materials);

/*
class Shape3_WavefrontObj{
public:
  void ReadObj(const std::string& fname);
//  void Draw() const;
  [[nodiscard]] std::vector<double> AABB3_MinMax() const;
  void ScaleXYZ(double s);
  void TranslateXYZ(double x, double y, double z);
public:
  std::vector<double> aXYZ;
  std::vector<double> aTex;
  std::vector<double> aNorm;
  std::vector<delfem2::TriGroupWavefrontObj> aObjGroupTri;
  std::vector<MaterialWavefrontObj> aMaterial;
};
 */



} // namespace delfem2

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/msh_io_obj.cpp"
#endif

#endif // DFM2_MSH_IO_OBJ_H
