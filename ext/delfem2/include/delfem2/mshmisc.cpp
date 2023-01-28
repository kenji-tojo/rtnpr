/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/mshmisc.h"

#include <cassert>
#include <cmath>
#include <vector>
#include <functional>
#include <array>

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

// ------------------------------------------------

namespace delfem2::mshmisc {

//! @details we have "float" and "double" versions Length3 because of sqrtf and sqrt
DFM2_INLINE double Length3(const double p[3]) {
  return sqrt(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
}

//! @details we have "float" and "double" versions Length3 because of sqrtf and sqrt
DFM2_INLINE float Length3(const float p[3]) {
  return sqrtf(p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
}

DFM2_INLINE void Cross3D(double r[3], const double v1[3], const double v2[3]) {
  r[0] = v1[1] * v2[2] - v2[1] * v1[2];
  r[1] = v1[2] * v2[0] - v2[2] * v1[0];
  r[2] = v1[0] * v2[1] - v2[0] * v1[1];
}

DFM2_INLINE double TriArea2D(const double p0[], const double p1[], const double p2[]) {
  return 0.5 * ((p1[0] - p0[0]) * (p2[1] - p0[1]) - (p2[0] - p0[0]) * (p1[1] - p0[1]));
}

template<typename T>
DFM2_INLINE T TriArea3D(
    const T v1[3],
    const T v2[3],
    const T v3[3]) {
  T n[3];
  n[0] = (v2[1] - v1[1]) * (v3[2] - v1[2]) - (v3[1] - v1[1]) * (v2[2] - v1[2]);
  n[1] = (v2[2] - v1[2]) * (v3[0] - v1[0]) - (v3[2] - v1[2]) * (v2[0] - v1[0]);
  n[2] = (v2[0] - v1[0]) * (v3[1] - v1[1]) - (v3[0] - v1[0]) * (v2[1] - v1[1]);
  return std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]) / 2;
}

template<typename T>
DFM2_INLINE void UnitNormalAreaTri3(
    T n[3],
    T &a,
    const T v1[3],
    const T v2[3],
    const T v3[3]) {
  n[0] = (v2[1] - v1[1]) * (v3[2] - v1[2]) - (v3[1] - v1[1]) * (v2[2] - v1[2]);
  n[1] = (v2[2] - v1[2]) * (v3[0] - v1[0]) - (v3[2] - v1[2]) * (v2[0] - v1[0]);
  n[2] = (v2[0] - v1[0]) * (v3[1] - v1[1]) - (v3[0] - v1[0]) * (v2[1] - v1[1]);
  a = std::sqrt(n[0] * n[0] + n[1] * n[1] + n[2] * n[2]) / 2;
  const T invlen = 1 / (a * 2);
  n[0] *= invlen;
  n[1] *= invlen;
  n[2] *= invlen;
}

template<typename T>
std::array<T,3> Normal_Tri3(
    const T v1[3],
    const T v2[3],
    const T v3[3]) {
  return {
      (v2[1] - v1[1]) * (v3[2] - v1[2]) - (v3[1] - v1[1]) * (v2[2] - v1[2]),
      (v2[2] - v1[2]) * (v3[0] - v1[0]) - (v3[2] - v1[2]) * (v2[0] - v1[0]),
      (v2[0] - v1[0]) * (v3[1] - v1[1]) - (v3[0] - v1[0]) * (v2[1] - v1[1]) };
}

template<typename T>
DFM2_INLINE void MatVec3(T y[3],
                         const T m[9], const T x[3]) {
  y[0] = m[0] * x[0] + m[1] * x[1] + m[2] * x[2];
  y[1] = m[3] * x[0] + m[4] * x[1] + m[5] * x[2];
  y[2] = m[6] * x[0] + m[7] * x[1] + m[8] * x[2];
}

//! @details we have "float" and "double" versions Distance3 because of sqrtf and sqrt
DFM2_INLINE double Distance3(const double p0[3], const double p1[3]) {
  return sqrt(
      (p1[0] - p0[0]) * (p1[0] - p0[0]) +
      (p1[1] - p0[1]) * (p1[1] - p0[1]) +
      (p1[2] - p0[2]) * (p1[2] - p0[2]));
}

//! @details we have "float" and "double" versions Distance3 because of sqrtf and sqrt
DFM2_INLINE float Distance3(const float p0[3], const float p1[3]) {
  return sqrtf(
      (p1[0] - p0[0]) * (p1[0] - p0[0]) +
      (p1[1] - p0[1]) * (p1[1] - p0[1]) +
      (p1[2] - p0[2]) * (p1[2] - p0[2]));
}

DFM2_INLINE double Distance2D(const double p0[3], const double p1[3]) {
  return sqrt((p1[0] - p0[0]) * (p1[0] - p0[0]) + (p1[1] - p0[1]) * (p1[1] - p0[1]));
}

DFM2_INLINE double Dot3(const double p0[3], const double p1[3]) {
  return p0[0] * p1[0] + p0[1] * p1[1] + p0[2] * p1[2];
}

DFM2_INLINE double largest(double x0, double x1, double x2) {
  double wmax = x0;
  wmax = (x1 > wmax) ? x1 : wmax;
  wmax = (x2 > wmax) ? x2 : wmax;
  return wmax;
}

template<typename T>
DFM2_INLINE  T TetVolume3D(
    const T v1[3],
    const T v2[3],
    const T v3[3],
    const T v4[3]) {
  return
      ((v2[0] - v1[0]) * ((v3[1] - v1[1]) * (v4[2] - v1[2]) - (v4[1] - v1[1]) * (v3[2] - v1[2]))
          - (v2[1] - v1[1]) * ((v3[0] - v1[0]) * (v4[2] - v1[2]) - (v4[0] - v1[0]) * (v3[2] - v1[2]))
          + (v2[2] - v1[2]) * ((v3[0] - v1[0]) * (v4[1] - v1[1]) - (v4[0] - v1[0]) * (v3[1] - v1[1]))
      ) * static_cast<T>(1.0 / 6.0);
}

DFM2_INLINE void Mat3_Bryant
    (double m[9],
     double rx, double ry, double rz) {
  m[0] = cos(rz) * cos(ry);
  m[1] = cos(rz) * sin(ry) * sin(rx) - sin(rz) * cos(rx);
  m[2] = cos(rz) * sin(ry) * cos(rx) + sin(rz) * sin(rx);
  m[3] = sin(rz) * cos(ry);
  m[4] = sin(rz) * sin(ry) * sin(rx) + cos(rz) * cos(rx);
  m[5] = sin(rz) * sin(ry) * cos(rx) - cos(rz) * sin(rx);
  m[6] = -sin(ry);
  m[7] = cos(ry) * sin(rx);
  m[8] = cos(ry) * cos(rx);
}

DFM2_INLINE void Mat3_Bryant
    (float m[9],
     float rx, float ry, float rz) {
  m[0] = cosf(rz) * cosf(ry);
  m[1] = cosf(rz) * sinf(ry) * sinf(rx) - sinf(rz) * cosf(rx);
  m[2] = cosf(rz) * sinf(ry) * cosf(rx) + sinf(rz) * sinf(rx);
  m[3] = sinf(rz) * cosf(ry);
  m[4] = sinf(rz) * sinf(ry) * sinf(rx) + cosf(rz) * cosf(rx);
  m[5] = sinf(rz) * sinf(ry) * cosf(rx) - cosf(rz) * sinf(rx);
  m[6] = -sinf(ry);
  m[7] = cosf(ry) * sinf(rx);
  m[8] = cosf(ry) * cosf(rx);
}

template<typename T>
void CenterWidth_MinMaxXYZ
    (T &cx, T &cy, T &cz,
     T &wx, T &wy, T &wz,
        //
     T x_min, T x_max,
     T y_min, T y_max,
     T z_min, T z_max) {
  cx = (x_min + x_max) * 0.5;
  cy = (y_min + y_max) * 0.5;
  cz = (z_min + z_max) * 0.5;
  wx = x_max - x_min;
  wy = y_max - y_min;
  wz = z_max - z_min;
}

template<typename T>
void updateMinMaxXYZ(
    T &x_min, T &x_max,
    T &y_min, T &y_max,
    T &z_min, T &z_max,
    T x, T y, T z) {
  if (x_min > x_max) {
    x_min = x_max = x;
    y_min = y_max = y;
    z_min = z_max = z;
    return;
  }
  x_min = (x_min < x) ? x_min : x;
  x_max = (x_max > x) ? x_max : x;
  y_min = (y_min < y) ? y_min : y;
  y_max = (y_max > y) ? y_max : y;
  z_min = (z_min < z) ? z_min : z;
  z_max = (z_max > z) ? z_max : z;
}
#ifdef DFM2_STATIC_LIBRARY
template void updateMinMaxXYZ(
    float &x_min, float &x_max,
    float &y_min, float &y_max,
    float &z_min, float &z_max,
    float X, float Y, float Z);
template void updateMinMaxXYZ(
    double &x_min, double &x_max,
    double &y_min, double &y_max,
    double &z_min, double &z_max,
    double X, double Y, double Z);
#endif

}

// static function above
// ==============================================
// exposed function below


// -------------------------------------

// ----------------------------------------------

void delfem2::RemoveUnreferencedPoints_MeshElem(
    std::vector<double> &aXYZ1,
    std::vector<unsigned int> &aElem1,
    std::vector<int> &aMap01,
    unsigned int ndim,
    const std::vector<double> &aXYZ0,
    const std::vector<unsigned int> &aElem0) {
  const size_t np0 = aXYZ0.size() / ndim;
  aMap01.assign(np0, -2);
  for (unsigned int ip : aElem0) {
    aMap01[ip] = -1;
  }
  int npj = 0;
  for (unsigned int ip = 0; ip < np0; ++ip) {
    if (aMap01[ip] == -2) continue;
    aMap01[ip] = npj;
    npj++;
  }
  aXYZ1.resize(npj * ndim);
  for (unsigned int ip = 0; ip < np0; ++ip) {
    if (aMap01[ip] == -2) continue;
    int jp = aMap01[ip];
    for (unsigned int idim = 0; idim < ndim; ++idim) {
      aXYZ1[jp * ndim + idim] = aXYZ0[ip * ndim + idim];
    }
  }
  aElem1.resize(aElem0.size());
  for (std::size_t it = 0; it < aElem0.size(); ++it) {
    unsigned int ip = aElem0[it];
    int jp = aMap01[ip];
    aElem1[it] = jp;
  }
}

// ---------------------------------

void delfem2::Quality_MeshTri2D(
    double &max_aspect,
    double &min_area,
    const double *aXY,
    const unsigned int *aTri,
    unsigned int nTri) {
  max_aspect = 0;
  min_area = 0;
  for (unsigned int itri = 0; itri < nTri; itri++) {
    const unsigned int i0 = aTri[itri * 3 + 0];
    const unsigned int i1 = aTri[itri * 3 + 1];
    const unsigned int i2 = aTri[itri * 3 + 2];
    const double *p0 = aXY + i0 * 2;
    const double *p1 = aXY + i1 * 2;
    const double *p2 = aXY + i2 * 2;
    const double area = mshmisc::TriArea2D(p0, p1, p2);
    const double len01 = mshmisc::Distance2D(p0, p1);
    const double len12 = mshmisc::Distance2D(p1, p2);
    const double len20 = mshmisc::Distance2D(p2, p0);
    const double len_ave = (len01 + len12 + len20) / 3.0;
    const double aspect = len_ave * len_ave / area;
    if (itri == 0) {
      max_aspect = aspect;
      min_area = area;
    } else {
      if (aspect > max_aspect) { max_aspect = aspect; }
      if (area < min_area) { min_area = area; }
    }
  }
}

// -----------------------------------------------------------------------------------------

void delfem2::SetTopology_ExtrudeTri2Tet(
  unsigned int *aTet,
  int nXY,
  const unsigned int *aTri,
  int nTri,
  int nlayer) {
  for (int il = 0; il < nlayer; ++il) {
    for (int itri = 0; itri < nTri; ++itri) {
      unsigned int ip0 = 0, ip1 = 0, ip2 = 0;
      {
        const unsigned int i0 = aTri[itri * 3 + 0];
        const unsigned int i1 = aTri[itri * 3 + 1];
        const unsigned int i2 = aTri[itri * 3 + 2];
        assert(i0 != i1 && i1 != i2);
        if (i0 > i1 && i0 > i2) {
          ip0 = i0;
          ip1 = i1;
          ip2 = i2;
        }
        if (i1 > i0 && i1 > i2) {
          ip0 = i1;
          ip1 = i2;
          ip2 = i0;
        }
        if (i2 > i0 && i2 > i1) {
          ip0 = i2;
          ip1 = i0;
          ip2 = i1;
        }
      }
      const unsigned int aIQ[6] = {
          (il + 0) * nXY + ip0, (il + 1) * nXY + ip0,
          (il + 0) * nXY + ip1, (il + 1) * nXY + ip1,
          (il + 0) * nXY + ip2, (il + 1) * nXY + ip2};
      aTet[il * nTri * 12 + itri * 12 + 4 * 0 + 0] = aIQ[0];
      aTet[il * nTri * 12 + itri * 12 + 4 * 0 + 1] = aIQ[2];
      aTet[il * nTri * 12 + itri * 12 + 4 * 0 + 2] = aIQ[4];
      aTet[il * nTri * 12 + itri * 12 + 4 * 0 + 3] = aIQ[1];
      if (ip1 > ip2) {
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 0] = aIQ[1];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 1] = aIQ[3];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 2] = aIQ[2];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 3] = aIQ[4];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 0] = aIQ[1];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 1] = aIQ[3];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 2] = aIQ[4];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 3] = aIQ[5];
      } else {
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 0] = aIQ[1];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 1] = aIQ[2];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 2] = aIQ[5];
        aTet[il * nTri * 12 + itri * 12 + 4 * 1 + 3] = aIQ[3];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 0] = aIQ[1];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 1] = aIQ[2];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 2] = aIQ[4];
        aTet[il * nTri * 12 + itri * 12 + 4 * 2 + 3] = aIQ[5];
      }
    }
  }
}

void delfem2::ExtrudeTri2Tet
    (int nlayer, double h,
     std::vector<double> &aXYZ,
     std::vector<unsigned int> &aTet,
     const std::vector<double> &aXY,
     const std::vector<unsigned int> &aTri) {
  const int nXY = (int) aXY.size() / 2;
  const int nTri = (int) aTri.size() / 3;
  aXYZ.resize(nXY * (nlayer + 1) * 3);
  for (int il = 0; il < nlayer + 1; ++il) {
    for (int ixy = 0; ixy < nXY; ixy++) {
      aXYZ[il * nXY * 3 + ixy * 3 + 0] = aXY[ixy * 2 + 0];
      aXYZ[il * nXY * 3 + ixy * 3 + 1] = aXY[ixy * 2 + 1];
      aXYZ[il * nXY * 3 + ixy * 3 + 2] = il * h;
    }
  }
  aTet.resize(nTri * nlayer * 3 * 4);
  SetTopology_ExtrudeTri2Tet(aTet.data(),
                             nXY, aTri.data(), nTri, nlayer);
}

// -----------------------------------------------------------------------

void delfem2::LaplacianSmoothing(
    std::vector<double> &vtx_xyz,
    const std::vector<int> &tri_vtx,
    const std::vector<int> &elsup_ind,
    const std::vector<int> &elsup) {
  for (std::size_t ip = 0; ip < vtx_xyz.size() / 3; ++ip) {
    double sum_area = 0.0;
    double pcnt[3] = {0, 0, 0};
    for (int ielsup = elsup_ind[ip]; ielsup < elsup_ind[ip + 1]; ++ielsup) {
      assert(ielsup < (int) elsup.size());
      int iel = elsup[ielsup];
      assert(iel >= 0 && iel < (int) tri_vtx.size() / 3);
      int i0 = tri_vtx[iel * 3 + 0];
      int i1 = tri_vtx[iel * 3 + 1];
      int i2 = tri_vtx[iel * 3 + 2];
      double aP[3][3] = {
          {vtx_xyz[i0 * 3 + 0], vtx_xyz[i0 * 3 + 1], vtx_xyz[i0 * 3 + 2]},
          {vtx_xyz[i1 * 3 + 0], vtx_xyz[i1 * 3 + 1], vtx_xyz[i1 * 3 + 2]},
          {vtx_xyz[i2 * 3 + 0], vtx_xyz[i2 * 3 + 1], vtx_xyz[i2 * 3 + 2]}};
      double area = mshmisc::TriArea3D(aP[0], aP[1], aP[2]);
      sum_area += area;
      pcnt[0] += area * (aP[0][0] + aP[1][0] + aP[2][0]) / 3.0;
      pcnt[1] += area * (aP[0][1] + aP[1][1] + aP[2][1]) / 3.0;
      pcnt[2] += area * (aP[0][2] + aP[1][2] + aP[2][2]) / 3.0;
    }
    pcnt[0] /= sum_area;
    pcnt[1] /= sum_area;
    pcnt[2] /= sum_area;
    vtx_xyz[ip * 3 + 0] = pcnt[0];
    vtx_xyz[ip * 3 + 1] = pcnt[1];
    vtx_xyz[ip * 3 + 2] = pcnt[2];
  }
}

/*
void LaplacianSmoothing_Cotan
(std::vector<double>& aXYZ,
 const std::vector<int>& aTri,
 const std::vector<int>& elsup_ind,
 const std::vector<int>& elsup)
{
  const int np = (int)aXYZ.size()/3;
  std::vector<double> aTmp(np,0.0);
  //  std::vector<double> aDist = aXYZ;
  for(int ip=0;ip<np;++ip){
    const CVector3 p0(aXYZ[ip*3+0],aXYZ[ip*3+1],aXYZ[ip*3+2]);
    double volo_area = 0;
    ////////////////////////////////////////////////////////
    for(int ielsup=elsup_ind[ip];ielsup<elsup_ind[ip+1];++ielsup){
      assert( ielsup < elsup.size() );
      int iel = elsup[ielsup];
      assert( iel>=0 && iel<aTri.size()/3 );
      const int i0 = aTri[iel*3+0];
      const int i1 = aTri[iel*3+1];
      const int i2 = aTri[iel*3+2];
      const int aIP[3] = {i0,i1,i2};
      const CVector3 aP[3] = {
        CVector3(aXYZ[i0*3+0],aXYZ[i0*3+1],aXYZ[i0*3+2]),
        CVector3(aXYZ[i1*3+0],aXYZ[i1*3+1],aXYZ[i1*3+2]),
        CVector3(aXYZ[i2*3+0],aXYZ[i2*3+1],aXYZ[i2*3+2]) };
      const double area = TriArea(aP[0],aP[1],aP[2]);
      for(int ie=0;ie<3;++ie){
        if( aIP[ie] == ip ) continue;
        const int ie1 = (ie+1)%3;
        const int ie2 = (ie+2)%3;
        assert( aIP[ie1] == ip || aIP[ie2] == ip );
        const CVector3 v01 = aP[ie1]-aP[ie];
        const CVector3 v02 = aP[ie2]-aP[ie];
        const CVector3 v12 = aP[ie2]-aP[ie1];
        const double cot102 = (v01*v02)/area*2;
        if( v01*v02>0 && v02*v12>0 && v01*v12<0 ){
          volo_area += v12.DLength()*cot102/0.125;
        }
        else if( v01*v02>0 ){
          volo_area += area*0.25;
        }
        else{
          volo_area += area*0.5;
        }
        aTmp[ aIP[ie1] ] += cot102;
        aTmp[ aIP[ie2] ] += cot102;
        //        std::cout << ip << " " << iel << " " << ie << " " << cot102 << std::endl;
      }
    }
    ////////////////////////////////////////////////////////
    CVector3 pcnt(0,0,0);
    double sum = 0.0;
    for(int ielsup=elsup_ind[ip];ielsup<elsup_ind[ip+1];++ielsup){
      assert( ielsup < elsup.size() );
      int iel = elsup[ielsup];
      assert( iel>=0 && iel<aTri.size()/3 );
      const int i0 = aTri[iel*3+0];
      const int i1 = aTri[iel*3+1];
      const int i2 = aTri[iel*3+2];
      const int aIP[3] = {i0,i1,i2};
      for(int ie=0;ie<3;++ie){
        if( aIP[ie] != ip ) continue;
        const int ie1 = (ie+1)%3;
        const int ie2 = (ie+2)%3;
        const int j1 = aIP[ie1];
        const int j2 = aIP[ie2];
        assert( j1 != ip );
        assert( j2 != ip );
        pcnt += aTmp[j1]*(CVector3(aXYZ[j1*3+0],aXYZ[j1*3+1],aXYZ[j1*3+2])-p0)*0.5;
        pcnt += aTmp[j2]*(CVector3(aXYZ[j2*3+0],aXYZ[j2*3+1],aXYZ[j2*3+2])-p0)*0.5;
      }
    }
    pcnt /= volo_area*2;
    double eps = 0.01;
    aXYZ[ip*3+0] += pcnt.x*eps;
    aXYZ[ip*3+1] += pcnt.y*eps;
    aXYZ[ip*3+2] += pcnt.z*eps;
    ////////////////////////////////////////////////////////
    for(int ielsup=elsup_ind[ip];ielsup<elsup_ind[ip+1];++ielsup){
      assert( ielsup < elsup.size() );
      const int iel = elsup[ielsup];
      assert( iel>=0 && iel<aTri.size()/3 );
      const int i0 = aTri[iel*3+0];
      const int i1 = aTri[iel*3+1];
      const int i2 = aTri[iel*3+2];
      aTmp[i0] = 0.0;
      aTmp[i1] = 0.0;
      aTmp[i2] = 0.0;
    }
  }
  //  aXYZ = aDist;
}
 */


DFM2_INLINE double delfem2::SolidAngleTri3D(
    const double v1[3],
    const double v2[3],
    const double v3[3]) {
  double l1 = mshmisc::Length3(v1);
  double l2 = mshmisc::Length3(v2);
  double l3 = mshmisc::Length3(v3);
  double crs_v1_v2[3];
  mshmisc::Cross3D(crs_v1_v2, v1, v2);
  double den = mshmisc::Dot3(crs_v1_v2, v3);
  double num = l1 * l2 * l3
      + (mshmisc::Dot3(v1, v2)) * l3
      + (mshmisc::Dot3(v2, v3)) * l1
      + (mshmisc::Dot3(v3, v1)) * l2;
  double tho = den / num;
  double v = atan(tho);
  if (v < 0) { v += 2 * M_PI; }
  v *= 2;
  return v;
}

void delfem2::makeSolidAngle(
    std::vector<double> &aSolidAngle,
    const std::vector<double> &aXYZ,
    const std::vector<unsigned int> &aTri,
    const std::vector<double> &aNorm,
    std::vector<int> &elsup_ind,
    std::vector<int> &elsup) {
  const size_t nXYZ = aXYZ.size() / 3;
  aSolidAngle.resize(nXYZ);
  for (unsigned int ip = 0; ip < nXYZ; ++ip) {
    const double n0[3] = {aNorm[ip * 3 + 0], aNorm[ip * 3 + 1], aNorm[ip * 3 + 2]};
    const double p0[3] = {aXYZ[ip * 3 + 0], aXYZ[ip * 3 + 1], aXYZ[ip * 3 + 2]};
    double sa = 0;
    for (int ielsup = elsup_ind[ip]; ielsup < elsup_ind[ip + 1]; ++ielsup) {
      int itri0 = elsup[ielsup];
      assert(itri0 >= 0 && itri0 < (int) aTri.size() / 3);
      int inotri0 = -1;
      for (int i = 0; i < 3; ++i) {
        if (aTri[itri0 * 3 + i] != ip) { continue; }
        inotri0 = i;
        break;
      }
      int inotri1 = (inotri0 + 1) % 3;
      int inotri2 = (inotri0 + 2) % 3;
      unsigned int ip1 = aTri[itri0 * 3 + inotri1];
      unsigned int ip2 = aTri[itri0 * 3 + inotri2];
      const double p1[3] = {aXYZ[ip1 * 3 + 0], aXYZ[ip1 * 3 + 1], aXYZ[ip1 * 3 + 2]};
      const double p2[3] = {aXYZ[ip2 * 3 + 0], aXYZ[ip2 * 3 + 1], aXYZ[ip2 * 3 + 2]};
      const double p10[3] = {p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2]};
      const double p20[3] = {p2[0] - p0[0], p2[1] - p0[1], p2[2] - p0[2]};
      sa += SolidAngleTri3D(p10, p20, n0);
    }
    if (elsup_ind[ip + 1] - elsup_ind[ip] == 0) {
      sa = -1.0; // floting point: negative
    }
    aSolidAngle[ip] = sa;
  }
}

void delfem2::MassPoint_Tet3D(
	double *aMassMatrixLumped,
	double rho,    
	const double *aXYZ, 
	size_t nXYZ,
    const unsigned int *aTet, 
	size_t nTet) {
  for (unsigned int i = 0; i < nXYZ; ++i) { aMassMatrixLumped[i] = 0.0; }
  for (unsigned int it = 0; it < nTet; ++it) {
    const unsigned int i0 = aTet[it * 4 + 0];
    assert(i0 < nXYZ);
    const unsigned int i1 = aTet[it * 4 + 1];
    assert(i1 < nXYZ);
    const unsigned int i2 = aTet[it * 4 + 2];
    assert(i2 < nXYZ);
    const unsigned int i3 = aTet[it * 4 + 3];
    assert(i3 < nXYZ);
    const double *p0 = aXYZ + i0 * 3;
    const double *p1 = aXYZ + i1 * 3;
    const double *p2 = aXYZ + i2 * 3;
    const double *p3 = aXYZ + i3 * 3;
    const double v0123 = mshmisc::TetVolume3D(p0, p1, p2, p3);
    aMassMatrixLumped[i0] += 0.25 * rho * v0123;
    aMassMatrixLumped[i1] += 0.25 * rho * v0123;
    aMassMatrixLumped[i2] += 0.25 * rho * v0123;
    aMassMatrixLumped[i3] += 0.25 * rho * v0123;
  }
}

void delfem2::MassPoint_Tri2D(
    double *aMass,
    double rho,
    const double *aXY,
    size_t nXY,
    const unsigned int *aTri,
    size_t nTri) {
  for (unsigned int i = 0; i < nXY; ++i) { aMass[i] = 0.0; }
  for (unsigned int it = 0; it < nTri; ++it) {
    const unsigned int i0 = aTri[it * 3 + 0];
    const unsigned int i1 = aTri[it * 3 + 1];
    const unsigned int i2 = aTri[it * 3 + 2];
    assert(i0 < nXY);
    assert(i1 < nXY);
    assert(i2 < nXY);
    const double *p0 = aXY + i0 * 2;
    const double *p1 = aXY + i1 * 2;
    const double *p2 = aXY + i2 * 2;
    const double a012 = mshmisc::TriArea2D(p0, p1, p2);
    aMass[i0] += rho * a012 / 3.0;
    aMass[i1] += rho * a012 / 3.0;
    aMass[i2] += rho * a012 / 3.0;
  }
}

void delfem2::MassPoint_Tri3D(
    double *aMass,
    double rho,
    const double *aXYZ,
    size_t nXYZ,
    const unsigned int *aTri,
    size_t nTri) {
  for (unsigned int ip = 0; ip < nXYZ; ++ip) { aMass[ip] = 0.0; }
  for (unsigned int it = 0; it < nTri; ++it) {
    const unsigned int i0 = aTri[it * 3 + 0];
    const unsigned int i1 = aTri[it * 3 + 1];
    const unsigned int i2 = aTri[it * 3 + 2];
    const double a012 = mshmisc::TriArea3D(
        aXYZ + i0 * 3,
        aXYZ + i1 * 3,
        aXYZ + i2 * 3);
    aMass[i0] += rho * a012 / 3;
    aMass[i1] += rho * a012 / 3;
    aMass[i2] += rho * a012 / 3;
  }
}

DFM2_INLINE void delfem2::AddMesh(
    std::vector<double> &aXYZ,
    std::vector<unsigned int> &aTri,
    const std::vector<double> &aXYZ0,
    const std::vector<unsigned int> &aTri0) {
  const unsigned int np0 = (unsigned int) aXYZ.size() / 3;
  aXYZ.reserve(aXYZ.size() + aXYZ0.size());
  aTri.reserve(aTri.size() + aTri0.size());
  for (double p: aXYZ0) { aXYZ.push_back(p); }
  for (unsigned int i: aTri0) { aTri.push_back(i + np0); }
}


DFM2_INLINE double delfem2::Area_MeshTri3(
    const std::vector<double> &vtx_xyz,
    const std::vector<unsigned int> &tri_vtx,
    const std::function<bool(unsigned int)>& flag){
  double area_sum = 0;
  for(unsigned int it=0;it<tri_vtx.size()/3;++it){
    if( !flag(it) ){ continue; }
    unsigned int ip0 = tri_vtx[it*3+0];
    unsigned int ip1 = tri_vtx[it*3+1];
    unsigned int ip2 = tri_vtx[it*3+2];
    area_sum += mshmisc::TriArea3D(
        vtx_xyz.data() + ip0*3,
        vtx_xyz.data() + ip1*3,
        vtx_xyz.data() + ip2*3);
  }
  return area_sum;
}


DFM2_INLINE bool delfem2::Distortion_MappingTriangleFrom2To3Dim(
    double thresA,
    double thresE,
    unsigned int it0,
    const std::vector<unsigned int> &aTri,
    const std::vector<double> &aXYZ,
    const std::vector<double> &aTexP) { // check the distortion
  namespace lcl = ::delfem2::mshmisc;
  const unsigned int i0 = aTri[it0 * 3 + 0];
  const unsigned int i1 = aTri[it0 * 3 + 1];
  const unsigned int i2 = aTri[it0 * 3 + 2];
  const double area2 = lcl::TriArea2D(aTexP.data() + i0 * 2, aTexP.data() + i1 * 2, aTexP.data() + i2 * 2);
  const double area3 = lcl::TriArea3D(aXYZ.data() + i0 * 3, aXYZ.data() + i1 * 3, aXYZ.data() + i2 * 3);
  const double scoreArea = 0.5 * (area2 / area3 + area3 / area2);
  if (scoreArea < 0 || scoreArea > thresA) { return true; }
  const double len12 = lcl::Distance2D(aTexP.data() + i1 * 2, aTexP.data() + i2 * 2);
  const double len20 = lcl::Distance2D(aTexP.data() + i2 * 2, aTexP.data() + i0 * 2);
  const double len01 = lcl::Distance2D(aTexP.data() + i0 * 2, aTexP.data() + i1 * 2);
  const double Len12 = lcl::Distance3(aXYZ.data() + i1 * 3, aXYZ.data() + i2 * 3);
  const double Len20 = lcl::Distance3(aXYZ.data() + i2 * 3, aXYZ.data() + i0 * 3);
  const double Len01 = lcl::Distance3(aXYZ.data() + i0 * 3, aXYZ.data() + i1 * 3);
  if (0.5 * (len12 / Len12 + Len12 / len12) > thresE) { return true; }
  if (0.5 * (len20 / Len20 + Len20 / len20) > thresE) { return true; }
  if (0.5 * (len01 / Len01 + Len01 / len01) > thresE) { return true; }
  return false;
}

// ======================================

