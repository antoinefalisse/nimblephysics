/*
 * Copyright (c) 2011-2019, The DART development contributors
 * All rights reserved.
 *
 * The list of contributors can be found at:
 *   https://github.com/dartsim/dart/blob/master/LICENSE
 *
 * This file is provided under the following "BSD-style" License:
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 *   CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 *   INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *   MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
 *   USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 *   AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *   POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DART_MATH_MATHTYPES_HPP_
#define DART_MATH_MATHTYPES_HPP_

#include <map>
#include <vector>

#include <Eigen/Dense>
#include <unsupported/Eigen/MPRealSupport>

#include "dart/common/Deprecated.hpp"
#include "dart/common/Memory.hpp"

#include "mpreal.h"

// #define DART_USE_ARBITRARY_PRECISION

#ifdef DART_USE_ARBITRARY_PRECISION
typedef mpfr::mpreal s_t;
#else
typedef double s_t;
using std::abs;
using std::ceil;
using std::floor;
using std::isfinite;
using std::isnan;
using std::max;
using std::pow;
#endif

//------------------------------------------------------------------------------
// Types
//------------------------------------------------------------------------------
namespace Eigen {

using Vector6s = Matrix<s_t, 6, 1>;
using Matrix6s = Matrix<s_t, 6, 6>;

typedef Matrix<s_t, Dynamic, Dynamic> MatrixXs;
typedef Matrix<s_t, Dynamic, 1> VectorXs;
typedef Matrix<s_t, 2, 1> Vector2s;
typedef Matrix<s_t, 3, 1> Vector3s;
typedef Matrix<s_t, 4, 1> Vector4s;
typedef Matrix<s_t, 5, 1> Vector5s;
typedef Matrix<s_t, 6, 1> Vector6s;
typedef Matrix<s_t, 2, 2> Matrix2s;
typedef Matrix<s_t, 3, 3> Matrix3s;
typedef Matrix<s_t, 4, 4> Matrix4s;
typedef Matrix<s_t, 5, 5> Matrix5s;
typedef Matrix<s_t, 6, 6> Matrix6s;
typedef Transform<s_t, 2, Isometry> Isometry2s;
typedef Transform<s_t, 3, Isometry> Isometry3s;
typedef Quaternion<s_t> Quaternion_s;
typedef AngleAxis<s_t> AngleAxis_s;
typedef Translation<s_t, 3> Translation3s;

inline Vector6s compose(
    const Eigen::Vector3s& _angular, const Eigen::Vector3s& _linear)
{
  Vector6s composition;
  composition << _angular, _linear;
  return composition;
}

// Deprecated
using EIGEN_V_VEC3D = std::vector<Eigen::Vector3s>;

// Deprecated
using EIGEN_VV_VEC3D = std::vector<std::vector<Eigen::Vector3s>>;

#if EIGEN_VERSION_AT_LEAST(3, 2, 1) && EIGEN_VERSION_AT_MOST(3, 2, 8)

// Deprecated in favor of dart::common::aligned_vector
template <typename _Tp>
using aligned_vector
    = std::vector<_Tp, dart::common::detail::aligned_allocator_cpp11<_Tp>>;

// Deprecated in favor of dart::common::aligned_map
template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>>
using aligned_map = std::map<
    _Key,
    _Tp,
    _Compare,
    dart::common::detail::aligned_allocator_cpp11<std::pair<const _Key, _Tp>>>;

#else

// Deprecated in favor of dart::common::aligned_vector
template <typename _Tp>
using aligned_vector = std::vector<_Tp, Eigen::aligned_allocator<_Tp>>;

// Deprecated in favor of dart::common::aligned_map
template <typename _Key, typename _Tp, typename _Compare = std::less<_Key>>
using aligned_map = std::map<
    _Key,
    _Tp,
    _Compare,
    Eigen::aligned_allocator<std::pair<const _Key, _Tp>>>;

#endif

// Deprecated in favor of dart::common::make_aligned_shared
template <typename _Tp, typename... _Args>
DART_DEPRECATED(6.2)
std::shared_ptr<_Tp> make_aligned_shared(_Args&&... __args)
{
  return ::dart::common::make_aligned_shared<_Tp, _Args...>(
      std::forward<_Args>(__args)...);
}

} // namespace Eigen

namespace dart {
namespace math {

using Inertia = Eigen::Matrix6s;
using LinearJacobian = Eigen::Matrix<s_t, 3, Eigen::Dynamic>;
using AngularJacobian = Eigen::Matrix<s_t, 3, Eigen::Dynamic>;
using Jacobian = Eigen::Matrix<s_t, 6, Eigen::Dynamic>;

} // namespace math
} // namespace dart

#endif // DART_MATH_MATHTYPES_HPP_
