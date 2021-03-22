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

#include "dart/dynamics/FreeJoint.hpp"

#include <string>

#include "dart/math/Helpers.hpp"
#include "dart/math/Geometry.hpp"
#include "dart/dynamics/DegreeOfFreedom.hpp"

namespace dart {
namespace dynamics {

//==============================================================================
FreeJoint::Properties::Properties(const Base::Properties& properties)
  : Base::Properties(properties)
{
  // Do nothing
}

//==============================================================================
FreeJoint::~FreeJoint()
{
  // Do nothing
}

//==============================================================================
FreeJoint::Properties FreeJoint::getFreeJointProperties() const
{
  return getGenericJointProperties();
}

//==============================================================================
Eigen::Vector6d FreeJoint::convertToPositions(const Eigen::Isometry3d& _tf)
{
  Eigen::Vector6d x;
  x.head<3>() = math::logMap(_tf.linear());
  x.tail<3>() = _tf.translation();
  return x;
}

//==============================================================================
Eigen::Isometry3d FreeJoint::convertToTransform(
    const Eigen::Vector6d& _positions)
{
  Eigen::Isometry3d tf(Eigen::Isometry3d::Identity());
  tf.linear() = math::expMapRot(_positions.head<3>());
  tf.translation() = _positions.tail<3>();
  return tf;
}

//==============================================================================
void FreeJoint::setTransform(Joint* joint,
                             const Eigen::Isometry3d& tf,
                             const Frame* withRespectTo)
{
  return setTransformOf(joint, tf, withRespectTo);
}

//==============================================================================
void FreeJoint::setTransformOf(
    Joint* joint, const Eigen::Isometry3d& tf, const Frame* withRespectTo)
{
  if (nullptr == joint)
    return;

  FreeJoint* freeJoint = dynamic_cast<FreeJoint*>(joint);

  if (nullptr == freeJoint)
  {
    dtwarn << "[FreeJoint::setTransform] Invalid joint type. Setting transform "
           << "is only allowed to FreeJoint. The joint type of given joint ["
           << joint->getName() << "] is [" << joint->getType() << "].\n";
    return;
  }

  freeJoint->setTransform(tf, withRespectTo);
}

//==============================================================================
void FreeJoint::setTransform(BodyNode* bodyNode,
                             const Eigen::Isometry3d& tf,
                             const Frame* withRespectTo)
{
  setTransformOf(bodyNode, tf, withRespectTo);
}

//==============================================================================
void FreeJoint::setTransformOf(
    BodyNode* bodyNode, const Eigen::Isometry3d& tf, const Frame* withRespectTo)
{
  if (nullptr == bodyNode)
    return;

  setTransformOf(bodyNode->getParentJoint(), tf, withRespectTo);
}

//==============================================================================
void FreeJoint::setTransform(Skeleton* skeleton,
                             const Eigen::Isometry3d& tf,
                             const Frame* withRespectTo,
                             bool applyToAllRootBodies)
{
  setTransformOf(skeleton, tf, withRespectTo, applyToAllRootBodies);
}

//==============================================================================
void FreeJoint::setTransformOf(
    Skeleton* skeleton,
    const Eigen::Isometry3d& tf,
    const Frame* withRespectTo,
    bool applyToAllRootBodies)
{
  if (nullptr == skeleton)
    return;

  const std::size_t numTrees = skeleton->getNumTrees();

  if (0 == numTrees)
    return;

  if (!applyToAllRootBodies)
  {
    setTransformOf(skeleton->getRootBodyNode(), tf, withRespectTo);
    return;
  }

  for (std::size_t i = 0; i < numTrees; ++i)
    setTransformOf(skeleton->getRootBodyNode(i), tf, withRespectTo);
}

//==============================================================================
void FreeJoint::setSpatialMotion(const Eigen::Isometry3d* newTransform,
                                 const Frame* withRespectTo,
                                 const Eigen::Vector6d* newSpatialVelocity,
                                 const Frame* velRelativeTo,
                                 const Frame* velInCoordinatesOf,
                                 const Eigen::Vector6d* newSpatialAcceleration,
                                 const Frame* accRelativeTo,
                                 const Frame* accInCoordinatesOf)
{
  if (newTransform)
    setTransform(*newTransform, withRespectTo);

  if (newSpatialVelocity)
    setSpatialVelocity(*newSpatialVelocity, velRelativeTo, velInCoordinatesOf);

  if (newSpatialAcceleration)
  {
    setSpatialAcceleration(*newSpatialAcceleration,
                           accRelativeTo,
                           accInCoordinatesOf);
  }
}

//==============================================================================
void FreeJoint::setRelativeTransform(const Eigen::Isometry3d& newTransform)
{
  setPositionsStatic(convertToPositions(
    Joint::mAspectProperties.mT_ParentBodyToJoint.inverse() *
    newTransform *
    Joint::mAspectProperties.mT_ChildBodyToJoint));
}

//==============================================================================
void FreeJoint::setTransform(const Eigen::Isometry3d& newTransform,
                             const Frame* withRespectTo)
{
  assert(nullptr != withRespectTo);

  setRelativeTransform(
        withRespectTo->getTransform(getChildBodyNode()->getParentFrame())
        * newTransform);
}

//==============================================================================
void FreeJoint::setRelativeSpatialVelocity(
    const Eigen::Vector6d& newSpatialVelocity)
{
  setVelocitiesStatic(getRelativeJacobianStatic().inverse() * newSpatialVelocity);
}

//==============================================================================
void FreeJoint::setRelativeSpatialVelocity(
    const Eigen::Vector6d& newSpatialVelocity,
    const Frame* inCoordinatesOf)
{
  assert(nullptr != inCoordinatesOf);

  if (getChildBodyNode() == inCoordinatesOf)
  {
    setRelativeSpatialVelocity(newSpatialVelocity);
  }
  else
  {
    setRelativeSpatialVelocity(
          math::AdR(inCoordinatesOf->getTransform(getChildBodyNode()),
                    newSpatialVelocity));
  }
}

//==============================================================================
void FreeJoint::setSpatialVelocity(const Eigen::Vector6d& newSpatialVelocity,
                                   const Frame* relativeTo,
                                   const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  if (getChildBodyNode() == relativeTo)
  {
    dtwarn << "[FreeJoint::setSpatialVelocity] Invalid reference frame "
              "for newSpatialVelocity. It shouldn't be the child BodyNode.\n";
    return;
  }

  // Change the reference frame of "newSpatialVelocity" to the child body node
  // frame.
  Eigen::Vector6d targetRelSpatialVel = newSpatialVelocity;
  if (getChildBodyNode() != inCoordinatesOf)
  {
    targetRelSpatialVel
        = math::AdR(inCoordinatesOf->getTransform(getChildBodyNode()),
                    newSpatialVelocity);
  }

  // Compute the target relative spatial velocity from the parent body node to
  // the child body node.
  if (getChildBodyNode()->getParentFrame() != relativeTo)
  {
    if (relativeTo->isWorld())
    {
      const Eigen::Vector6d parentVelocity = math::AdInvT(
            getRelativeTransform(),
            getChildBodyNode()->getParentFrame()->getSpatialVelocity());

      targetRelSpatialVel -= parentVelocity;
    }
    else
    {
      const Eigen::Vector6d parentVelocity = math::AdInvT(
            getRelativeTransform(),
            getChildBodyNode()->getParentFrame()->getSpatialVelocity());
      const Eigen::Vector6d arbitraryVelocity = math::AdT(
            relativeTo->getTransform(getChildBodyNode()),
            relativeTo->getSpatialVelocity());

      targetRelSpatialVel += -parentVelocity + arbitraryVelocity;
    }
  }

  setRelativeSpatialVelocity(targetRelSpatialVel);
}

//==============================================================================
void FreeJoint::setLinearVelocity(const Eigen::Vector3d& newLinearVelocity,
                                  const Frame* relativeTo,
                                  const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  Eigen::Vector6d targetSpatialVelocity;

  if (Frame::World() == relativeTo)
  {
    targetSpatialVelocity.head<3>()
        = getChildBodyNode()->getSpatialVelocity().head<3>();
  }
  else
  {
    targetSpatialVelocity.head<3>()
        = getChildBodyNode()->getSpatialVelocity(
            relativeTo, getChildBodyNode()).head<3>();
  }

  targetSpatialVelocity.tail<3>()
      = getChildBodyNode()->getWorldTransform().linear().transpose()
        * inCoordinatesOf->getWorldTransform().linear()
        * newLinearVelocity;
  // Above code is equivalent to:
  // targetSpatialVelocity.tail<3>()
  //     = getChildBodyNode()->getTransform(
  //         inCoordinatesOf).linear().transpose()
  //       * newLinearVelocity;
  // but faster.

  setSpatialVelocity(targetSpatialVelocity, relativeTo, getChildBodyNode());
}

//==============================================================================
void FreeJoint::setAngularVelocity(const Eigen::Vector3d& newAngularVelocity,
                                   const Frame* relativeTo,
                                   const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  Eigen::Vector6d targetSpatialVelocity;

  targetSpatialVelocity.head<3>()
      = getChildBodyNode()->getWorldTransform().linear().transpose()
        * inCoordinatesOf->getWorldTransform().linear()
        * newAngularVelocity;
  // Above code is equivalent to:
  // targetSpatialVelocity.head<3>()
  //     = getChildBodyNode()->getTransform(
  //         inCoordinatesOf).linear().transpose()
  //       * newAngularVelocity;
  // but faster.

  if (Frame::World() == relativeTo)
  {
    targetSpatialVelocity.tail<3>()
        = getChildBodyNode()->getSpatialVelocity().tail<3>();
  }
  else
  {
    targetSpatialVelocity.tail<3>()
        = getChildBodyNode()->getSpatialVelocity(
            relativeTo, getChildBodyNode()).tail<3>();
  }

  setSpatialVelocity(targetSpatialVelocity, relativeTo, getChildBodyNode());
}

//==============================================================================
void FreeJoint::setRelativeSpatialAcceleration(
    const Eigen::Vector6d& newSpatialAcceleration)
{
  const Eigen::Matrix6d& J = getRelativeJacobianStatic();
  const Eigen::Matrix6d& dJ = getRelativeJacobianTimeDerivStatic();

  setAccelerationsStatic(
    J.inverse() * (newSpatialAcceleration - dJ * getVelocitiesStatic()));
}

//==============================================================================
void FreeJoint::setRelativeSpatialAcceleration(
    const Eigen::Vector6d& newSpatialAcceleration,
    const Frame* inCoordinatesOf)
{
  assert(nullptr != inCoordinatesOf);

  if (getChildBodyNode() == inCoordinatesOf)
  {
    setRelativeSpatialAcceleration(newSpatialAcceleration);
  }
  else
  {
    setRelativeSpatialAcceleration(
          math::AdR(inCoordinatesOf->getTransform(getChildBodyNode()),
                    newSpatialAcceleration));
  }
}

//==============================================================================
void FreeJoint::setSpatialAcceleration(
    const Eigen::Vector6d& newSpatialAcceleration,
    const Frame* relativeTo,
    const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  if (getChildBodyNode() == relativeTo)
  {
    dtwarn << "[FreeJoint::setSpatialAcceleration] Invalid reference "
           << "frame for newSpatialAcceleration. It shouldn't be the child "
           << "BodyNode.\n";
    return;
  }

  // Change the reference frame of "newSpatialAcceleration" to the child body
  // node frame.
  Eigen::Vector6d targetRelSpatialAcc = newSpatialAcceleration;
  if (getChildBodyNode() != inCoordinatesOf)
  {
    targetRelSpatialAcc
        = math::AdR(inCoordinatesOf->getTransform(getChildBodyNode()),
                    newSpatialAcceleration);
  }

  // Compute the target relative spatial acceleration from the parent body node
  // to the child body node.
  if (getChildBodyNode()->getParentFrame() != relativeTo)
  {
    if (relativeTo->isWorld())
    {
      const Eigen::Vector6d parentAcceleration
          = math::AdInvT(
            getRelativeTransform(),
            getChildBodyNode()->getParentFrame()->getSpatialAcceleration())
            + math::ad(getChildBodyNode()->getSpatialVelocity(),
                       getRelativeJacobianStatic() * getVelocitiesStatic());

      targetRelSpatialAcc -= parentAcceleration;
    }
    else
    {
      const Eigen::Vector6d parentAcceleration
          = math::AdInvT(
            getRelativeTransform(),
            getChildBodyNode()->getParentFrame()->getSpatialAcceleration())
            + math::ad(getChildBodyNode()->getSpatialVelocity(),
                       getRelativeJacobianStatic() * getVelocitiesStatic());
      const Eigen::Vector6d arbitraryAcceleration =
          math::AdT(relativeTo->getTransform(getChildBodyNode()),
                    relativeTo->getSpatialAcceleration())
          - math::ad(getChildBodyNode()->getSpatialVelocity(),
                     math::AdT(relativeTo->getTransform(getChildBodyNode()),
                               relativeTo->getSpatialVelocity()));

      targetRelSpatialAcc += -parentAcceleration + arbitraryAcceleration;
    }
  }

  setRelativeSpatialAcceleration(targetRelSpatialAcc);
}

//==============================================================================
void FreeJoint::setLinearAcceleration(
    const Eigen::Vector3d& newLinearAcceleration,
    const Frame* relativeTo,
    const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  Eigen::Vector6d targetSpatialAcceleration;

  if (Frame::World() == relativeTo)
  {
    targetSpatialAcceleration.head<3>()
        = getChildBodyNode()->getSpatialAcceleration().head<3>();
  }
  else
  {
    targetSpatialAcceleration.head<3>()
        = getChildBodyNode()->getSpatialAcceleration(
            relativeTo, getChildBodyNode()).head<3>();
  }

  const Eigen::Vector6d& V
      = getChildBodyNode()->getSpatialVelocity(relativeTo, inCoordinatesOf);
  targetSpatialAcceleration.tail<3>()
      = getChildBodyNode()->getWorldTransform().linear().transpose()
        * inCoordinatesOf->getWorldTransform().linear()
        * (newLinearAcceleration - V.head<3>().cross(V.tail<3>()));
  // Above code is equivalent to:
  // targetSpatialAcceleration.tail<3>()
  //     = getChildBodyNode()->getTransform(
  //         inCoordinatesOf).linear().transpose()
  //       * (newLinearAcceleration - V.head<3>().cross(V.tail<3>()));
  // but faster.

  setSpatialAcceleration(
        targetSpatialAcceleration, relativeTo, getChildBodyNode());
}

//==============================================================================
void FreeJoint::setAngularAcceleration(
    const Eigen::Vector3d& newAngularAcceleration,
    const Frame* relativeTo,
    const Frame* inCoordinatesOf)
{
  assert(nullptr != relativeTo);
  assert(nullptr != inCoordinatesOf);

  Eigen::Vector6d targetSpatialAcceleration;

  targetSpatialAcceleration.head<3>()
      = getChildBodyNode()->getWorldTransform().linear().transpose()
        * inCoordinatesOf->getWorldTransform().linear()
        * newAngularAcceleration;
  // Above code is equivalent to:
  // targetSpatialAcceleration.head<3>()
  //     = getChildBodyNode()->getTransform(
  //         inCoordinatesOf).linear().transpose()
  //       * newAngularAcceleration;
  // but faster.

  if (Frame::World() == relativeTo)
  {
    targetSpatialAcceleration.tail<3>()
        = getChildBodyNode()->getSpatialAcceleration().tail<3>();
  }
  else
  {
    targetSpatialAcceleration.tail<3>()
        = getChildBodyNode()->getSpatialAcceleration(
            relativeTo, getChildBodyNode()).tail<3>();
  }

  setSpatialAcceleration(
        targetSpatialAcceleration, relativeTo, getChildBodyNode());
}

//==============================================================================
Eigen::Matrix6d FreeJoint::getRelativeJacobianStatic(
    const Eigen::Vector6d& positions) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  (void)positions;
  return mJacobian;
#else
  const Eigen::Vector6d& q = positions;
  const Eigen::Isometry3d& T = Joint::mAspectProperties.mT_ChildBodyToJoint;

  Eigen::Matrix6d J;

  J.topLeftCorner<3, 3>().noalias()
      = T.rotation() * math::so3RightJacobian(q.head<3>());
  J.bottomLeftCorner<3, 3>().noalias()
      = math::makeSkewSymmetric(T.translation()) * J.topLeftCorner<3, 3>();

  J.topRightCorner<3, 3>().setZero();
  J.bottomRightCorner<3, 3>().noalias()
      = T.rotation() * math::expMapRot(-q.head<3>());
//  J.bottomRightCorner<3, 3>() = T.rotation();

  return J;

  // return finiteDifferenceRelativeJacobianStatic(positions);
#endif
}

//==============================================================================
Eigen::Matrix6d FreeJoint::finiteDifferenceRelativeJacobianStatic(
    const Eigen::Vector6d& positions) const {
  Eigen::Matrix6d J;

  const Eigen::Vector6d& q = positions;
  const auto& old_q = getPositionsStatic();

  for (auto i = 0u; i < 6; ++i)
  {
    const double EPS = 1e-6;

    Eigen::VectorXd tweaked = q;

    const_cast<FreeJoint*>(this)->setPositions(tweaked);
    auto center = getRelativeTransform();

    tweaked(i) += EPS;
    const_cast<FreeJoint*>(this)->setPositions(tweaked);
    auto plus = getRelativeTransform();

    tweaked = q;
    tweaked(i) -= EPS;
    const_cast<FreeJoint*>(this)->setPositions(tweaked);
    auto minus = getRelativeTransform();

    const Eigen::Matrix4d tmp
        = (center.inverse().matrix()
           * (plus.matrix() - minus.matrix())) / (2 * EPS);
    J.col(i)[0] = tmp(2,1);
    J.col(i)[1] = tmp(0,2);
    J.col(i)[2] = tmp(1,0);
    J.col(i)[3] = tmp(0,3);
    J.col(i)[4] = tmp(1,3);
    J.col(i)[5] = tmp(2,3);
  }

  const_cast<FreeJoint*>(this)->setPositions(old_q);

  return J;
}

//==============================================================================
math::Jacobian FreeJoint::getRelativeJacobianDeriv(std::size_t index) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
   // return finiteDifferenceRelativeJacobianTimeDerivDeriv2(index);
  (void)index;
  return Eigen::Matrix6d::Zero();
#else
//  Eigen::Matrix6d DS_Dq = Eigen::Matrix6d::Zero();

//  const auto& q = getPositionsStatic();
//  const Eigen::Isometry3d& T = Joint::mAspectProperties.mT_ChildBodyToJoint;

//  if (index < 3)
//  {
//    Eigen::Vector3d dq = Eigen::Vector3d::Zero();
//    dq[static_cast<int>(index)] = 1;
//    const Eigen::Matrix3d S = math::so3RightJacobianTimeDeriv(q.head<3>(), dq);

//    DS_Dq.topLeftCorner<3, 3>().noalias() = T.rotation() * S;
//    DS_Dq.bottomLeftCorner<3, 3>().noalias()
//        = math::makeSkewSymmetric(T.translation()) * DS_Dq.topLeftCorner<3, 3>();
//    DS_Dq.bottomRightCorner<3, 3>().noalias()
//        = T.rotation() * math::expMapJacDot(-q.head<3>(), dq);
//  }

  // return DS_Dq;

  // TODO(JS): Change to analytical method
  const auto DS_Dq_num = finiteDifferenceRelativeJacobianDeriv(index);
  // std::cout << "[DEBUG] DS_Dq    :\n" << DS_Dq << std::endl;
  // std::cout << "[DEBUG] DS_Dq_num:\n" << DS_Dq_num << std::endl;
  // std::cout << "[DEBUG] Diff     :\n" << DS_Dq - DS_Dq_num << std::endl << std::endl;
  return DS_Dq_num;
#endif
}

//==============================================================================
math::Jacobian FreeJoint::finiteDifferenceRelativeJacobianDeriv(
    std::size_t index) const
{
  const auto& q = getPositionsStatic();

  const double EPS = 1e-6;
  Eigen::VectorXd tweaked = q;
  tweaked(index) += EPS;
  const_cast<FreeJoint*>(this)->setPositions(tweaked);
  auto plus = getRelativeJacobian();
  tweaked = q;
  tweaked(index) -= EPS;
  const_cast<FreeJoint*>(this)->setPositions(tweaked);
  auto minus = getRelativeJacobian();
  const Eigen::Matrix6d DS_Dq_num = (plus - minus) / (2 * EPS);
  const_cast<FreeJoint*>(this)->setPositions(q);

  return DS_Dq_num;
}

//==============================================================================
math::Jacobian FreeJoint::getRelativeJacobianTimeDerivDeriv(
    std::size_t index) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
 // return finiteDifferenceRelativeJacobianTimeDerivDeriv2(index);
  (void)index;
  return Eigen::Matrix6d::Zero();
#else
//  Eigen::Matrix6d DS_Dq = Eigen::Matrix6d::Zero();

//  const auto& q = getPositionsStatic();
//  const auto& dq = getVelocitiesStatic();
//  const Eigen::Isometry3d& T = Joint::mAspectProperties.mT_ChildBodyToJoint;

//  if (index < 3)
//  {
//    const Eigen::Matrix3d S
//        = math::so3RightJacobianTimeDerivDeriv(
//          q.head<3>(), dq.head<3>(), static_cast<int>(index));

//    DS_Dq.topLeftCorner<3, 3>().noalias() = T.rotation() * S;
//    DS_Dq.bottomLeftCorner<3, 3>().noalias()
//        = math::makeSkewSymmetric(T.translation()) * DS_Dq.topLeftCorner<3, 3>();
//  }

  // return DS_Dq;

  // TODO(JS): Change to analytical method
  const auto DS_Dq_num = finiteDifferenceRelativeJacobianTimeDerivDeriv(index);
  // std::cout << "[DEBUG] DS_Dq    :\n" << DS_Dq << std::endl;
  // std::cout << "[DEBUG] DS_Dq_num:\n" << DS_Dq_num << std::endl;
  // std::cout << "[DEBUG] Diff     :\n" << DS_Dq - DS_Dq_num << std::endl << std::endl;
  return DS_Dq_num;
#endif
}

//==============================================================================
math::Jacobian FreeJoint::finiteDifferenceRelativeJacobianTimeDerivDeriv(
    std::size_t index) const
{
  const auto& q = getPositionsStatic();

  const double EPS = 1e-6;
  Eigen::VectorXd tweaked = q;
  tweaked(index) += EPS;
  const_cast<FreeJoint*>(this)->setPositions(tweaked);
  auto plus = getRelativeJacobianTimeDeriv();
  tweaked = q;
  tweaked(index) -= EPS;
  const_cast<FreeJoint*>(this)->setPositions(tweaked);
  auto minus = getRelativeJacobianTimeDeriv();
  const Eigen::Matrix6d DS_Dq_num = (plus - minus) / (2 * EPS);
  const_cast<FreeJoint*>(this)->setPositions(q);

  return DS_Dq_num;
}

//==============================================================================
math::Jacobian FreeJoint::getRelativeJacobianTimeDerivDeriv2(
    std::size_t index) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
   // return finiteDifferenceRelativeJacobianTimeDerivDeriv2(index);
   (void)index;
   return Eigen::Matrix6d::Zero();
#else
//  Eigen::Matrix6d DS_Dq = Eigen::Matrix6d::Zero();

//  const auto& q = getPositionsStatic();
//  const auto& dq = getVelocitiesStatic();
//  const Eigen::Isometry3d& T = Joint::mAspectProperties.mT_ChildBodyToJoint;

//  if (index < 3)
//  {
//    const Eigen::Matrix3d S
//        = math::so3RightJacobianTimeDerivDeriv2(
//          q.head<3>(), dq.head<3>(), static_cast<int>(index));

//    DS_Dq.topLeftCorner<3, 3>().noalias() = T.rotation() * S;
//    DS_Dq.bottomLeftCorner<3, 3>().noalias()
//        = math::makeSkewSymmetric(T.translation()) * DS_Dq.topLeftCorner<3, 3>();
//  }

  // return DS_Dq;

  // TODO(JS): Change to analytical method
  const auto DS_Dq_num = finiteDifferenceRelativeJacobianTimeDerivDeriv2(index);
  // std::cout << "[DEBUG] DS_Dq    :\n" << DS_Dq << std::endl;
  // std::cout << "[DEBUG] DS_Dq_num:\n" << DS_Dq_num << std::endl;
  // std::cout << "[DEBUG] Diff     :\n" << DS_Dq - DS_Dq_num << std::endl << std::endl;
  return DS_Dq_num;
#endif
}

//==============================================================================
math::Jacobian FreeJoint::finiteDifferenceRelativeJacobianTimeDerivDeriv2(
    std::size_t index) const
{
  const auto& dq = getVelocitiesStatic();

  const double EPS = 1e-6;
  Eigen::VectorXd tweaked = dq;
  tweaked(index) += EPS;
  const_cast<FreeJoint*>(this)->setVelocities(tweaked);
  auto plus = getRelativeJacobianTimeDeriv();
  tweaked = dq;
  tweaked(index) -= EPS;
  const_cast<FreeJoint*>(this)->setVelocities(tweaked);
  auto minus = getRelativeJacobianTimeDeriv();
  const Eigen::Matrix6d DS_Dq_num = (plus - minus) / (2 * EPS);
  const_cast<FreeJoint*>(this)->setVelocities(dq);

  return DS_Dq_num;
}

//==============================================================================
Eigen::Matrix6d FreeJoint::getRelativeJacobianInPositionSpaceStatic(
    const Eigen::Vector6d& positions) const
{
  const Eigen::Vector6d& q = positions;
  Eigen::Matrix6d J;

  J.topLeftCorner<3, 3>().noalias()
      = math::expMapJac(q.head<3>()).transpose();
  J.bottomLeftCorner<3, 3>().setZero();
  J.topRightCorner<3, 3>().setZero();
  J.bottomRightCorner<3, 3>().noalias()
      = math::expMapRot(q.head<3>()).transpose();

  Eigen::Matrix6d result = math::AdTJacFixed(Joint::mAspectProperties.mT_ChildBodyToJoint, J);

#ifndef NDEBUG
  const double threshold = 1e-10;
  Eigen::Matrix6d fd = const_cast<FreeJoint*>(this)->finiteDifferenceRelativeJacobianInPositionSpace();
  if (((fd - result).cwiseAbs().array() > threshold).any())
  {
    std::cout << "FreeJoint position Jacobian wrong!" << std::endl;
    std::cout << "Position:" << std::endl << getPositions() << std::endl;
    std::cout << "Analytical:" << std::endl << result << std::endl;
    std::cout << "Brute Force:" << std::endl << fd << std::endl;
    std::cout << "Diff:" << std::endl << result - fd << std::endl;
    assert(false);
  }
#endif

  return result;
}


//==============================================================================
Eigen::Vector6d FreeJoint::getPositionDifferencesStatic(
    const Eigen::Vector6d& q2,
    const Eigen::Vector6d& q1) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  const Eigen::Isometry3d T1 = convertToTransform(q1);
  const Eigen::Isometry3d T2 = convertToTransform(q2);

  return convertToPositions(T1.inverse() * T2);
#else
  const Eigen::Isometry3d T1 = convertToTransform(q1);
  const Eigen::Isometry3d T2 = convertToTransform(q2);
//  const Eigen::Matrix3d S_angular = math::so3RightJacobian(_q1.head<3>());
//  Eigen::Matrix6d J = Eigen::Matrix6d::Zero();
//  J.topLeftCorner<3, 3>() = S_angular;
//  J.bottomRightCorner<3, 3>() = T1.linear().transpose();

  const auto& J = getRelativeJacobianStatic(q1);
  return J.inverse() * convertToPositions(T1.inverse() * T2);
#endif
}

//==============================================================================
FreeJoint::FreeJoint(const Properties& properties)
#ifdef DART_USE_IDENTITY_JACOBIAN
  : Base(properties),
    mQ(Eigen::Isometry3d::Identity())
#else
  : Base(properties)
#endif
{
  mJacobianDeriv = Eigen::Matrix6d::Zero();

  // Inherited Aspects must be created in the final joint class in reverse order
  // or else we get pure virtual function calls
  createGenericJointAspect(properties);
  createJointAspect(properties);
}

//==============================================================================
Joint* FreeJoint::clone() const
{
  return new FreeJoint(getFreeJointProperties());
}

//==============================================================================
const std::string& FreeJoint::getType() const
{
  return getStaticType();
}

//==============================================================================
const std::string& FreeJoint::getStaticType()
{
  static const std::string name = "FreeJoint";
  return name;
}

//==============================================================================
bool FreeJoint::isCyclic(std::size_t _index) const
{
  return _index < 3
      && !hasPositionLimit(0) && !hasPositionLimit(1) && !hasPositionLimit(2);
}

//==============================================================================
void FreeJoint::integratePositions(double dt)
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  const Eigen::Isometry3d Qnext
      = getQ() * convertToTransform(getVelocitiesStatic() * dt);

  setPositionsStatic(convertToPositions(Qnext));
#else
  const Eigen::Vector6d& q = getPositionsStatic();
  const Eigen::Vector6d& dq = getVelocitiesStatic();

  setPositionsStatic(integratePositionsExplicit(q, dq, dt));
#endif
}

//==============================================================================
#ifndef DART_USE_IDENTITY_JACOBIAN
void FreeJoint::integrateVelocities(double dt)
{
  const Eigen::Vector6d& dq = getVelocitiesStatic();
  const Eigen::Vector6d& ddq = getAccelerationsStatic();

  const auto& S = getRelativeJacobian();
  const auto& dS = getRelativeJacobianTimeDeriv();

  setVelocitiesStatic(S.inverse() * (S * dq + dt * (dS * dq + S * ddq)));

  // setVelocitiesStatic(integratePositionsExplicit(q, dq, dt));
}
#endif

//==============================================================================
Eigen::VectorXd FreeJoint::integratePositionsExplicit(
    const Eigen::VectorXd& pos, const Eigen::VectorXd& vel, double dt) {
#ifdef DART_USE_IDENTITY_JACOBIAN
  const Eigen::Isometry3d mQ = FreeJoint::convertToTransform(pos);
  const Eigen::Isometry3d Qnext = mQ * FreeJoint::convertToTransform(vel * dt);

  return FreeJoint::convertToPositions(Qnext);
#else
  const Eigen::Vector6d& q = pos;
  const Eigen::Vector6d& dq = vel;

//  const Eigen::Matrix3d S_angular = math::so3RightJacobian(q.head<3>());
//  const Eigen::Isometry3d T1 = convertToTransform(q);

//  Eigen::Isometry3d T2 = Eigen::Isometry3d::Identity();
//  T2.linear() = T1.linear() * math::expMapRot(S_angular * dq.head<3>() * dt);
//  T2.translation() = T1.translation() + dq.tail<3>() * dt;

//  return convertToPositions(T2);

  const auto& J = getRelativeJacobianStatic(q);
  return convertToPositions(convertToTransform(q) * convertToTransform(J * dq * dt));
#endif
}

//==============================================================================
Eigen::MatrixXd FreeJoint::getPosPosJacobian(
    const Eigen::VectorXd& pos, const Eigen::VectorXd& vel, double _dt) {
  // TODO
  return finiteDifferencePosPosJacobian(pos, vel, _dt);
}

//==============================================================================
Eigen::MatrixXd FreeJoint::getVelPosJacobian(
    const Eigen::VectorXd& pos, const Eigen::VectorXd& vel, double _dt) {
  // TODO
  return finiteDifferenceVelPosJacobian(pos, vel, _dt);
}

//==============================================================================
/// Returns d/dpos of integratePositionsExplicit() by finite differencing
Eigen::MatrixXd FreeJoint::finiteDifferencePosPosJacobian(
    const Eigen::VectorXd& pos, const Eigen::VectorXd& vel, double dt)
{
  Eigen::MatrixXd jac = Eigen::MatrixXd::Zero(6, 6);
  double EPS = 1e-6;
  for (int i = 0; i < 6; i++) {
    Eigen::VectorXd perturbed = pos;
    perturbed(i) += EPS;
    Eigen::VectorXd plus = integratePositionsExplicit(perturbed, vel, dt);

    perturbed = pos;
    perturbed(i) -= EPS;
    Eigen::VectorXd minus = integratePositionsExplicit(perturbed, vel, dt);

    jac.col(i) = (plus - minus) / (2 * EPS);
  }
  return jac;
}

//==============================================================================
/// Returns d/dvel of integratePositionsExplicit() by finite differencing
Eigen::MatrixXd FreeJoint::finiteDifferenceVelPosJacobian(const Eigen::VectorXd& pos, const Eigen::VectorXd& vel, double dt)
{
  Eigen::MatrixXd jac = Eigen::MatrixXd::Zero(6, 6);
  double EPS = 1e-7;
  for (int i = 0; i < 6; i++) {
    Eigen::VectorXd perturbed = vel;
    perturbed(i) += EPS;
    Eigen::VectorXd plus = integratePositionsExplicit(pos, perturbed, dt);

    perturbed = vel;
    perturbed(i) -= EPS;
    Eigen::VectorXd minus = integratePositionsExplicit(pos, perturbed, dt);

    jac.col(i) = (plus - minus) / (2 * EPS);
  }
  return jac;
}

//==============================================================================
void FreeJoint::updateDegreeOfFreedomNames()
{
  if(!mDofs[0]->isNamePreserved())
    mDofs[0]->setName(Joint::mAspectProperties.mName + "_rot_x", false);
  if(!mDofs[1]->isNamePreserved())
    mDofs[1]->setName(Joint::mAspectProperties.mName + "_rot_y", false);
  if(!mDofs[2]->isNamePreserved())
    mDofs[2]->setName(Joint::mAspectProperties.mName + "_rot_z", false);
  if(!mDofs[3]->isNamePreserved())
    mDofs[3]->setName(Joint::mAspectProperties.mName + "_pos_x", false);
  if(!mDofs[4]->isNamePreserved())
    mDofs[4]->setName(Joint::mAspectProperties.mName + "_pos_y", false);
  if(!mDofs[5]->isNamePreserved())
    mDofs[5]->setName(Joint::mAspectProperties.mName + "_pos_z", false);
}

//==============================================================================
void FreeJoint::updateRelativeTransform() const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  mQ = convertToTransform(getPositionsStatic());

  // T_pj * mQ * T_cj^{-1}

  mT = Joint::mAspectProperties.mT_ParentBodyToJoint * mQ
      * Joint::mAspectProperties.mT_ChildBodyToJoint.inverse();

  assert(math::verifyTransform(mT));
#else
  const Eigen::Isometry3d T = convertToTransform(getPositionsStatic());

  mT = Joint::mAspectProperties.mT_ParentBodyToJoint * T
      * Joint::mAspectProperties.mT_ChildBodyToJoint.inverse();

  assert(math::verifyTransform(mT));
#endif
}

//==============================================================================
void FreeJoint::updateRelativeJacobian(bool mandatory) const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  // Ad[T_cj]

  if (mandatory)
    mJacobian = math::getAdTMatrix(Joint::mAspectProperties.mT_ChildBodyToJoint);
#else
  (void)mandatory;
  mJacobian = getRelativeJacobianStatic(getPositionsStatic());
#endif
}

//==============================================================================
void FreeJoint::updateRelativeJacobianTimeDeriv() const
{
#ifdef DART_USE_IDENTITY_JACOBIAN
  assert(Eigen::Matrix6d::Zero() == mJacobianDeriv);
#else
  const auto& q = getPositionsStatic();
  const auto& dq = getVelocitiesStatic();
  const Eigen::Isometry3d& T = Joint::mAspectProperties.mT_ChildBodyToJoint;

  const Eigen::Matrix3d dJ
      = math::so3RightJacobianTimeDeriv(q.head<3>(), dq.head<3>());

  const Eigen::Matrix3d S = math::so3RightJacobian(q.head<3>());

  mJacobianDeriv.topLeftCorner<3, 3>().noalias() = T.rotation() * dJ;
  mJacobianDeriv.bottomLeftCorner<3, 3>().noalias()
      = math::makeSkewSymmetric(T.translation()) * mJacobianDeriv.topLeftCorner<3, 3>();
  mJacobianDeriv.bottomRightCorner<3, 3>().noalias()
      = T.rotation()
      * math::makeSkewSymmetric(S * -dq.head<3>())
      * math::expMapRot(q.head<3>()).transpose();
#endif
}

//==============================================================================
const Eigen::Isometry3d& FreeJoint::getQ() const
{
  if (mNeedTransformUpdate)
  {
    updateRelativeTransform();
    mNeedTransformUpdate = false;
  }

  return mQ;
}

/*
//==============================================================================
// This gets the world axis screw at the current position, without moving the joint.
Eigen::Vector6d FreeJoint::getWorldAxisScrewForPosition(int dof) const {
  return getWorldAxisScrewAt(getPositionsStatic(), dof);
}
*/

//==============================================================================
// This computes the world axis screw at a given position, without moving the joint.
//
// We do this relative to the parent body, rather than the child body, because in
// moving the joint we also move the child body.
Eigen::Vector6d FreeJoint::getWorldAxisScrewAt(Eigen::Vector6d pos, int dof) const
{
  Eigen::Vector6d grad = Eigen::Vector6d::Zero();
  if (dof < 3) {
    grad.head<3>() = math::expMapJac(pos.head<3>()).col(dof);
    // Shift so the rotation is taking place at the relative center
    Eigen::Isometry3d translation = Eigen::Isometry3d::Identity();
    translation.translation() = pos.tail<3>();
    grad = math::AdT(translation, grad);
  }
  else {
    grad(dof) = 1.0;
  }
  Eigen::Vector6d parentTwist = math::AdT(Joint::mAspectProperties.mT_ParentBodyToJoint, grad);

  Eigen::Isometry3d parentTransform = Eigen::Isometry3d::Identity();
  if (getParentBodyNode() != nullptr) {
    parentTransform = getParentBodyNode()->getWorldTransform();
  }
  return math::AdT(parentTransform, parentTwist);
}

//==============================================================================
// This estimates the new world screw axis at `axisDof` when we perturbe `rotateDof` by `eps`
Eigen::Vector6d FreeJoint::estimatePerturbedScrewAxisForPosition(
  int axisDof,
  int rotateDof,
  double eps)
{
  Eigen::Vector6d pos = getPositionsStatic();
  pos(rotateDof) += eps;
  return getWorldAxisScrewAt(pos, axisDof);
}

//==============================================================================
// This estimates the new world screw axis at `axisDof` when we perturbe `rotateDof` by `eps`
Eigen::Vector6d FreeJoint::estimatePerturbedScrewAxisForForce(
  int axisDof,
  int rotateDof,
  double eps)
{
  Eigen::Vector6d pos = getPositionsStatic();
  pos(rotateDof) += eps;

  Eigen::Isometry3d parentTransform = Eigen::Isometry3d::Identity();
  if (getParentBodyNode() != nullptr) {
    parentTransform = getParentBodyNode()->getWorldTransform();
  }
  return math::AdT(
    parentTransform * Joint::mAspectProperties.mT_ParentBodyToJoint * convertToTransform(pos)
        * Joint::mAspectProperties.mT_ChildBodyToJoint.inverse(), getRelativeJacobian(pos).col(axisDof));
}

//==============================================================================
// Returns the gradient of the screw axis with respect to the rotate dof
Eigen::Vector6d FreeJoint::getScrewAxisGradientForPosition(
  int axisDof,
  int rotateDof)
{
  double EPS = 5e-9;
  Eigen::Vector6d pos = estimatePerturbedScrewAxisForPosition(axisDof, rotateDof, EPS);
  Eigen::Vector6d neg = estimatePerturbedScrewAxisForPosition(axisDof, rotateDof, -EPS);
  return (pos - neg) / (2 * EPS);
}

//==============================================================================
// Returns the gradient of the screw axis with respect to the rotate dof
Eigen::Vector6d FreeJoint::getScrewAxisGradientForForce(
  int axisDof,
  int rotateDof)
{
  // toRotate is constant wrt position
  Eigen::Vector6d toRotate = math::AdT(Joint::mAspectProperties.mT_ChildBodyToJoint.inverse(), getRelativeJacobian().col(axisDof));
  Eigen::Vector6d grad = Eigen::Vector6d::Zero();

  // If rotateDof is a rotation index (first 3), treat it like an offset BallJoint
  if (rotateDof < 3) {
    // Compute gradients here just like BallJoint
    Eigen::Matrix3d rotate = math::expMapRot(getPositionsStatic().head<3>());
    Eigen::Vector3d screwAxis = math::expMapJac(getPositionsStatic().head<3>()).row(rotateDof);
    grad.head<3>() = rotate * screwAxis.cross(toRotate.head<3>());
    grad.tail<3>() = rotate * screwAxis.cross(toRotate.tail<3>());
    // Offset, without rotation, so that "grad" is now centered back at the root of the joint
    Eigen::Isometry3d transform = Eigen::Isometry3d::Identity();
    transform.translation() = getPositionsStatic().tail<3>();
    grad = math::AdT(transform, grad);
  }
  // Otherwise rotateDof is a translation index (last 3)
  else {
    assert(rotateDof >= 3 && rotateDof < 6);

    Eigen::Matrix3d rotate = math::expMapRot(getPositionsStatic().head<3>());
    Eigen::Vector3d unitGrad = Eigen::Vector3d::Unit(rotateDof - 3);
    grad.tail<3>() = unitGrad.cross(rotate * toRotate.head<3>());
  }

  Eigen::Isometry3d parentTransform = Eigen::Isometry3d::Identity();
  if (getParentBodyNode() != nullptr) {
    parentTransform = getParentBodyNode()->getWorldTransform();
  }
  return math::AdT(
    parentTransform * Joint::mAspectProperties.mT_ParentBodyToJoint, grad);
}

}  // namespace dynamics
}  // namespace dart
