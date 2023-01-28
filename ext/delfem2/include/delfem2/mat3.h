/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

/**
 * @file 3x3 matrix class (CMat3) and functions
 * @detail The order of dependency in delfem2 is "vec2 -> mat2 -> vec3 -> quaternion -> mat3 -> mat4",
 */


#ifndef DFM2_MAT3_H
#define DFM2_MAT3_H

#include <vector>
#include <cassert>
#include <cmath>
#include <iostream>
#include <array>
#include <limits> // using NaN Check

#include "delfem2/dfm2_inline.h"
#include "delfem2/geo_meta_funcs.h"

// -----------------------------

namespace delfem2 {

static inline bool myIsNAN_Matrix3(double d) { return !(d > d - 1); }

/**
 * @class class of 3x3 matrix
 */
template<typename REAL>
class CMat3 {
 public:
  CMat3();
  explicit CMat3(REAL s);
  explicit CMat3(const REAL m[9]);
  CMat3(REAL v00, REAL v01, REAL v02,
        REAL v10, REAL v11, REAL v12,
        REAL v20, REAL v21, REAL v22);
  CMat3(REAL x, REAL y, REAL z);
  CMat3(const std::array<REAL, 9> &m) :
      p_{m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8]} {}
  // ---------------
  REAL *data() { return p_; }
  [[nodiscard]] const REAL *data() const { return p_; }
  // ---------------
  void GetElements(REAL m[9]) const { for (unsigned int i = 0; i < 9; i++) { m[i] = p_[i]; }}
  [[nodiscard]] double Get(int i, int j) const { return p_[i * 3 + j]; }
  // ---------
  void AffineMatrixTrans(REAL m[16]) const {
    m[0 * 4 + 0] = p_[0];
    m[1 * 4 + 0] = p_[1];
    m[2 * 4 + 0] = p_[2];
    m[3 * 4 + 0] = 0;
    m[0 * 4 + 1] = p_[3];
    m[1 * 4 + 1] = p_[4];
    m[2 * 4 + 1] = p_[5];
    m[3 * 4 + 1] = 0;
    m[0 * 4 + 2] = p_[6];
    m[1 * 4 + 2] = p_[7];
    m[2 * 4 + 2] = p_[8];
    m[3 * 4 + 2] = 0;
    m[0 * 4 + 3] = 0;
    m[1 * 4 + 3] = 0;
    m[2 * 4 + 3] = 0;
    m[3 * 4 + 3] = 1;
  }
  void CopyToMat4(REAL m[16]) const {
    m[0 * 4 + 0] = p_[0];
    m[0 * 4 + 1] = p_[1];
    m[0 * 4 + 2] = p_[2];
    m[1 * 4 + 0] = p_[3];
    m[1 * 4 + 1] = p_[4];
    m[1 * 4 + 2] = p_[5];
    m[2 * 4 + 0] = p_[6];
    m[2 * 4 + 1] = p_[7];
    m[2 * 4 + 2] = p_[8];
  }
  void CopyTo(REAL *ptr) const {
    for (int i = 0; i < 9; ++i) { ptr[i] = p_[i]; }
  }
  void CopyToScale(REAL *ptr, REAL s) const {
    for (int i = 0; i < 9; ++i) { ptr[i] = p_[i] * s; }
  }
  void AddToScale(REAL *ptr, REAL s) const {
    for (int i = 0; i < 9; ++i) { ptr[i] += p_[i] * s; }
  }
  // ---------------
  [[nodiscard]] std::array<REAL, 3> MatVec(const REAL vec0[3]) const;
  void MatVecTrans(const REAL vec0[], REAL vec1[]) const;

  [[nodiscard]] CMat3 MatMat(const CMat3 &mat0) const;
  [[nodiscard]] CMat3 MatMatTrans(const CMat3 &mat0) const;
  // ----------------
  [[nodiscard]] CMat3 Sym() const {
    CMat3 m;
    for (unsigned int i = 0; i < 3; i++) {
      for (unsigned int j = 0; j < 3; j++) {
        m.p_[i * 3 + j] = (p_[i * 3 + j] + p_[j * 3 + i]) * 0.5;
      }
    }
    return m;
  }
  // -----------
  // below: operator
  inline CMat3 operator-() const { return (*this) * static_cast<REAL>(-1); }
  inline CMat3 operator+() const { return (*this); }
  inline CMat3 &operator+=(const CMat3 &rhs) {
    for (unsigned int i = 0; i < 9; i++) { p_[i] += rhs.p_[i]; }
    return *this;
  }
  inline CMat3 &operator-=(const CMat3 &rhs) {
    for (unsigned int i = 0; i < 9; i++) { p_[i] -= rhs.p_[i]; }
    return *this;
  }
  inline CMat3 &operator*=(REAL d) {
    for (auto &m: p_) { m *= d; }
    return *this;
  }
  inline CMat3 &operator/=(REAL d) {
    REAL invd = (REAL) 1.0 / d;
    for (auto &m: p_) { m *= invd; }
    return *this;
  }
  template<typename INDEX>
  inline REAL operator[](INDEX i) const {
    return this->p_[i];
  }
  template<typename INDEX0, typename INDEX1>
  inline REAL &operator()(INDEX0 i, INDEX1 j) {
    return this->p_[i * 3 + j];
  }
  template<typename INDEX0, typename INDEX1>
  inline const REAL &operator()(INDEX0 i, INDEX1 j) const {
    return this->p_[i * 3 + j];
  }

  // implicit cast
  operator std::array<REAL, 9>() const {
    return {
        p_[0], p_[1], p_[2],
        p_[3], p_[4], p_[5],
        p_[6], p_[7], p_[8]};
  }

  // above: operators
  // -------------------------
  /**
   * @return
   * @details named after Eigen library (#include <Eigen/LU>)
   */
  [[nodiscard]] CMat3 Inverse() const;
  // -------------------------
  // function whose name starts with "Set" changes itself
  void SetInverse();
  void SetSymetric(const REAL sm[6]);
  void SetRandom();
  void SetRotMatrix_Quaternion(const REAL quat[]);
  void SetRotMatrix_BryantAngle(REAL rx, REAL ry, REAL rz);
  void SetIdentity(REAL scale = 1);
  void SetMat4(const REAL m[16]) {
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        this->p_[i * 3 + j] = m[i * 4 + j];
      }
    }
  }
  // ------------------------

  // quaterion (x,y,z,w)
  [[nodiscard]] std::array<REAL, 4> GetQuaternion() const;

  // -----------------------

  /**
   * named after Eigen library
   */
  void setZero();

  // ------------------------
  /**
   * named after Eigen library
   * @return
   */
  [[nodiscard]] CMat3 transpose() const {
    CMat3 m;
    m.p_[0] = p_[0];
    m.p_[1] = p_[3];
    m.p_[2] = p_[6];
    m.p_[3] = p_[1];
    m.p_[4] = p_[4];
    m.p_[5] = p_[7];
    m.p_[6] = p_[2];
    m.p_[7] = p_[5];
    m.p_[8] = p_[8];
    return m;
  }
  [[nodiscard]] bool isNaN() const {
    double s = p_[0] + p_[1] + p_[2] + p_[3] + p_[4] + p_[5] + p_[6] + p_[7] + p_[8];
    return myIsNAN_Matrix3(s) != 0;
  }
  /**
   * @return
   * @details named after Eigen library (#include <Eigen/LU>)
   */
  [[nodiscard]] REAL determinant() const {
    return
        p_[0] * p_[4] * p_[8] +
            p_[3] * p_[7] * p_[2] +
            p_[6] * p_[1] * p_[5] -
            p_[0] * p_[7] * p_[5] -
            p_[6] * p_[4] * p_[2] -
            p_[3] * p_[1] * p_[8];
  }
  /**
   * Frobenius norm. named after Eigen
   * @return
   */
  [[nodiscard]] REAL squaredNorm() const {
    REAL s = 0;
    for (auto &i: p_) { s += i * i; }
    return s;
  }
  /**
   * named after Eigen library
   * @return
   */
  [[nodiscard]] REAL trace() const {
    return p_[0] + p_[4] + p_[8];
  }
  [[nodiscard]] double SecondInvarint() const {
    const CMat3 &m2 = (*this) * (*this);
    const double tr = this->Trace();
    return 0.5 * (tr * tr - m2.Trace());
  }
  void Print() const {
    std::cout << p_[0] << " " << p_[1] << " " << p_[2] << std::endl;
    std::cout << p_[3] << " " << p_[4] << " " << p_[5] << std::endl;
    std::cout << p_[6] << " " << p_[7] << " " << p_[8] << std::endl;
  }
  void PolerDecomp(CMat3 &R, int nitr) const {
    GetRotPolarDecomp(R.p_,
                      p_, nitr);
  }
  // --------------------
  // static functions
  static CMat3 Identity(REAL scale = 1) {
    CMat3 m;
    m.SetIdentity(scale);
    return m;
  }
  static CMat3 Zero() {
    CMat3 m;
    m.setZero();
    return m;
  }
  template <typename VEC>
  static CMat3 Skew(const VEC &v) {
    return CMat3{
        0, -v[2], +v[1],
        +v[2], 0, -v[0],
        -v[1], +v[0], 0};
  }
  static CMat3 OuterProduct(const REAL *v0, const REAL *v1) {
    return CMat3<REAL>(
        v0[0] * v1[0], v0[0] * v1[1], v0[0] * v1[2],
        v0[1] * v1[0], v0[1] * v1[1], v0[1] * v1[2],
        v0[2] * v1[0], v0[2] * v1[1], v0[2] * v1[2]);
  }
  // quaternion order of (x,y,z,w)
  static CMat3 Quat(const REAL *q) {
    const REAL x2 = q[0] * q[0] * 2;
    const REAL y2 = q[1] * q[1] * 2;
    const REAL z2 = q[2] * q[2] * 2;
    const REAL xy = q[0] * q[1] * 2;
    const REAL yz = q[1] * q[2] * 2;
    const REAL zx = q[2] * q[0] * 2;
    const REAL xw = q[0] * q[3] * 2;
    const REAL yw = q[1] * q[3] * 2;
    const REAL zw = q[2] * q[3] * 2;
    CMat3<REAL> m;
    m.p_[0 * 3 + 0] = 1 - y2 - z2;
    m.p_[0 * 3 + 1] = xy - zw;
    m.p_[0 * 3 + 2] = zx + yw;
    m.p_[1 * 3 + 0] = xy + zw;
    m.p_[1 * 3 + 1] = 1 - z2 - x2;
    m.p_[1 * 3 + 2] = yz - xw;
    m.p_[2 * 3 + 0] = zx - yw;
    m.p_[2 * 3 + 1] = yz + xw;
    m.p_[2 * 3 + 2] = 1 - x2 - y2;
    return m;
  }
 public:
  REAL p_[9]; // value with row-major order
  using Scalar = REAL;
};

using CMat3d = CMat3<double>;
using CMat3f = CMat3<float>;

template<typename T>
CMat3<T> operator+(const CMat3<T> &lhs, const CMat3<T> &rhs);

template<typename T>
CMat3<T> operator-(const CMat3<T> &lhs, const CMat3<T> &rhs);

// ------------

/**
 * scalar multiplication
 */
template<
    typename T0, typename T1,
    typename std::enable_if_t<std::is_scalar_v<T0>> * = nullptr>
CMat3<T1> operator*(T0 d, const CMat3<T1> &rhs) {
  CMat3<T1> temp = rhs;
  temp *= d;
  return temp;
}

/**
 * scalar multiplication
 */
template<typename T0, typename T1,
    typename std::enable_if_t<std::is_scalar_v<T1>> * = nullptr>
// make sure T1 is scalar type
CMat3<T0> operator*(const CMat3<T0> &m, T1 d) {
  CMat3<T0> t = m;
  t *= d;
  return t;
}

template<typename T>
CMat3<T> operator*(const CMat3<T> &lhs, const CMat3<T> &rhs);

template<typename T>
class CVec3;

/**
 *
 * @tparam VEC has definition VEC::Scalar, is not CMat3 (e.g., dfm2::CVec3, Eigen::Vector3)
 */
template<typename VEC,
    typename T = typename VEC::Scalar,
    std::enable_if_t<std::is_same_v<VEC, CVec3<T>>> * = nullptr>
VEC operator*(
    const VEC &v,
    const CMat3<T> &m) {
  return {
      m[0] * v[0] + m[3] * v[1] + m[6] * v[2],
      m[1] * v[0] + m[4] * v[1] + m[7] * v[2],
      m[2] * v[0] + m[5] * v[1] + m[8] * v[2]};
}

/**
 *
 * @tparam VEC has definition VEC::Scalar, is not CMat3 (e.g., dfm2::CVec3, Eigen::Vector3)
 */
template<typename VEC,
    typename T = typename VEC::Scalar,
    std::enable_if_t<std::is_same_v<VEC, CVec3<T>>> * = nullptr>
VEC operator*(
    const CMat3<T> &m,
    const VEC &v) {
  return {
      m[0] * v[0] + m[1] * v[1] + m[2] * v[2],
      m[3] * v[0] + m[4] * v[1] + m[5] * v[2],
      m[6] * v[0] + m[7] * v[1] + m[8] * v[2]};
}

// -------

template<typename T>
CMat3<T> operator/(const CMat3<T> &m, T d);

template<typename T>
std::ostream &operator<<(std::ostream &output, const CMat3<T> &m);

template<typename T>
std::istream &operator>>(std::istream &output, CMat3<T> &m);

}  // namespace delfem2

#ifndef DFM2_STATIC_LIBRARY
#  include "delfem2/mat3.cpp"
#endif

#endif /* DFM2_MAT3_H */
