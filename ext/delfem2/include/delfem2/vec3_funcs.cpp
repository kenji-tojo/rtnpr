//
// Created by Nobuyuki Umetani on 2022/02/05.
//

#include "delfem2/vec3_funcs.h"

template<typename VEC0, typename VEC1>
void delfem2::FrameFromVectorZ(
    VEC0 &&vec_x,
    VEC0 &&vec_y,
    const VEC1 &vec_n) {
  using T = vecn_value_t<VEC1,3>;
  const T vec_s[3] = {0, 1, 0};
  Cross(vec_x, vec_s, vec_n);
  const T len = Length3(vec_x);
  if (len < 1.0e-10) {
    const T vec_t[3] = {1, 0, 0};
    Cross(vec_x, vec_t, vec_n);  // z????
    Cross(vec_y, vec_n, vec_x);  // x????
  } else {
    const T invlen = 1 / len;
    vec_x[0] *= invlen;
    vec_x[1] *= invlen;
    vec_x[2] *= invlen;
    Cross(vec_y, vec_n, vec_x);
  }
}

template<typename VEC, typename T>
VEC delfem2::RotateVec3WithAxisAngleVector(
    const VEC &vec0,
    const VEC &rot) {
  const T theta = Length3(rot);
  if (theta < 1.0e-30) {
    return vec0;
  }
  VEC e0 = rot;
  Normalize3(e0);
  VEC e2 = Cross(e0, vec0);
  if (Length3(e2) < 1.0e-30) {
    return vec0;
  }
  Normalize3(e2);
  VEC e1 = Cross(e2, e0);
  assert(fabs(Length3(e1) - 1) < 1.0e-10);
  //	assert( e2.p[0]*vec_0.p[0] + e2.p[1]*vec_0.p[1] + e2.p[2]*vec_0.p[2] < 1.0e-10 );
  const T dot00 = Dot3(vec0, e0);
  const T dot01 = Dot3(vec0, e1);
  const T cost = std::cos(theta);
  const T sint = std::sin(theta);
  return {
      dot00 * e0[0] + dot01 * cost * e1[0] + dot01 * sint * e2[0],
      dot00 * e0[1] + dot01 * cost * e1[1] + dot01 * sint * e2[1],
      dot00 * e0[2] + dot01 * cost * e1[2] + dot01 * sint * e2[2]};
}

// ----------

template<typename VEC>
std::ostream &delfem2::operator<<(
    std::ostream &output,
    const std::vector<VEC> &aV) {
  output << aV.size() << std::endl;
  for (unsigned int iv = 0; iv < aV.size(); ++iv) {
    output << "  " << iv << "-->" << aV[iv][0] << " " << aV[iv][1] << " " << aV[iv][2] << std::endl;
  }
  return output;
}

// -----------

template<typename VEC>
std::istream &delfem2::operator>>(
    std::istream &input,
    std::vector<VEC> &aV) {
  int nV;
  input >> nV;
  aV.resize(nV);
  for (int iv = 0; iv < nV; iv++) {
    input >> aV[iv][0] >> aV[iv][1] >> aV[iv][2];
  }
  return input;
}


// ==================================


#ifdef DFM2_STATIC_LIBRARY

#include "delfem2/vec3.h"

namespace delfem2 {
using f0 = float [3];
using d0 = double [3];
using f1 = float *;
using d1 = double *;
using f2 = std::array<float, 3>;
using d2 = std::array<double, 3>;
using f3 = CVec3f;
using d3 = CVec3d;
//
template float Dot3(const f0 &, const f0 &);
template float Dot3(const f1 &, const f1 &);
template float Dot3(const f2 &, const f2 &);
template float Dot3(const f3 &, const f3 &);
template double Dot3(const d0 &, const d0 &);
template double Dot3(const d1 &, const d1 &);
template double Dot3(const d2 &, const d2 &);
template double Dot3(const d3 &, const d3 &);
//
template float Distance3(const f0 &, const f0 &);
template float Distance3(const f1 &, const f1 &);
template float Distance3(const f2 &, const f2 &);
template float Distance3(const f3 &, const f3 &);
template double Distance3(const d0 &, const d0 &);
template double Distance3(const d1 &, const d1 &);
template double Distance3(const d2 &, const d2 &);
template double Distance3(const d3 &, const d3 &);
//
template float Length3(const f0 &);
template float Length3(const f1 &);
template float Length3(const f2 &);
template float Length3(const f3 &);
template double Length3(const d0 &);
template double Length3(const d1 &);
template double Length3(const d2 &);
template double Length3(const d3 &);
//
template void Normalize3(f0 &);
template void Normalize3(f1 &);
template void Normalize3(f2 &);
template void Normalize3(f3 &);
template void Normalize3(d0 &);
template void Normalize3(d1 &);
template void Normalize3(d2 &);
template void Normalize3(d3 &);
//
template f2 Cross(const f2 &, const f2 &);
template f3 Cross(const f3 &, const f3 &);
template d2 Cross(const d2 &, const d2 &);
template d3 Cross(const d3 &, const d3 &);
//
template void Cross(f0 &, const f0 &, const f0 &);
template void Cross(f1 &, const f1 &, const f1 &);
template void Cross(f2 &, const f2 &, const f2 &);
template void Cross(f3 &, const f3 &, const f3 &);
template void Cross(d0 &, const d0 &, const d0 &);
template void Cross(d1 &, const d1 &, const d1 &);
template void Cross(d2 &, const d2 &, const d2 &);
template void Cross(d3 &, const d3 &, const d3 &);
//
template float ScalarTripleProduct(const f0 &, const f0 &, const f0 &);
template float ScalarTripleProduct(const f1 &, const f1 &, const f1 &);
template float ScalarTripleProduct(const f2 &, const f2 &, const f2 &);
template float ScalarTripleProduct(const f3 &, const f3 &, const f3 &);
template double ScalarTripleProduct(const d0 &, const d0 &, const d0 &);
template double ScalarTripleProduct(const d1 &, const d1 &, const d1 &);
template double ScalarTripleProduct(const d2 &, const d2 &, const d2 &);
template double ScalarTripleProduct(const d3 &, const d3 &, const d3 &);
//
template void FrameFromVectorZ(f0 &, f0&, const f0&);
template void FrameFromVectorZ(f1 &, f1&, const f1&);
template void FrameFromVectorZ(f2 &, f2&, const f2&);
template void FrameFromVectorZ(f3 &, f3&, const f3&);
template void FrameFromVectorZ(d0 &, d0&, const d0&);
template void FrameFromVectorZ(d1 &, d1&, const d1&);
template void FrameFromVectorZ(d2 &, d2&, const d2&);
template void FrameFromVectorZ(d3 &, d3&, const d3&);
//
template f2 RotateVec3WithAxisAngleVector(const f2&, const f2&);
template f3 RotateVec3WithAxisAngleVector(const f3&, const f3&);
//
template std::ostream & operator <<(std::ostream &output, const std::vector<f0> &);
template std::ostream & operator <<(std::ostream &output, const std::vector<f1> &);
template std::ostream & operator <<(std::ostream &output, const std::vector<f2> &);
template std::ostream & operator <<(std::ostream &output, const std::vector<f3> &);
}

#endif
