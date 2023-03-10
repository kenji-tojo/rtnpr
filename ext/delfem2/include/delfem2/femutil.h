/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#ifndef DFM2_FEMUTIL_H
#define DFM2_FEMUTIL_H

#include <cassert>
#include <climits>

#include "delfem2/dfm2_inline.h"

namespace delfem2 {


namespace femutil {

// area of a triangle
DFM2_INLINE double TriArea2D(
    const double p0[],
    const double p1[],
    const double p2[]);

// area coordinate inside a triangle
DFM2_INLINE void TriAreaCoord(
    double vc_p[],
    const double p0[],
    const double p1[],
    const double p2[],
    const double pb[]);

DFM2_INLINE double Distance3D(
    const double p0[3],
    const double p1[3]);

DFM2_INLINE double TriArea3D(
    const double v1[3],
    const double v2[3],
    const double v3[3]);

DFM2_INLINE void UnitNormalAreaTri3D(
    double n[3],
    double &a,
    const double v1[3],
    const double v2[3],
    const double v3[3]);

DFM2_INLINE double Dot3D(
    const double a[],
    const double b[]);

DFM2_INLINE void Cross3D(
    double r[3],
    const double v1[3],
    const double v2[3]);

DFM2_INLINE double TetVolume3D(
    const double v1[3],
    const double v2[3],
    const double v3[3],
    const double v4[3]);

DFM2_INLINE void MatVec3(
    double y[3],
    const double m[9],
    const double x[3]);

DFM2_INLINE void MatMat3(
    double *C,
    const double *A,
    const double *B);

DFM2_INLINE void MatMatTrans3(
    double *C,
    const double *A,
    const double *B);

}

// =======================================================================

// derivative of a shape function of a triangle and constant compornent 
DFM2_INLINE void TriDlDx(
    double dldx[][2],
    double const_term[],
    const double p0[],
    const double p1[],
    const double p2[]);

DFM2_INLINE void TetDlDx(
    double dldx[][3],
    double a[],
    const double p0[],
    const double p1[],
    const double p2[],
    const double p3[]);

DFM2_INLINE void ShapeFunc_Vox8(
    const double &r0,
    const double &r1,
    const double &r2,
    const double coords[][3],
    double &detjac,
    double dndx[][3],
    double an[]);

DFM2_INLINE void ShapeFunc_Hex8(
    const double &r0,
    const double &r1,
    const double &r2,
    const double coords[][3],
    double &detjac,
    double dndx[][3],
    double an[]);

template<unsigned int ndim>
void RightCauchyGreen_DispGrad(
    double C[ndim][ndim],
    const double dudx[ndim][ndim]) {
  for (unsigned int idim = 0; idim < ndim; idim++) {
    for (unsigned int jdim = 0; jdim < ndim; jdim++) {
      C[idim][jdim] = dudx[idim][jdim] + dudx[jdim][idim];
      for (unsigned int kdim = 0; kdim < ndim; kdim++) {
        C[idim][jdim] += dudx[kdim][idim] * dudx[kdim][jdim];
      }
    }
    C[idim][idim] += 1.0;
  }
}

template<int ndim, int nno, typename T>
void DispGrad_GradshapeDisp(
    T dudx[ndim][ndim],
    const T dndx[nno][ndim],
    const T aU[nno][ndim]) {
  for (unsigned int idim = 0; idim < ndim; idim++) {
    for (unsigned int jdim = 0; jdim < ndim; jdim++) {
      T dtmp1 = 0;
      for (unsigned int ino = 0; ino < nno; ino++) {
        dtmp1 += aU[ino][idim] * dndx[ino][jdim];
      }
      dudx[idim][jdim] = dtmp1;
    }
  }
}

double DiffShapeFuncAtQuadraturePoint_Hex(
    double dndx[8][3],
    int iGauss, int ir1, int ir2, int ir3,
    const double aP0[8][3]);

template<typename T>
T DetInv_Mat3(
    T Cinv[][3],
    const T C[3][3]) {
  const T p3C =
      +C[0][0] * C[1][1] * C[2][2]
          + C[1][0] * C[2][1] * C[0][2]
          + C[2][0] * C[0][1] * C[1][2]
          - C[0][0] * C[2][1] * C[1][2]
          - C[2][0] * C[1][1] * C[0][2]
          - C[1][0] * C[0][1] * C[2][2];
  // -----
  { // inverse of right Cauchy-Green tensor
    const T inv_det = 1. / p3C;
    Cinv[0][0] = inv_det * (C[1][1] * C[2][2] - C[1][2] * C[2][1]);
    Cinv[0][1] = inv_det * (C[0][2] * C[2][1] - C[0][1] * C[2][2]);
    Cinv[0][2] = inv_det * (C[0][1] * C[1][2] - C[0][2] * C[1][1]);
    Cinv[1][0] = inv_det * (C[1][2] * C[2][0] - C[1][0] * C[2][2]);
    Cinv[1][1] = inv_det * (C[0][0] * C[2][2] - C[0][2] * C[2][0]);
    Cinv[1][2] = inv_det * (C[0][2] * C[1][0] - C[0][0] * C[1][2]);
    Cinv[2][0] = inv_det * (C[1][0] * C[2][1] - C[1][1] * C[2][0]);
    Cinv[2][1] = inv_det * (C[0][1] * C[2][0] - C[0][0] * C[2][1]);
    Cinv[2][2] = inv_det * (C[0][0] * C[1][1] - C[0][1] * C[1][0]);
  }
  return p3C;
}

template<int nno, int ndim, typename T0>
DFM2_INLINE void FetchData(
    double val_to[nno][ndim],
    const unsigned int *aIP,
    const T0 *val_from,
    int nstride = -1) {
  if (nstride == -1) { nstride = ndim; }
  assert(nstride >= ndim);
  for (int ino = 0; ino < nno; ++ino) {
    unsigned int ip = aIP[ino];
    for (int idim = 0; idim < ndim; ++idim) {
      val_to[ino][idim] = val_from[ip * nstride + idim];
    }
  }
}

template<int ndofno>
DFM2_INLINE void AddEmatConsistentMassTet(
    double eM[4][4][ndofno][ndofno],
    double w0) {
  const double dtmp1 = w0 * 0.05;
  for (int ino = 0; ino < 4; ino++) {
    for (int jno = 0; jno < 4; jno++) {
      eM[ino][jno][0][0] += dtmp1;
      eM[ino][jno][1][1] += dtmp1;
      eM[ino][jno][2][2] += dtmp1;
    }
    {
      eM[ino][ino][0][0] += dtmp1;
      eM[ino][ino][1][1] += dtmp1;
      eM[ino][ino][2][2] += dtmp1;
    }
  }
}

DFM2_INLINE void SetEMatLaplaceTet(
    double C[4][4][3][3],
    double w0,
    const double dldx[4][3]);

DFM2_INLINE void SetEMatLaplaceTet(
    double C[4][4],
    double w0,
    const double dldx[4][3]);

DFM2_INLINE void SetEmatConsistentMassTet(
    double eM[4][4][3][3],
    double w0);

template<int ndofno>
DFM2_INLINE void EmatConsistentMassTri2(
    double eM[3][3][ndofno][ndofno],
    double w0,
    bool is_add) {
  if (!is_add) {
    for (unsigned int i = 0; i < 3 * 3 * ndofno * ndofno; ++i) { (&eM[0][0][0][0])[i] = 0.0; }
  }
  const double dtmp1 = w0 * 0.0833333333333333333333333333;
  for (int ino = 0; ino < 3; ino++) {
    for (int jno = 0; jno < 3; jno++) {
      eM[ino][jno][0][0] += dtmp1;
      eM[ino][jno][1][1] += dtmp1;
    }
    {
      eM[ino][ino][0][0] += dtmp1;
      eM[ino][ino][1][1] += dtmp1;
    }
  }
}

template<int nno>
void AddEmatEvecScale3(
    double R[nno][3],
    const double eM[nno][nno][3][3],
    const double v0[nno][3],
    double scale) {
  for (int ino = 0; ino < nno; ino++) {
    for (int jno = 0; jno < nno; jno++) {
      R[ino][0] +=
          scale * (eM[ino][jno][0][0] * v0[jno][0] + eM[ino][jno][0][1] * v0[jno][1] + eM[ino][jno][0][2] * v0[jno][2]);
      R[ino][1] +=
          scale * (eM[ino][jno][1][0] * v0[jno][0] + eM[ino][jno][1][1] * v0[jno][1] + eM[ino][jno][1][2] * v0[jno][2]);
      R[ino][2] +=
          scale * (eM[ino][jno][2][0] * v0[jno][0] + eM[ino][jno][2][1] * v0[jno][1] + eM[ino][jno][2][2] * v0[jno][2]);
    }
  }
}

} // namespace delfem2


#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/femutil.cpp"
#endif

#endif /* DFM2_FEMUTIL_H */
