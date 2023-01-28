/*
 * Copyright (c) 2019 Nobuyuki Umetani
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "delfem2/geo_tri.h"

#include <cmath>
#include <stack>

#include "delfem2/vec3_funcs.h"
#include "delfem2/geo_edge.h"
#include "delfem2/geo_plane.h"

#include "delfem2/mat3.h"
#include "delfem2/mat3_funcs.h"

#ifndef M_PI
#  define M_PI 3.14159265358979323846
#endif

namespace delfem2::geo_tri {

template<typename T>
T Volume_Tet3(
    const T v1[3],
    const T v2[3],
    const T v3[3],
    const T v4[3]) {
  const T a0 = +(v2[0] - v1[0]) * ((v3[1] - v1[1]) * (v4[2] - v1[2]) - (v4[1] - v1[1]) * (v3[2] - v1[2]));
  const T a1 = -(v2[1] - v1[1]) * ((v3[0] - v1[0]) * (v4[2] - v1[2]) - (v4[0] - v1[0]) * (v3[2] - v1[2]));
  const T a2 = +(v2[2] - v1[2]) * ((v3[0] - v1[0]) * (v4[1] - v1[1]) - (v4[0] - v1[0]) * (v3[1] - v1[1]));
  return (a0 + a1 + a2) * 0.16666666666666666666666666666667;
}

template<typename T>
bool isPointSameSide(
    const CVec3<T> &p0,
    const CVec3<T> &p1,
    const CVec3<T> &line_p0,
    const CVec3<T> &line_p1) {
  CVec3<T> crossProd1 = Cross(line_p1 - line_p0, p0 - line_p0);
  CVec3<T> crossProd2 = Cross(line_p1 - line_p0, p1 - line_p0);
  if (crossProd1.dot(crossProd2) >= 0) {
    return true;
  } else {
    return false;
  }
}

}  // delfem2::geo_tri

// -------------------------------

DFM2_INLINE void delfem2::Nearest_Triangle3_Point3(
    double nearest_position[3],
    double &r0,
    double &r1,
    const double ps[3], // origin point
    const double q0[3],
    const double q1[3],
    const double q2[3]) {
  namespace lcl = delfem2::geo_tri;
  double area, n012[3];
  UnitNormalAreaTri3(n012, area, q0, q1, q2);
  const double pe[3] = {ps[0] + n012[0], ps[1] + n012[1], ps[2] + n012[2]};
  const double v012 = lcl::Volume_Tet3(ps, q0, q1, q2);
  if (fabs(v012) > 1.0e-10) {
    const double sign = (v012 > 0) ? +1 : -1;
    const double v0 = lcl::Volume_Tet3(ps, q1, q2, pe) * sign;
    const double v1 = lcl::Volume_Tet3(ps, q2, q0, pe) * sign;
    const double v2 = lcl::Volume_Tet3(ps, q0, q1, pe) * sign;
    assert(fabs(v0 + v1 + v2) > 1.0e-10);
    double inv_v012 = 1.0 / (v0 + v1 + v2);
    r0 = v0 * inv_v012;
    r1 = v1 * inv_v012;
    const double r2 = (1.0 - r0 - r1);
    const double tol = 1.0e-4;
    if (r0 > -tol && r1 > -tol && r2 > -tol) {
      nearest_position[0] = q0[0] * r0 + q1[0] * r1 + q2[0] * r2;
      nearest_position[1] = q0[1] * r0 + q1[1] * r1 + q2[1] * r2;
      nearest_position[2] = q0[2] * r0 + q1[2] * r1 + q2[2] * r2;
      return;
    }
  }
  double r12[3];
  Nearest_Edge3_Point3(r12, ps, q1, q2);
  double r20[3];
  Nearest_Edge3_Point3(r20, ps, q2, q0);
  double r01[3];
  Nearest_Edge3_Point3(r01, ps, q0, q1);
  const double d12 = Distance3(r12, ps);
  const double d20 = Distance3(r20, ps);
  const double d01 = Distance3(r01, ps);
  if (d12 < d20) {
    if (d12 < d01) { // 12 is the smallest
      nearest_position[0] = r12[0];
      nearest_position[1] = r12[1];
      nearest_position[2] = r12[2];
      r0 = 0;
      r1 = Distance3(nearest_position, q2) / Distance3(q1, q2);
      return;
    }
  } else {
    if (d20 < d01) { // d20 is the smallest
      nearest_position[0] = r20[0];
      nearest_position[1] = r20[1];
      nearest_position[2] = r20[2];
      r0 = Distance3(nearest_position, q2) / Distance3(q0, q2);
      r1 = 0;
      return;
    }
  }
  nearest_position[0] = r01[0];
  nearest_position[1] = r01[1];
  nearest_position[2] = r01[2];
  r0 = Distance3(nearest_position, q1) / Distance3(q0, q1);
  r1 = 1 - r0;
}

// ---------------------------------------------------------------------------

template<typename T>
delfem2::CVec3<T> delfem2::Nearest_Origin3_Tri3(
    T &r0,
    T &r1,
    const CVec3<T> &q0,
    const CVec3<T> &q1,
    const CVec3<T> &q2) {

  if (((q1 - q0).cross(q2 - q0)).norm() > 1.0e-10) {
    CVec3<T> p012 = Nearest_Origin3_PlaneTri3(r0, r1, q0, q1, q2);
    if (r0 > 0 && r1 > 0 && (1 - r0 - r1) > 0) { return p012; }
  }
  CVec3<T> p_min = q0;
  T d_min = q0.norm();
  r0 = 1;
  r1 = 0;
  {
    T s2;
    CVec3<T> p12 = Nearest_Origin_Edge(s2, q1, q2);
    const T d12 = p12.norm();
    if (d12 < d_min) {
      d_min = d12;
      p_min = p12;
      r1 = 1 - s2;
      r0 = 0;
    }
  }
  {
    T s0;
    CVec3<T> p20 = Nearest_Origin_Edge(s0, q2, q0);
    const T d20 = p20.norm();
    if (d20 < d_min) {
      d_min = d20;
      p_min = p20;
      r1 = 0;
      r0 = s0;
    }
  }
  {
    T s1;
    CVec3<T> p01 = Nearest_Origin_Edge(s1, q0, q1);
    const T d01 = p01.norm();
    if (d01 < d_min) {
      d_min = d01;
      p_min = p01;
      r0 = 1 - s1;
      r1 = s1;
    }
  }
  return p_min;
}

// ===================


template<typename REAL>
bool delfem2::IntersectRay_Tri3(
    REAL &r0,
    REAL &r1,
    const CVec3<REAL> &org,
    const CVec3<REAL> &dir,
    const CVec3<REAL> &p0,
    const CVec3<REAL> &p1,
    const CVec3<REAL> &p2,
    REAL eps) {
  namespace lcl = delfem2::geo_tri;
  const REAL v0 = lcl::Volume_Tet3(p1.data(), p2.data(), org.data(), (org + dir).data());
  const REAL v1 = lcl::Volume_Tet3(p2.data(), p0.data(), org.data(), (org + dir).data());
  const REAL v2 = lcl::Volume_Tet3(p0.data(), p1.data(), org.data(), (org + dir).data());
  const REAL vt = v0 + v1 + v2;
  r0 = v0 / vt;
  r1 = v1 / vt;
  const REAL r2 = v2 / vt;
  return (r0 >= -eps && r1 >= -eps && r2 >= -eps);
}

// =====================

template<typename T>
bool delfem2::isIntersectTriPair(
    CVec3<T> &P0,
    CVec3<T> &P1,
    int itri,
    int jtri,
    const std::vector<unsigned int> &aTri,
    const std::vector<double> &aXYZ) {
  const unsigned int i0 = aTri[itri * 3 + 0];
  const unsigned int i1 = aTri[itri * 3 + 1];
  const unsigned int i2 = aTri[itri * 3 + 2];
  const unsigned int j0 = aTri[jtri * 3 + 0];
  const unsigned int j1 = aTri[jtri * 3 + 1];
  const unsigned int j2 = aTri[jtri * 3 + 2];
  if (i0 == j0 || i0 == j1 || i0 == j2) return false;
  if (i1 == j0 || i1 == j1 || i1 == j2) return false;
  if (i2 == j0 || i2 == j1 || i2 == j2) return false;
  const CVec3<T> p0(aXYZ[i0 * 3 + 0], aXYZ[i0 * 3 + 1], aXYZ[i0 * 3 + 2]);
  const CVec3<T> p1(aXYZ[i1 * 3 + 0], aXYZ[i1 * 3 + 1], aXYZ[i1 * 3 + 2]);
  const CVec3<T> p2(aXYZ[i2 * 3 + 0], aXYZ[i2 * 3 + 1], aXYZ[i2 * 3 + 2]);
  const CVec3<T> q0(aXYZ[j0 * 3 + 0], aXYZ[j0 * 3 + 1], aXYZ[j0 * 3 + 2]);
  const CVec3<T> q1(aXYZ[j1 * 3 + 0], aXYZ[j1 * 3 + 1], aXYZ[j1 * 3 + 2]);
  const CVec3<T> q2(aXYZ[j2 * 3 + 0], aXYZ[j2 * 3 + 1], aXYZ[j2 * 3 + 2]);
  const CVec3<T> np = Normal_Tri3(p0, p1, p2);
  const CVec3<T> nq = Normal_Tri3(q0, q1, q2);
  double dp0 = (p0 - q0).dot(nq);
  double dp1 = (p1 - q0).dot(nq);
  double dp2 = (p2 - q0).dot(nq);
  double dq0 = (q0 - p0).dot(np);
  double dq1 = (q1 - p0).dot(np);
  double dq2 = (q2 - p0).dot(np);
  if (((dp0 > 0) == (dp1 > 0)) && ((dp1 > 0) == (dp2 > 0))) return false;
  if (((dq0 > 0) == (dq1 > 0)) && ((dq1 > 0) == (dq2 > 0))) return false;
  const CVec3<T> p01 = (1.0 / (dp0 - dp1)) * (dp0 * p1 - dp1 * p0);
  const CVec3<T> p12 = (1.0 / (dp1 - dp2)) * (dp1 * p2 - dp2 * p1);
  const CVec3<T> p20 = (1.0 / (dp2 - dp0)) * (dp2 * p0 - dp0 * p2);
  const CVec3<T> q01 = (1.0 / (dq0 - dq1)) * (dq0 * q1 - dq1 * q0);
  const CVec3<T> q12 = (1.0 / (dq1 - dq2)) * (dq1 * q2 - dq2 * q1);
  const CVec3<T> q20 = (1.0 / (dq2 - dq0)) * (dq2 * q0 - dq0 * q2);
  const CVec3<T> vz = np.cross(nq);
  CVec3<T> ps, pe;
  if (dp0 * dp1 > 0) {
    ps = p20;
    pe = p12;
  } else if (dp1 * dp2 > 0) {
    ps = p01;
    pe = p20;
  } else {
    ps = p12;
    pe = p01;
  }
  if (ps.dot(vz) > pe.dot(vz)) {
    CVec3<T> pt = ps;
    ps = pe;
    pe = pt;
  }
  double zps = ps.dot(vz);
  double zpe = pe.dot(vz);
  assert(zps <= zpe);
  //
  CVec3<T> qs, qe;
  if (dq0 * dq1 > 0) {
    qs = q20;
    qe = q12;
  } else if (dq1 * dq2 > 0) {
    qs = q01;
    qe = q20;
  } else {
    qs = q12;
    qe = q01;
  }
  if (qs.dot(vz) > qe.dot(vz)) {
    CVec3<T> qt = qs;
    qs = qe;
    qe = qt;
  }
  double zqs = qs.dot(vz);
  double zqe = qe.dot(vz);
  assert(zqs <= zqe);
  //
  if (zps > zqe || zqs > zpe) return false;
  CVec3<T> P[4];
  int icnt = 0;
  if (zps > zqs && zps < zqe) {
    P[icnt] = ps;
    icnt++;
  }
  if (zpe > zqs && zpe < zqe) {
    P[icnt] = pe;
    icnt++;
  }
  if (zqs > zps && zqs < zpe) {
    P[icnt] = qs;
    icnt++;
  }
  if (zqe > zps && zqe < zpe) {
    P[icnt] = qe;
    icnt++;
  }
  if (icnt != 2) return false;
  P0 = P[0];
  P1 = P[1];
  return true;
}
#ifdef DFM2_STATIC_LIBRARY
template bool delfem2::isIntersectTriPair(
  CVec3d &P0, CVec3d &P1,
  int itri, int jtri,
  const std::vector<unsigned int> &aTri,
  const std::vector<double> &aXYZ);
#endif

// =================



template<typename T>
delfem2::CVec3<T> delfem2::ProjectPointOnTriangle(
    const CVec3<T> &p0,
    const CVec3<T> &tri_p1,
    const CVec3<T> &tri_p2,
    const CVec3<T> &tri_p3) {
  CVec3<T> normal = Cross(tri_p2 - tri_p1, tri_p3 - tri_p1);
  double cosAlpha = Dot(p0 - tri_p1, normal) / (Length(p0 - tri_p1) * Length(normal));
  double lenP0ProjectedP0 = Length(tri_p1 - p0) * cosAlpha;
  CVec3<T> p0ProjectedP0 = -1 * lenP0ProjectedP0 * normal / Length(normal);
  return p0 + p0ProjectedP0;
}

// ----------------------

template<typename T>
bool delfem2::isRayIntersectingTriangle(
    const CVec3<T> &line0, const CVec3<T> &line1,
    const CVec3<T> &tri0, const CVec3<T> &tri1, const CVec3<T> &tri2,
    CVec3<T> &intersectionPoint) {
  CVec3<T> normal = Cross(tri1 - tri0, tri2 - tri0);

  // The ray is parallel to the triangle plane
  if (normal.dot(line1 - line0) == 0) {
    return false;
  }

  double r = normal.dot(tri0 - line0) / normal.dot(line1 - line0);

  // The ray does not intersect the triangle plane
  if (r < 0) {
    return false;
  }

  // Find the intersection point
  intersectionPoint = line0 + r * (line1 - line0);

  if (!isPointInsideTriangle(intersectionPoint,
                             tri0, tri1, tri2)) {
    return false;
  }

  return true;
}

// ----------------------------------------

template<typename T>
bool delfem2::isPointInsideTriangle(
    const CVec3<T> &p0,
    const CVec3<T> &tri_p1,
    const CVec3<T> &tri_p2,
    const CVec3<T> &tri_p3) {
  namespace lcl = delfem2::geo_tri;
  if (lcl::isPointSameSide(p0, tri_p1, tri_p2, tri_p3)
      && lcl::isPointSameSide(p0, tri_p2, tri_p1, tri_p3)
      && lcl::isPointSameSide(p0, tri_p3, tri_p1, tri_p2)) {
    return true;
  } else {
    return false;
  }
}

// distance VF
template<typename T>
double delfem2::DistanceFaceVertex(
    const CVec3<T> &p0,
    const CVec3<T> &p1,
    const CVec3<T> &p2,
    const CVec3<T> &p3,
    double &w0,
    double &w1) {
  CVec3<T> v20 = p0 - p2;
  CVec3<T> v21 = p1 - p2;
  double t0 = v20.dot(v20);
  double t1 = v21.dot(v21);
  double t2 = v20.dot(v21);
  double t3 = v20.dot(p3 - p2);
  double t4 = v21.dot(p3 - p2);
  double det = t0 * t1 - t2 * t2;
  double invdet = 1.0 / det;
  w0 = (+t1 * t3 - t2 * t4) * invdet;
  w1 = (-t2 * t3 + t0 * t4) * invdet;
  const double w2 = 1 - w0 - w1;
  CVec3<T> pw = w0 * p0 + w1 * p1 + w2 * p2;
  return (pw - p3).norm();
}

// -------------------

template<typename VEC, typename T>
T delfem2::SolidAngleTri(
    const VEC &v1,
    const VEC &v2,
    const VEC &v3) {
  const T l1 = v1.norm();
  const T l2 = v2.norm();
  const T l3 = v3.norm();
  const T den = (v1.cross(v2)).dot(v3);
  const T num = l1 * l2 * l3 + (v1.dot(v2)) * l3 + (v2.dot(v3)) * l1 + (v3.dot(v1)) * l2;
  const T tho = den / num;
  T v = std::atan(tho);
  if (v < 0) { v += 2 * M_PI; }
  v *= 2;
  return v;
}

template<typename VEC0, typename VEC1, typename T>
void delfem2::UnitNormalAreaTri3(
    VEC0 &&n,
    T &a,
    const VEC1 &v1,
    const VEC1 &v2,
    const VEC1 &v3) {
  Normal_Tri3(
      n,
      v1, v2, v3);
  a = Length3(n) / 2;
  const T invlen = 1 / (a * 2);
  n[0] *= invlen;
  n[1] *= invlen;
  n[2] *= invlen;
}

template<typename VEC, typename T>
std::array<T,3> delfem2::Normal_Tri3(
    const VEC &v1,
    const VEC &v2,
    const VEC &v3) {
  return {
      (v2[1] - v1[1]) * (v3[2] - v1[2]) - (v2[2] - v1[2]) * (v3[1] - v1[1]),
      (v2[2] - v1[2]) * (v3[0] - v1[0]) - (v2[0] - v1[0]) * (v3[2] - v1[2]),
      (v2[0] - v1[0]) * (v3[1] - v1[1]) - (v2[1] - v1[1]) * (v3[0] - v1[0])};
}

template<typename VEC>
VEC delfem2::UnitNormal_Tri3(
    const VEC &v1,
    const VEC &v2,
    const VEC &v3) {
  VEC vnorm = Normal_Tri3(v1, v2, v3);
  Normalize3(vnorm);
  return vnorm;
}

// moment of inertia around origin triangle (d0,d1,d2) the area_density=1
// see http://www.dcs.warwick.ac.uk/~rahil/files/RigidBodySimulation.pdf
template<typename VEC, typename T>
std::array<T, 9> delfem2::Mat3_IrotTri(
    const VEC &d0,
    const VEC &d1,
    const VEC &d2) {
  const VEC dv = d0 + d1 + d2;
  const CMat3<T> m0 = Mat3_OuterProduct(d0, d0);
  const CMat3<T> m1 = Mat3_OuterProduct(d1, d1);
  const CMat3<T> m2 = Mat3_OuterProduct(d2, d2);
  const CMat3<T> mv = Mat3_OuterProduct(dv, dv);
  const CMat3<T> I0 = m0 + m1 + m2 + mv;
  T tr0 = I0.trace();
  CMat3<T> I = tr0 * CMat3<T>::Identity() - I0;
  T darea = ((d1 - d0).cross(d2 - d0)).norm();
  I *= darea / 24.0;
  return I;
}

// moment of inertia triangle pyramid with vtx (origin,d0,d1,d2) volume_density = 1
// see http://www.dcs.warwick.ac.uk/~rahil/files/RigidBodySimulation.pdf
template<typename VEC, typename T>
std::array<T, 9> delfem2::Mat3_IrotTriSolid(
    const VEC &d0,
    const VEC &d1,
    const VEC &d2) {
  VEC dv = d0 + d1 + d2;
  const CMat3<T> m0 = Mat3_OuterProduct(d0, d0);
  const CMat3<T> m1 = Mat3_OuterProduct(d1, d1);
  const CMat3<T> m2 = Mat3_OuterProduct(d2, d2);
  const CMat3<T> mv = Mat3_OuterProduct(dv, dv);
  const CMat3<T> I0 = m0 + m1 + m2 + mv;
  T tr0 = I0.trace();
  CMat3<T> I = tr0 * CMat3<T>::Identity() - I0;
  T darea = d0.dot(d1.cross(d2));
  I *= darea / 120.0;
  return I;
}

// ==============================================

#ifdef DFM2_STATIC_LIBRARY

#include "delfem2/vec3.h"

namespace delfem2 {
  using f0 = float [3];
  using d0 = double [3];
  using f1 = float*;
  using d1 = double*;
  using f2 = std::array<float,3>;
  using d2 = std::array<double,3>;
  using f3 = CVec3f;
  using d3 = CVec3d;
  //
  template float Area_Tri3(const f1&, const f1&, const f1&);
  template float Area_Tri3(const f2&, const f2&, const f2&);
  template float Area_Tri3(const f3&, const f3&, const f3&);
  template double Area_Tri3(const d1&, const d1&, const d1&);
  template double Area_Tri3(const d2&, const d2&, const d2&);
  template double Area_Tri3(const d3&, const d3&, const d3&);
  //
  template f2 Normal_Tri3(const f0 &, const f0 &, const f0 &);
  template f2 Normal_Tri3(const f1 &, const f1 &, const f1 &);
  template f2 Normal_Tri3(const f2 &, const f2 &, const f2 &);
  template f2 Normal_Tri3(const f3 &, const f3 &, const f3 &);
  //
  template f2 UnitNormal_Tri3(const f2 &, const f2 &, const f2 &);
  template f3 UnitNormal_Tri3(const f3 &, const f3 &, const f3 &);
  //
  template void UnitNormalAreaTri3(f1&, float&, const f1&, const f1&, const f1&);
  template void UnitNormalAreaTri3(f2&, float&, const f2&, const f2&, const f2&);
  template void UnitNormalAreaTri3(f3&, float&, const f3&, const f3&, const f3&);
  template void UnitNormalAreaTri3(d0&, double&, const d0&, const d0&, const d0&);
  template void UnitNormalAreaTri3(d1&, double&, const d1&, const d1&, const d1&);
  template void UnitNormalAreaTri3(d2&, double&, const d2&, const d2&, const d2&);
  template void UnitNormalAreaTri3(d3&, double&, const d3&, const d3&, const d3&);
  //
  template float SolidAngleTri(const f3&, const f3&, const f3&);
  template double SolidAngleTri(const d3&, const d3&, const d3&);
  //
  template bool IntersectRay_Tri3(
      double &, double &, const d3 &, const d3 &,
      const d3 &, const d3 &, const d3 &, double);
  template bool IntersectRay_Tri3(
      float &, float &, const f3 &, const f3 &,
      const f3 &, const f3 &, const f3 &, float);
  template bool isRayIntersectingTriangle(
      const d3 &, const d3 &,
      const d3 &, const d3 &, const d3 &,
      d3 &);
  template double DistanceFaceVertex(
      const d3 &, const d3 &,
      const d3 &, const d3 &,
      double &, double &);
  template f3 Nearest_Origin3_Tri3(float &, float &,const f3 &, const f3 &, const f3 &);
  template d3 Nearest_Origin3_Tri3(double &, double &,const d3 &, const d3 &, const d3 &);
  //
  template std::array<float, 9> Mat3_IrotTri(const f3 &,const f3 &,const f3 &);
  template std::array<double, 9> Mat3_IrotTri(const d3 &,const d3 &,const d3 &);
  //
  template std::array<float,9> Mat3_IrotTriSolid(const f3 &,const f3 &,const f3 &);
  template std::array<double,9> Mat3_IrotTriSolid(const d3 &,const d3 &,const d3 &);
}
#endif
