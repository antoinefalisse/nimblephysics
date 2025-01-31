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

#ifndef DART_DYNAMICS_FREEJOINT_HPP_
#define DART_DYNAMICS_FREEJOINT_HPP_

#include <string>

#include <Eigen/Dense>

#include "dart/common/Deprecated.hpp"
#include "dart/dynamics/GenericJoint.hpp"

namespace dart {
namespace dynamics {

/// class FreeJoint
class FreeJoint : public GenericJoint<math::SE3Space>
{
public:
  friend class Skeleton;

  using Base = GenericJoint<math::SE3Space>;

  struct Properties : Base::Properties
  {
    DART_DEFINE_ALIGNED_SHARED_OBJECT_CREATOR(Properties)

    Properties(const Base::Properties& properties = Base::Properties());

    virtual ~Properties() = default;
  };

  FreeJoint(const FreeJoint&) = delete;

  /// Destructor
  virtual ~FreeJoint();

  /// Get the Properties of this FreeJoint
  Properties getFreeJointProperties() const;

  // Documentation inherited
  const std::string& getType() const override;

  /// Get joint type for this class
  static const std::string& getStaticType();

  // Documentation inherited
  bool isCyclic(std::size_t _index) const override;

  /// Convert a transform into a 6D vector that can be used to set the positions
  /// of a FreeJoint. The positions returned by this function will result in a
  /// relative transform of
  /// getTransformFromParentBodyNode() * _tf *
  /// getTransformFromChildBodyNode().inverse() between the parent BodyNode and
  /// the child BodyNode frames when applied to a FreeJoint.
  static Eigen::Vector6s convertToPositions(const Eigen::Isometry3s& _tf);

  /// Convert a FreeJoint-style 6D vector into a transform
  static Eigen::Isometry3s convertToTransform(
      const Eigen::Vector6s& _positions);

  /// If the given joint is a FreeJoint, then set the transform of the given
  /// Joint's child BodyNode so that its transform with respect to
  /// "withRespecTo" is equal to "tf".
  ///
  /// \deprecated Deprecated in DART 6.9. Use setTransformOf() instead
  DART_DEPRECATED(6.9)
  static void setTransform(
      Joint* joint,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World());

  /// If the given joint is a FreeJoint, then set the transform of the given
  /// Joint's child BodyNode so that its transform with respect to
  /// "withRespecTo" is equal to "tf".
  static void setTransformOf(
      Joint* joint,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World());

  /// If the parent Joint of the given BodyNode is a FreeJoint, then set the
  /// transform of the given BodyNode so that its transform with respect to
  /// "withRespecTo" is equal to "tf".
  ///
  /// \deprecated Deprecated in DART 6.9. Use setTransformOf() instead
  DART_DEPRECATED(6.9)
  static void setTransform(
      BodyNode* bodyNode,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World());

  /// If the parent Joint of the given BodyNode is a FreeJoint, then set the
  /// transform of the given BodyNode so that its transform with respect to
  /// "withRespecTo" is equal to "tf".
  static void setTransformOf(
      BodyNode* bodyNode,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World());

  /// Apply setTransform(bodyNode, tf, withRespecTo) for all the root BodyNodes
  /// of the given Skeleton. If false is passed in "applyToAllRootBodies", then
  /// it will be applied to only the default root BodyNode that will be obtained
  /// by Skeleton::getRootBodyNode().
  ///
  /// \deprecated Deprecated in DART 6.9. Use setTransformOf() instead
  DART_DEPRECATED(6.9)
  static void setTransform(
      Skeleton* skeleton,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World(),
      bool applyToAllRootBodies = true);

  /// Apply setTransform(bodyNode, tf, withRespecTo) for all the root BodyNodes
  /// of the given Skeleton. If false is passed in "applyToAllRootBodies", then
  /// it will be applied to only the default root BodyNode that will be obtained
  /// by Skeleton::getRootBodyNode().
  static void setTransformOf(
      Skeleton* skeleton,
      const Eigen::Isometry3s& tf,
      const Frame* withRespectTo = Frame::World(),
      bool applyToAllRootBodies = true);

  /// Set the transform, spatial velocity, and spatial acceleration of the child
  /// BodyNode relative to an arbitrary Frame. The reference frame can be
  /// arbitrarily specified.
  ///
  /// If you want to set more than one kind of Cartetian coordinates (e.g.,
  /// transform and spatial velocity) at the same time, you should call
  /// corresponding setters in a certain order (transform -> velocity ->
  /// acceleration), If you don't velocity or acceleration can be corrupted by
  /// transform or velocity. This function calls the corresponding setters in
  /// the right order so that all the desired Cartetian coordinates are properly
  /// set.
  ///
  /// Pass nullptr for "newTransform", "newSpatialVelocity", or
  /// "newSpatialAcceleration" if you don't want to set them.
  ///
  /// \param[in] newTransform Desired transform of the child BodyNode.
  /// \param[in] withRespectTo The relative Frame of "newTransform".
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] velRelativeTo The relative frame of "newSpatialVelocity".
  /// \param[in] velInCoordinatesOf The reference frame of "newSpatialVelocity".
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] accRelativeTo The relative frame of "newSpatialAcceleration".
  /// \param[in] accInCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setSpatialMotion(
      const Eigen::Isometry3s* newTransform,
      const Frame* withRespectTo,
      const Eigen::Vector6s* newSpatialVelocity,
      const Frame* velRelativeTo,
      const Frame* velInCoordinatesOf,
      const Eigen::Vector6s* newSpatialAcceleration,
      const Frame* accRelativeTo,
      const Frame* accInCoordinatesOf);

  /// Set the transform of the child BodyNode relative to the parent BodyNode
  /// \param[in] newTransform Desired transform of the child BodyNode.
  void setRelativeTransform(const Eigen::Isometry3s& newTransform);

  /// Set the transform of the child BodyNode relative to an arbitrary Frame.
  /// \param[in] newTransform Desired transform of the child BodyNode.
  /// \param[in] withRespectTo The relative Frame of "newTransform".
  void setTransform(
      const Eigen::Isometry3s& newTransform,
      const Frame* withRespectTo = Frame::World());

  /// Set the spatial velocity of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode. The reference frame of "newSpatialVelocity" is the child
  /// BodyNode.
  void setRelativeSpatialVelocity(const Eigen::Vector6s& newSpatialVelocity);

  /// Set the spatial velocity of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] inCoordinatesOf The reference frame of "newSpatialVelocity".
  void setRelativeSpatialVelocity(
      const Eigen::Vector6s& newSpatialVelocity, const Frame* inCoordinatesOf);

  /// Set the spatial velocity of the child BodyNode relative to an arbitrary
  /// Frame.
  /// \param[in] newSpatialVelocity Desired spatial velocity of the child
  /// BodyNode.
  /// \param[in] relativeTo The relative frame of "newSpatialVelocity".
  /// \param[in] inCoordinatesOf The reference frame of "newSpatialVelocity".
  void setSpatialVelocity(
      const Eigen::Vector6s& newSpatialVelocity,
      const Frame* relativeTo,
      const Frame* inCoordinatesOf);

  /// Set the linear portion of classical velocity of the child BodyNode
  /// relative to an arbitrary Frame.
  ///
  /// Note that the angular portion of claasical velocity of the child
  /// BodyNode will not be changed after this function called.
  ///
  /// \param[in] newLinearVelocity
  /// \param[in] relativeTo The relative frame of "newLinearVelocity".
  /// \param[in] inCoordinatesOf The reference frame of "newLinearVelocity".
  void setLinearVelocity(
      const Eigen::Vector3s& newLinearVelocity,
      const Frame* relativeTo = Frame::World(),
      const Frame* inCoordinatesOf = Frame::World());

  /// Set the angular portion of classical velocity of the child BodyNode
  /// relative to an arbitrary Frame.
  ///
  /// Note that the linear portion of claasical velocity of the child
  /// BodyNode will not be changed after this function called.
  ///
  /// \param[in] newAngularVelocity
  /// \param[in] relativeTo The relative frame of "newAngularVelocity".
  /// \param[in] inCoordinatesOf The reference frame of "newAngularVelocity".
  void setAngularVelocity(
      const Eigen::Vector3s& newAngularVelocity,
      const Frame* relativeTo = Frame::World(),
      const Frame* inCoordinatesOf = Frame::World());

  /// Set the spatial acceleration of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode. The reference frame of "newSpatialAcceleration" is the
  /// child BodyNode.
  void setRelativeSpatialAcceleration(
      const Eigen::Vector6s& newSpatialAcceleration);

  /// Set the spatial acceleration of the child BodyNode relative to the parent
  /// BodyNode.
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] inCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setRelativeSpatialAcceleration(
      const Eigen::Vector6s& newSpatialAcceleration,
      const Frame* inCoordinatesOf);

  /// Set the spatial acceleration of the child BodyNode relative to an
  /// arbitrary Frame.
  /// \param[in] newSpatialAcceleration Desired spatial acceleration of the
  /// child BodyNode.
  /// \param[in] relativeTo The relative frame of "newSpatialAcceleration".
  /// \param[in] inCoordinatesOf The reference frame of
  /// "newSpatialAcceleration".
  void setSpatialAcceleration(
      const Eigen::Vector6s& newSpatialAcceleration,
      const Frame* relativeTo,
      const Frame* inCoordinatesOf);

  /// Set the linear portion of classical acceleration of the child BodyNode
  /// relative to an arbitrary Frame.
  ///
  /// Note that the angular portion of claasical acceleration of the child
  /// BodyNode will not be changed after this function called.
  ///
  /// \param[in] newLinearAcceleration
  /// \param[in] relativeTo The relative frame of "newLinearAcceleration".
  /// \param[in] inCoordinatesOf The reference frame of "newLinearAcceleration".
  void setLinearAcceleration(
      const Eigen::Vector3s& newLinearAcceleration,
      const Frame* relativeTo = Frame::World(),
      const Frame* inCoordinatesOf = Frame::World());

  /// Set the angular portion of classical acceleration of the child BodyNode
  /// relative to an arbitrary Frame.
  ///
  /// Note that the linear portion of claasical acceleration of the child
  /// BodyNode will not be changed after this function called.
  ///
  /// \param[in] newAngularAcceleration
  /// \param[in] relativeTo The relative frame of "newAngularAcceleration".
  /// \param[in] inCoordinatesOf The reference frame of
  /// "newAngularAcceleration".
  void setAngularAcceleration(
      const Eigen::Vector3s& newAngularAcceleration,
      const Frame* relativeTo = Frame::World(),
      const Frame* inCoordinatesOf = Frame::World());

  // Documentation inherited
  Eigen::Matrix6s getRelativeJacobianStatic(
      const Eigen::Vector6s& _positions) const override;

  Eigen::Matrix6s finiteDifferenceRelativeJacobianStatic(
      const Eigen::Vector6s& _positions) const;

  Eigen::Matrix6s getRelativeJacobianDerivWrtPositionStatic(
      std::size_t index) const override;
  math::Jacobian finiteDifferenceRelativeJacobianDeriv(std::size_t index) const;

  math::Jacobian getRelativeJacobianTimeDerivDerivWrtPosition(
      std::size_t index) const override;
  math::Jacobian finiteDifferenceRelativeJacobianTimeDerivDeriv(
      std::size_t index) const;

  math::Jacobian getRelativeJacobianTimeDerivDerivWrtVelocity(
      std::size_t index) const override;
  math::Jacobian finiteDifferenceRelativeJacobianTimeDerivDeriv2(
      std::size_t index) const;

  // Documentation inherited
  Eigen::Matrix6s getRelativeJacobianInPositionSpaceStatic(
      const Eigen::Vector6s& _positions) const override;

  // Documentation inherited
  Eigen::Vector6s getPositionDifferencesStatic(
      const Eigen::Vector6s& _q2, const Eigen::Vector6s& _q1) const override;

  /*
  // This gets the world axis screw at the current position, without moving the
  joint. Eigen::Vector6s getWorldAxisScrewForPosition(int dof) const override;
  */

  // This computes the world axis screw at a given position, without moving the
  // joint.
  Eigen::Vector6s getWorldAxisScrewAt(Eigen::Vector6s pos, int dof) const;

  // This estimates the new world screw axis at `axisDof` when we perturbe
  // `rotateDof` by `eps`
  Eigen::Vector6s estimatePerturbedScrewAxisForPosition(
      int axisDof, int rotateDof, s_t eps);

  // This estimates the new world screw axis at `axisDof` when we perturbe
  // `rotateDof` by `eps`
  Eigen::Vector6s estimatePerturbedScrewAxisForForce(
      int axisDof, int rotateDof, s_t eps);

  // Returns the gradient of the screw axis with respect to the rotate dof
  Eigen::Vector6s getScrewAxisGradientForPosition(
      int axisDof, int rotateDof) override;

  // Returns the gradient of the screw axis with respect to the rotate dof
  Eigen::Vector6s getScrewAxisGradientForForce(
      int axisDof, int rotateDof) override;

protected:
  /// Constructor called by Skeleton class
  FreeJoint(const Properties& properties);

  // Documentation inherited
  Joint* clone() const override;

  using Base::getRelativeJacobianStatic;

  // Documentation inherited
  void integratePositions(s_t _dt) override;

#ifndef DART_USE_IDENTITY_JACOBIAN
  // Documentation inherited
  void integrateVelocities(s_t dt) override;
#endif

  // Documentation inherited
  Eigen::VectorXs integratePositionsExplicit(
      const Eigen::VectorXs& pos, const Eigen::VectorXs& vel, s_t dt) override;

  /// Returns d/dpos of integratePositionsExplicit()
  Eigen::MatrixXs getPosPosJacobian(
      const Eigen::VectorXs& pos, const Eigen::VectorXs& vel, s_t _dt) override;

  /// Returns d/dvel of integratePositionsExplicit()
  Eigen::MatrixXs getVelPosJacobian(
      const Eigen::VectorXs& pos, const Eigen::VectorXs& vel, s_t _dt) override;

  /// Returns d/dpos of integratePositionsExplicit() by finite differencing
  Eigen::MatrixXs finiteDifferencePosPosJacobian(
      const Eigen::VectorXs& pos, const Eigen::VectorXs& vel, s_t _dt);

  /// Returns d/dvel of integratePositionsExplicit() by finite differencing
  Eigen::MatrixXs finiteDifferenceVelPosJacobian(
      const Eigen::VectorXs& pos, const Eigen::VectorXs& vel, s_t _dt);

  // Documentation inherited
  void updateDegreeOfFreedomNames() override;

  // Documentation inherited
  void updateRelativeTransform() const override;

  // Documentation inherited
  void updateRelativeJacobian(bool _mandatory = true) const override;

  // Documentation inherited
  void updateRelativeJacobianTimeDeriv() const override;

#ifdef DART_USE_IDENTITY_JACOBIAN
protected:
  /// Access mQ, which is an auto-updating variable
  const Eigen::Isometry3s& getQ() const;

  /// Transformation matrix dependent on generalized coordinates
  ///
  /// Do not use directly! Use getQ() to access this
  mutable Eigen::Isometry3s mQ;
#endif

public:
  // To get byte-aligned Eigen vectors
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

} // namespace dynamics
} // namespace dart

#endif // DART_DYNAMICS_FREEJOINT_HPP_
