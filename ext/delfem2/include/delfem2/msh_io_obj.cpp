/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/msh_io_obj.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cassert>
#include <string>
#include <cstring>

#include "delfem2/msh_affine_transformation.h"

namespace delfem2::msh_ioobj {

DFM2_INLINE void ParseVtxObj_(
    int &ip,
    int &it,
    int &in,
    char *str) {
  ip = -1;
  it = -1;
  in = -1;
  std::size_t n = strlen(str);
  int icnt = 0;
  unsigned int aloc[3];
  for (unsigned int i = 0; i < n; ++i) {
    if (str[i] != '/') { continue; }
    str[i] = '\0';
    aloc[icnt] = i;
    icnt++;
  }
  ip = std::stoi(str);
  ip--;
  if (icnt == 0) {
    return;
  }
  if (icnt == 1) {
    it = std::stoi(str + aloc[0] + 1);
    it--;
    return;
  }
  if (icnt == 2) {
    if (aloc[1] - aloc[0] != 1) {
      it = std::stoi(str + aloc[0] + 1);
      it--;
    }
    in = std::stoi(str + aloc[1] + 1);
    in--;
    return;
  }
}

}

template <typename REAL, typename INT>
DFM2_INLINE void delfem2::Write_Obj_UniformMesh(
    const std::string &file_path,
    const REAL *vtx_xyz,
    size_t num_xyz,
    const INT *elem_vtx,
    size_t num_elem,
    size_t num_node) {
  std::ofstream fout(
    file_path.c_str(),
    std::ofstream::out);
  for (size_t ip = 0; ip < num_xyz; ip++) {
    fout << "v ";
    fout << vtx_xyz[ip * 3 + 0] << " ";
    fout << vtx_xyz[ip * 3 + 1] << " ";
    fout << vtx_xyz[ip * 3 + 2] << std::endl;
  }
  for (size_t iel = 0; iel < num_elem; iel++) {
    fout << "f ";
    for (size_t ino = 0; ino < num_node; ++ino ) {
      fout << elem_vtx[iel * num_node + ino] + 1 << " ";
    }
    fout << std::endl;
  }
}
#ifdef DFM2_STATIC_LIBRARY
template DFM2_INLINE void delfem2::Write_Obj_UniformMesh(
    const std::string &file_path,
    const double *vtx_xyz,
    size_t num_xyz,
    const unsigned int *tri_vtx,
    size_t num_tri,
    size_t num_node);
template DFM2_INLINE void delfem2::Write_Obj_UniformMesh(
    const std::string &file_path,
    const float *vtx_xyz,
    size_t num_xyz,
    const unsigned int *tri_vtx,
    size_t num_tri,
    size_t num_node);
#endif

// ------------------------------------------

DFM2_INLINE void delfem2::Write_Obj_ElemJArray(
    const std::string &str,
    const std::vector<double> &aXYZ,
    const std::vector<int> &aElemInd,
    const std::vector<int> &aElem) {
  assert(!aElemInd.empty());
  const size_t np = aXYZ.size() / 3;
  std::ofstream fout(str.c_str(), std::ofstream::out);
  for (unsigned int ip = 0; ip < np; ip++) {
    fout << "v ";
    fout << aXYZ[ip * 3 + 0] << " ";
    fout << aXYZ[ip * 3 + 1] << " ";
    fout << aXYZ[ip * 3 + 2] << std::endl;
  }
  const size_t ne = aElemInd.size() - 1;
  for (unsigned int iie = 0; iie < ne; iie++) {
    const int ie0 = aElemInd[iie];
    const int nnoel = aElemInd[iie + 1] - ie0;
    assert(nnoel == 3 || nnoel == 4);
    if (nnoel == 3) {
      fout << "f ";
      fout << aElem[ie0 + 0] + 1 << " ";
      fout << aElem[ie0 + 1] + 1 << " ";
      fout << aElem[ie0 + 2] + 1 << std::endl;
    } else if (nnoel == 4) {
      fout << "f ";
      fout << aElem[ie0 + 0] + 1 << " ";
      fout << aElem[ie0 + 1] + 1 << " ";
      fout << aElem[ie0 + 2] + 1 << " ";
      fout << aElem[ie0 + 3] + 1 << std::endl;
    }
  }
}

DFM2_INLINE void delfem2::Write_Obj_TriFlag(
    const std::string &pathf,
    std::vector<double> &aXYZ,
    std::vector<unsigned int> &aTri,
    std::vector<unsigned int> &aFlgTri) {
  const size_t nt = aTri.size() / 3;
//  std::cout << nt << " " << aFlgTri.size() << std::endl;
  assert(aFlgTri.size() == nt);
  unsigned int flgmax = 0;
  for (unsigned int it = 0; it < nt; ++it) {
    if (aFlgTri[it] > flgmax) { flgmax = aFlgTri[it]; }
  }
//  std::cout << flgmax << std::endl;
  std::ofstream fout(pathf.c_str(), std::ofstream::out);
  const size_t np = aXYZ.size() / 3;
  for (unsigned int ip = 0; ip < np; ip++) {
    fout << "v ";
    fout << aXYZ[ip * 3 + 0] << " ";
    fout << aXYZ[ip * 3 + 1] << " ";
    fout << aXYZ[ip * 3 + 2] << std::endl;
  }
  for (unsigned int iflg = 0; iflg < flgmax + 1; ++iflg) {
    fout << "g flag" << std::to_string(iflg) << std::endl;
    for (unsigned int it = 0; it < aTri.size() / 3; ++it) {
      if (aFlgTri[it] != iflg) { continue; }
      fout << "f ";
      fout << aTri[it * 3 + 0] + 1 << " ";
      fout << aTri[it * 3 + 1] + 1 << " ";
      fout << aTri[it * 3 + 2] + 1 << std::endl;
    }
  }
}

DFM2_INLINE void delfem2::Write_Obj(
    const std::string &str,
    const std::vector<std::pair<std::vector<double>, std::vector<unsigned int> > > &aMesh) {
  std::ofstream fout(str.c_str(), std::ofstream::out);
  int ipsum = 0;
  for (int im = 0; im < (int) aMesh.size(); im++) {
    const std::vector<double> &aXYZ = aMesh[im].first;
    const std::vector<unsigned int> &aTri = aMesh[im].second;
    int np = (int) aXYZ.size() / 3;
    int nt = (int) aTri.size() / 3;
    { // group id
      fout << "g " << im << std::endl;
    }
    for (int ip = 0; ip < np; ip++) {
      fout << "v ";
      fout << aXYZ[ip * 3 + 0] << " ";
      fout << aXYZ[ip * 3 + 1] << " ";
      fout << aXYZ[ip * 3 + 2] << std::endl;
    }
    for (int itri = 0; itri < nt; itri++) {
      fout << "f ";
      fout << aTri[itri * 3 + 0] + 1 + ipsum << " ";
      fout << aTri[itri * 3 + 1] + 1 + ipsum << " ";
      fout << aTri[itri * 3 + 2] + 1 + ipsum << std::endl;
    }
    ipsum += np;
  }
}

DFM2_INLINE void delfem2::Write_WavefrontObj(
    const std::filesystem::path &path_file,
    const std::vector<double> &vtx_xyz,
    const std::vector<double> &vtx_tex,
    const std::vector<double> &vtx_nrm,
    const std::vector<unsigned int> &tri_vtx_xyz,
    const std::vector<unsigned int> &tri_vtx_tex,
    const std::vector<unsigned int> &tri_vtx_nrm,
    const std::vector<std::string> &group_names,
    const std::vector<unsigned int> &group_elem_index) {
  std::ofstream fout(path_file, std::ofstream::out);
  for (size_t ip = 0; ip < vtx_xyz.size() / 3; ip++) {
    fout << "v ";
    fout << vtx_xyz[ip * 3 + 0] << " ";
    fout << vtx_xyz[ip * 3 + 1] << " ";
    fout << vtx_xyz[ip * 3 + 2] << std::endl;
  }
  for (size_t ip = 0; ip < vtx_tex.size() / 2; ip++) {
    fout << "vt ";
    fout << vtx_tex[ip * 2 + 0] << " ";
    fout << vtx_tex[ip * 2 + 1] << std::endl;
  }
  for (size_t ip = 0; ip < vtx_nrm.size() / 3; ip++) {
    fout << "vn ";
    fout << vtx_nrm[ip * 3 + 0] << " ";
    fout << vtx_nrm[ip * 3 + 1] << " ";
    fout << vtx_nrm[ip * 3 + 2] << std::endl;
  }
  for (size_t ig = 0; ig < group_elem_index.size() - 1; ++ig) {
    if (ig-1 < group_names.size() && ig != 0) {
      fout << "g " << group_names[ig-1] << std::endl;
    }
    for (size_t itri = group_elem_index[ig]; itri < group_elem_index[ig + 1]; itri++) {
      assert(itri<tri_vtx_xyz.size()/3);
      assert(itri<tri_vtx_tex.size()/3);
      assert(itri<tri_vtx_nrm.size()/3);
      fout << "f ";
      for (int i = 0; i < 3; ++i) {
        fout << tri_vtx_xyz[itri * 3 + i] + 1 << "/";
        fout << tri_vtx_tex[itri * 3 + i] + 1 << "/";
        fout << tri_vtx_nrm[itri * 3 + i] + 1 << " ";
      }
      fout << std::endl;
    }
  }
}


// above: write
// ===========================================================
// below: read

DFM2_INLINE void delfem2::Read_Obj(
    std::vector<double> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path) {
  std::ifstream fin;
  fin.open(file_path);
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    return;
  }
  vtx_xyz.clear();
  tri_vtx.clear();
  vtx_xyz.reserve(256 * 16);
  tri_vtx.reserve(256 * 16);
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'v' && buff[1] == ' ') {
      char str[256];
      double x, y, z;
      {
        std::istringstream is(buff);
        is >> str >> x >> y >> z;
//        sscanf(buff, "%s %lf %lf %lf", str, &x, &y, &z);
      }
      vtx_xyz.push_back(x);
      vtx_xyz.push_back(y);
      vtx_xyz.push_back(z);
    }
    if (buff[0] == 'f') {
      char str[256];
      int i0, i1, i2;
      {
        std::istringstream is(buff);
        is >> str >> i0 >> i1 >> i2;
//       sscanf(buff, "%s %d %d %d", str, &i0, &i1, &i2);
      }
      tri_vtx.push_back(i0 - 1);
      tri_vtx.push_back(i1 - 1);
      tri_vtx.push_back(i2 - 1);
    }
  }
}

DFM2_INLINE void delfem2::Read_Obj_MeshQuad3(
    std::vector<double> &vtx_xyz,
    std::vector<unsigned int> &quad_vtx,
    const std::filesystem::path &file_path) {
  std::ifstream fin;
  fin.open(file_path.c_str());
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    return;
  }
  vtx_xyz.clear();
  quad_vtx.clear();
  vtx_xyz.reserve(256 * 16);
  quad_vtx.reserve(256 * 16);
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'v' && buff[1] == ' ') {
      char str[256];
      double x, y, z;
      std::istringstream is(buff);
      is >> str >> x >> y >> z;
//      sscanf(buff, "%s %lf %lf %lf", str, &x, &y, &z);
      vtx_xyz.push_back(x);
      vtx_xyz.push_back(y);
      vtx_xyz.push_back(z);
    }
    if (buff[0] == 'f') {
      char str[256];
      int i0, i1, i2, i3;
      std::istringstream is(buff);
      is >> str >> i0 >> i1 >> i2 >> i3;
//      sscanf(buff, "%s %d %d %d %d", str, &i0, &i1, &i2, &i3);
      quad_vtx.push_back(i0 - 1);
      quad_vtx.push_back(i1 - 1);
      quad_vtx.push_back(i2 - 1);
      quad_vtx.push_back(i3 - 1);
    }
  }
}

DFM2_INLINE void delfem2::Read_Obj2(
    const std::string &fname,
    std::vector<double> &aXYZ,
    std::vector<unsigned int> &aTri) {
  std::ifstream fin;
  fin.open(fname.c_str());
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    return;
  }
  aXYZ.clear();
  aTri.clear();
  aXYZ.reserve(256 * 16);
  aTri.reserve(256 * 16);
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'v' && buff[1] == ' ') {
      char str[256];
      double x, y, z;
      std::istringstream is(buff);
      is >> str >> x >> y >> z;
//      sscanf(buff, "%s %lf %lf %lf", str, &x, &y, &z);
      aXYZ.push_back(x);
      aXYZ.push_back(y);
      aXYZ.push_back(z);
    }
    if (buff[0] == 'f') {
      char str[256], str0[256], str1[256], str2[256];
      {
        std::istringstream is(buff);
        is >> str >> str0 >> str1 >> str2;
//        sscanf(buff, "%s %s %s %s", str, str0, str1, str2);
      }
      for (unsigned int i = 0; i < strlen(str0); ++i) { if (str0[i] == '/') { str0[i] = '\0'; }}
      for (unsigned int i = 0; i < strlen(str1); ++i) { if (str1[i] == '/') { str1[i] = '\0'; }}
      for (unsigned int i = 0; i < strlen(str2); ++i) { if (str2[i] == '/') { str2[i] = '\0'; }}
      const int i0 = std::stoi(str0);
      const int i1 = std::stoi(str1);
      const int i2 = std::stoi(str2);
//      sscanf(str0,"%d",&i0);
//      sscanf(str1,"%d",&i1);
//      sscanf(str2,"%d",&i2);
      aTri.push_back(i0 - 1);
      aTri.push_back(i1 - 1);
      aTri.push_back(i2 - 1);
    }
  }
}

template <typename REAL>
void delfem2::Read_Obj3(
    std::vector<REAL> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path) {
  std::ifstream fin;
  fin.open(file_path);
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    return;
  }
  vtx_xyz.clear();
  tri_vtx.clear();
  vtx_xyz.reserve(256 * 16);
  tri_vtx.reserve(256 * 16);
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'v' && buff[1] == ' ') {
      char str[256];
      REAL x, y, z;
      std::istringstream is(buff);
      is >> str >> x >> y >> z;
//      sscanf(buff, "%s %lf %lf %lf", str, &x, &y, &z);
      vtx_xyz.push_back(x);
      vtx_xyz.push_back(y);
      vtx_xyz.push_back(z);
    }
    if (buff[0] == 'f') {
      std::vector<std::string> vec_str;
      {
        std::istringstream iss(buff);
        std::string s;
        bool is_init = true;
        while (iss >> s) {
          if (is_init) {
            is_init = false;
            continue;
          }
          vec_str.push_back(s);
        }
      }
      std::vector<int> aI;
      aI.reserve(4);
      for (auto str : vec_str) {
        for (size_t i=0;i<str.size();++i) {
          if (str[i] == '/') {
            str[i] = '\0';
            break;
          }
        }
        int i0 = std::stoi(str);
        aI.push_back(i0-1);
      }
      if (aI.size() == 3) {
        tri_vtx.push_back(aI[0]);
        tri_vtx.push_back(aI[1]);
        tri_vtx.push_back(aI[2]);
      }
      if (aI.size() == 4) {
        tri_vtx.push_back(aI[0]);
        tri_vtx.push_back(aI[1]);
        tri_vtx.push_back(aI[2]);
        //
        tri_vtx.push_back(aI[0]);
        tri_vtx.push_back(aI[2]);
        tri_vtx.push_back(aI[3]);
      }
    }
  }
}
#ifdef DFM2_STATIC_LIBRARY
template void delfem2::Read_Obj3(
    std::vector<double> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path);
template void delfem2::Read_Obj3(
    std::vector<float> &vtx_xyz,
    std::vector<unsigned int> &tri_vtx,
    const std::filesystem::path &file_path);
#endif

// ==========================

template<typename T>
DFM2_INLINE bool delfem2::Read_WavefrontObjWithMaterialMixedElem(
    std::string &fname_mtl,
    std::vector<T> &vtx_xyz,
    std::vector<T> &vtx_tex,
    std::vector<T> &vtx_nrm,
    std::vector<unsigned int> &elem_vtx_index,
    std::vector<unsigned int> &elem_vtx_xyz,
    std::vector<unsigned int> &elem_vtx_tex,
    std::vector<unsigned int> &elem_vtx_nrm,
    std::vector<std::string> &group_names,
    std::vector<unsigned int> &group_elem_index,
    const std::filesystem::path &file_path) {
  std::ifstream fin;
  fin.open(file_path);
  if (fin.fail()) { return false; }
  //
  vtx_xyz.clear();
  vtx_nrm.clear();
  vtx_tex.clear();
  vtx_xyz.reserve(256 * 16);
  //
  elem_vtx_index.clear();
  elem_vtx_index.push_back(0);
  elem_vtx_xyz.clear();
  elem_vtx_tex.clear();
  elem_vtx_nrm.clear();
  //
  group_names.clear();
  group_names.push_back("default");
  group_elem_index.clear();
  group_elem_index.push_back(0);
  //
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  fname_mtl.clear();
  std::string current_material_name;
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == 'm') {
      std::stringstream ss(buff);
      std::string str0, str1;
      ss >> str0 >> str1;
      fname_mtl = str1;
      continue;
    }
    if (buff[0] == 'v') {
      char str[256];
      T x, y, z;
      std::istringstream is(buff);
      if (buff[1] == ' ') { // vertex
        is >> str >> x >> y >> z;
        vtx_xyz.push_back(x);
        vtx_xyz.push_back(y);
        vtx_xyz.push_back(z);
      } else if (buff[1] == 'n') { // noraml
        is >> str >> x >> y >> z;
        T len = std::sqrt(x * x + y * y + z * z);
        x /= len;
        y /= len;
        z /= len;
        vtx_nrm.push_back(x);
        vtx_nrm.push_back(y);
        vtx_nrm.push_back(z);
      } else if (buff[1] == 't') { // tex
        is >> str >> x >> y;
        vtx_tex.push_back(x);
        vtx_tex.push_back(y);
      }
    }
    if (buff[0] == 'g') { // group
      std::stringstream ss(buff);
      std::string str0, str1;
      ss >> str0 >> str1;
      group_names.push_back(str1);
      group_elem_index.push_back(elem_vtx_index.size()-1);  // number of elements
      continue;
    }
    if (buff[0] == 'u') { // usemtl
      std::stringstream ss(buff);
      std::string str0, str1;
      ss >> str0 >> str1;
      current_material_name = str1;
      continue;
    }
    if (buff[0] == 'f') {
      std::vector<std::string> vec_str;
      {
        std::istringstream iss(buff);
        std::string s;
        bool is_init = true;
        while (iss >> s) {
          if (is_init) {
            is_init = false;
            continue;
          }
          vec_str.push_back(s);
        }
      }
      for (const auto &str0: vec_str) {
        char buff1[256];
        std::strncpy(buff1, str0.c_str(), str0.size());
        buff1[str0.size()] = '\0';
        int ip0, it0, in0;
        msh_ioobj::ParseVtxObj_(ip0, it0, in0, buff1);
        elem_vtx_xyz.push_back(ip0);
        elem_vtx_tex.push_back(it0);
        elem_vtx_nrm.push_back(in0);
      }
      elem_vtx_index.push_back(elem_vtx_xyz.size());
    }
  }
  group_elem_index.push_back(elem_vtx_index.size()-1); // number of elements
  return true;
}
#ifdef DFM2_STATIC_LIBRARY
template bool delfem2::Read_WavefrontObjWithMaterialMixedElem(
    std::string &fname_mtl,
    std::vector<float> &vtx_xyz,
    std::vector<float> &vtx_tex,
    std::vector<float> &vtx_nrm,
    std::vector<unsigned int> &elem_vtx_index,
    std::vector<unsigned int> &elem_vtx_xyz,
    std::vector<unsigned int> &elem_vtx_tex,
    std::vector<unsigned int> &elem_vtx_nrm,
    std::vector<std::string>& group_names,
    std::vector<unsigned int>& group_elem_index,
    const std::filesystem::path &file_path);
template bool delfem2::Read_WavefrontObjWithMaterialMixedElem(
    std::string &fname_mtl,
    std::vector<double> &vtx_xyz,
    std::vector<double> &vtx_tex,
    std::vector<double> &vtx_nrm,
    std::vector<unsigned int> &elem_vtx_index,
    std::vector<unsigned int> &elem_vtx_xyz,
    std::vector<unsigned int> &elem_vtx_tex,
    std::vector<unsigned int> &elem_vtx_nrm,
    std::vector<std::string>& group_names,
    std::vector<unsigned int>& group_elem_index,
    const std::filesystem::path &file_path);
#endif

void delfem2::Read_WavefrontMaterial(
    const std::filesystem::path &file_path,
    std::vector<MaterialWavefrontObj> &materials) {
  std::ifstream fin;
  fin.open(file_path);
  if (fin.fail()) {
    std::cout << "File Read Fail" << std::endl;
    return;
  }
  materials.clear();
  const int BUFF_SIZE = 256;
  char buff[BUFF_SIZE];
  while (fin.getline(buff, BUFF_SIZE)) {
    if (buff[0] == '#') { continue; }
    if (buff[0] == '\n') { continue; }
    std::stringstream ss(buff);
    std::string str0, str1, str2, str3, str4;
    ss >> str0;
    if (str0 == "newmtl") {
      materials.resize(materials.size() + 1);
      const int imtl0 = static_cast<int>(materials.size()) - 1;
      ss >> str1;
      materials[imtl0].name_mtl = str1;
    }
    const int imtl0 = static_cast<int>(materials.size()) - 1;
    if (str0 == "Kd") {
      ss >> str1 >> str2 >> str3;
      materials[imtl0].Kd[0] = std::stof(str1);
      materials[imtl0].Kd[1] = std::stof(str2);
      materials[imtl0].Kd[2] = std::stof(str3);
      materials[imtl0].Kd[3] = 1.0;
    }
    if (str0 == "Ka") {
      ss >> str1 >> str2 >> str3;
      materials[imtl0].Ka[0] = std::stof(str1);
      materials[imtl0].Ka[1] = std::stof(str2);
      materials[imtl0].Ka[2] = std::stof(str3);
      materials[imtl0].Ka[3] = 1.0;
    }
    if (str0 == "Ks") {
      ss >> str1 >> str2 >> str3;
      materials[imtl0].Ks[0] = std::stof(str1);
      materials[imtl0].Ks[1] = std::stof(str2);
      materials[imtl0].Ks[2] = std::stof(str3);
      materials[imtl0].Ks[3] = 1.0;
    }
    if (str0 == "Ke") {
      ss >> str1 >> str2 >> str3;
      materials[imtl0].Ke[0] = std::stof(str1);
      materials[imtl0].Ke[1] = std::stof(str2);
      materials[imtl0].Ke[2] = std::stof(str3);
      materials[imtl0].Ke[3] = std::stof(str3);
    }
    if (str0 == "Ns") {
      ss >> str1;
      materials[imtl0].Ns = std::stof(str1);
    }
    if (str0 == "illum") {
      ss >> str1;
      materials[imtl0].illum = std::stoi(str1);
    }
    if (str0 == "map_Kd") {
      ss >> str1;
      materials[imtl0].map_Kd = str1;
    }
  }
}

// ----------------------

/*
void delfem2::Shape3_WavefrontObj::ReadObj(
    const std::string &path_obj) {
  std::string fname_mtl;
  Read_WavefrontObjWithSurfaceAttributes2(
      path_obj,
      fname_mtl, aXYZ, aTex, aNorm, aObjGroupTri);
  std::string path_dir = std::string(path_obj.begin(), path_obj.begin() + path_obj.rfind("/"));
  Read_WavefrontMaterial(path_dir + "/" + fname_mtl,
                         aMaterial);
  //  std::cout << aObjGroupTri.size() << " " << aMaterial.size() << std::endl;
  { //
    std::map<std::string, int> mapMtlName2Ind;
    for (int imtl = 0; imtl < (int) aMaterial.size(); ++imtl) {
      mapMtlName2Ind.insert(std::make_pair(aMaterial[imtl].name_mtl, imtl));
    }
    for (auto &iogt: aObjGroupTri) {
      std::string name_mtl = iogt.name_mtl;
      auto itr = mapMtlName2Ind.find(name_mtl);
      if (name_mtl.empty() || itr == mapMtlName2Ind.end()) {
        iogt.idx_material = -1;
        continue;
      }
      iogt.idx_material = itr->second;
    }
  }
}

std::vector<double> delfem2::Shape3_WavefrontObj::AABB3_MinMax() const {
  double c[3], w[3];
  delfem2::CenterWidth_Points3(c, w,
                               aXYZ);
  std::vector<double> aabb(6);
  aabb[0] = c[0] - 0.5 * w[0];
  aabb[1] = c[0] + 0.5 * w[0];
  aabb[2] = c[1] - 0.5 * w[1];
  aabb[3] = c[1] + 0.5 * w[1];
  aabb[4] = c[2] - 0.5 * w[2];
  aabb[5] = c[2] + 0.5 * w[2];
  return aabb;
}

void delfem2::Shape3_WavefrontObj::ScaleXYZ(
    double s) {
  delfem2::Scale_PointsX(aXYZ,
                         s);
}

void delfem2::Shape3_WavefrontObj::TranslateXYZ(
    double x, double y, double z) {
  delfem2::Translate_Points3(aXYZ,
                             x, y, z);
}
*/
