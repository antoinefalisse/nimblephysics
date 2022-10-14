#include <memory>
#include <string>

#include <gtest/gtest.h>

#include "dart/biomechanics/OpenSimParser.hpp"
#include "dart/dart.hpp"
#include "dart/dynamics/BodyNode.hpp"
#include "dart/dynamics/BoxShape.hpp"
#include "dart/dynamics/ConstantCurveIncompressibleJoint.hpp"
#include "dart/dynamics/EulerJoint.hpp"
#include "dart/dynamics/Frame.hpp"
#include "dart/dynamics/detail/BodyNodePtr.hpp"
#include "dart/math/Geometry.hpp"
#include "dart/math/LinearFunction.hpp"
#include "dart/math/MathTypes.hpp"
#include "dart/server/GUIRecording.hpp"
#include "dart/simulation/World.hpp"

#include "TestHelpers.hpp"

using namespace dart;

// #define ALL_TESTS
// #define GUI_TESTS

//==============================================================================
// We know the Jacobians are correct for a number of joints, so we can check our
// finite differencing works correctly
bool verifyJacobianFiniteDifferencing(dynamics::Joint* shoulder)
{
  Eigen::VectorXs pos = shoulder->getPositions();
  Eigen::VectorXs vel = shoulder->getVelocities();

  math::Jacobian j = shoulder->getRelativeJacobian();
  math::Jacobian j_fd = shoulder->finiteDifferenceRelativeJacobian();

  if (!equals(j, j_fd, 1e-8))
  {
    std::cout << "relativeJacobian: " << std::endl;
    std::cout << "Analytical j: " << std::endl << j << std::endl;
    std::cout << "FD j: " << std::endl << j_fd << std::endl;
    std::cout << "Diff: " << std::endl << j - j_fd << std::endl;
    EXPECT_TRUE(equals(j, j_fd, 1e-8));
    return false;
  }
  return true;
}

//==============================================================================
bool verifyConstantCurveIncompressibleJoint(
    dynamics::ConstantCurveIncompressibleJoint* shoulder, s_t TEST_THRESHOLD)
{
  Eigen::VectorXs pos = shoulder->getPositions();
  Eigen::VectorXs vel = shoulder->getVelocities();

  for (int i = 0; i < 3; i++)
  {
    for (int j = 0; j < 3; j++)
    {
      Eigen::MatrixXs scratch = shoulder->analyticalScratch(i, j);
      Eigen::MatrixXs scratch_fd = shoulder->finiteDifferenceScratch(i, j);
      if (scratch.hasNaN())
      {
        std::cout << "Scatch failed for Jac wrt " << i << " wrt " << j
                  << std::endl;
        std::cout << "Analytical scratch: " << std::endl
                  << scratch << std::endl;
        EXPECT_FALSE(scratch.hasNaN());
        return false;
      }
      if (!equals(scratch, scratch_fd, TEST_THRESHOLD))
      {
        std::cout << "Scatch failed for Jac wrt " << i << " wrt " << j
                  << std::endl;
        std::cout << "Analytical scratch: " << std::endl
                  << scratch << std::endl;
        std::cout << "FD scratch: " << std::endl << scratch_fd << std::endl;
        std::cout << "Diff: " << std::endl << scratch - scratch_fd << std::endl;
        EXPECT_TRUE(equals(scratch, scratch_fd, TEST_THRESHOLD));
        return false;
      }
    }
  }

  math::Jacobian j = shoulder->getRelativeJacobian();
  math::Jacobian j_fd = shoulder->finiteDifferenceRelativeJacobian();

  if (!equals(j, j_fd, TEST_THRESHOLD))
  {
    std::cout << "relativeJacobian: " << std::endl;
    std::cout << "Analytical j: " << std::endl << j << std::endl;
    std::cout << "FD j: " << std::endl << j_fd << std::endl;
    std::cout << "Diff: " << std::endl << j - j_fd << std::endl;
    EXPECT_TRUE(equals(j, j_fd, TEST_THRESHOLD));
    return false;
  }

  for (int i = 0; i < 3; i++)
  {
    math::Jacobian dj = shoulder->getRelativeJacobianDerivWrtPositionStatic(i);
    math::Jacobian dj_fd
        = shoulder->finiteDifferenceRelativeJacobianDerivWrtPosition(i);

    if (!equals(dj, dj_fd, TEST_THRESHOLD))
    {
      std::cout << "FD J: " << std::endl << j_fd << std::endl;

      std::cout << "relativeJacobianDeriv(index=" << i << "): " << std::endl;
      std::cout << "Analytical dj: " << std::endl << dj << std::endl;
      std::cout << "FD dj: " << std::endl << dj_fd << std::endl;
      std::cout << "Diff: " << std::endl << dj - dj_fd << std::endl;
      EXPECT_TRUE(equals(dj, dj_fd, TEST_THRESHOLD));
      return false;
    }
  }

  math::Jacobian dj_dt = shoulder->getRelativeJacobianTimeDeriv();
  math::Jacobian dj_dt_fd
      = shoulder->finiteDifferenceRelativeJacobianTimeDeriv();

  if (!equals(dj_dt, dj_dt_fd, TEST_THRESHOLD))
  {
    std::cout << "relativeJacobianTimeDeriv: " << std::endl;
    std::cout << "Analytical dj_dt: " << std::endl << dj_dt << std::endl;
    std::cout << "FD dj_dt: " << std::endl << dj_dt_fd << std::endl;
    std::cout << "Diff: " << std::endl << dj_dt - dj_dt_fd << std::endl;
    EXPECT_TRUE(equals(dj_dt, dj_dt_fd, TEST_THRESHOLD));
    return false;
  }

  for (int i = 0; i < shoulder->getNumDofs(); i++)
  {
    ////////////////////////////////////////////////////////////////////////////////
    // Check d/dt d/dx of relative Jacobians
    ////////////////////////////////////////////////////////////////////////////////

    math::Jacobian dj_dt_dp
        = shoulder->getRelativeJacobianTimeDerivDerivWrtPosition(i);
    math::Jacobian dj_dt_dp_fd
        = shoulder->finiteDifferenceRelativeJacobianTimeDerivDerivWrtPosition(
            i);

    if (!equals(dj_dt_dp, dj_dt_dp_fd, TEST_THRESHOLD))
    {
      std::cout << "getRelativeJacobianTimeDerivDerivWrtPosition(index=" << i
                << "): " << std::endl;
      std::cout << "Analytical dj dt dp: " << std::endl
                << dj_dt_dp << std::endl;
      std::cout << "FD dj dt dp: " << std::endl << dj_dt_dp_fd << std::endl;
      std::cout << "Diff: " << std::endl << dj_dt_dp - dj_dt_dp_fd << std::endl;
      EXPECT_TRUE(equals(dj_dt_dp, dj_dt_dp_fd, TEST_THRESHOLD));
      return false;
    }

    math::Jacobian dj_dt_dv
        = shoulder->getRelativeJacobianTimeDerivDerivWrtVelocity(i);
    math::Jacobian dj_dt_dv_fd
        = shoulder->finiteDifferenceRelativeJacobianTimeDerivDerivWrtVelocity(
            i);

    if (!equals(dj_dt_dv, dj_dt_dv_fd, TEST_THRESHOLD))
    {
      std::cout << "getRelativeJacobianTimeDerivDerivWrtVelocity(index=" << i
                << "): " << std::endl;
      std::cout << "Analytical dj dt dv: " << std::endl
                << dj_dt_dv << std::endl;
      std::cout << "FD dj dt dv: " << std::endl << dj_dt_dv_fd << std::endl;
      std::cout << "Diff: " << std::endl << dj_dt_dv - dj_dt_dv_fd << std::endl;
      EXPECT_TRUE(equals(dj_dt_dv, dj_dt_dv_fd, TEST_THRESHOLD));
      return false;
    }
  }

  return true;
}

//==============================================================================
#ifdef GUI_TESTS
TEST(ConstantCurveIncompressibleJoint, DEBUG_RANGE_OF_MOTION_TO_GUI)
{
  server::GUIRecording server;
  server.setFramesPerSecond(20);

  std::shared_ptr<dynamics::Skeleton> skel = dynamics::Skeleton::create();
  auto pair = skel->createJointAndBodyNodePair<
      dynamics::ConstantCurveIncompressibleJoint>();
  ConstantCurveIncompressibleJoint* joint = pair.first;
  joint->setAxisOrder(EulerJoint::AxisOrder::XZY);
  BodyNode* body = pair.second;
  std::shared_ptr<dynamics::BoxShape> boxShape
      = std::make_shared<dynamics::BoxShape>(Eigen::Vector3s(0.01, 0.01, 0.01));
  body->createShapeNodeWith<dynamics::VisualAspect>(boxShape);

  // Render the ellipse that we'll be sliding the scapula along the top of
  server.renderBasis();

  // Do the whole range of motion
  for (int i = -10; i < 10; i++)
  {
    for (int j = -10; j < 10; j++)
    {
      Eigen::Vector4s pos
          = Eigen::Vector4s(i * 0.1, j * 0.1, i * j * 0.01, 0.0);
      skel->setPositions(pos);
      server.renderSkeleton(skel);

      for (int frac = 0; frac < 20; frac++)
      {
        s_t percentage = (s_t)frac / 20;
        Eigen::Vector4s localPos = pos * percentage;
        localPos(3) = -1 + percentage;
        skel->setPositions(localPos);
        server.renderSkeleton(skel, "frac_" + std::to_string(frac));
      }

      server.saveFrame();
    }
  }

  server.writeFramesJson("../../../javascript/src/data/movement2.bin");
}
#endif

//==============================================================================
#ifdef ALL_TESTS
TEST(ConstantCurveIncompressibleJoint, EulerJacobian)
{
  EulerJoint::Properties props;
  EulerJoint joint(props);
  joint.setAxisOrder(EulerJoint::AxisOrder::XZY);

  // Check at random positions
  for (int i = 0; i < 10; i++)
  {
    joint.setPositions(Eigen::VectorXs::Random(joint.getNumDofs()));
    joint.setVelocities(Eigen::VectorXs::Random(joint.getNumDofs()));

    /*
    joint.setPosition(0, 0.0);
    joint.setVelocity(0, 1.0);
    */
    std::cout << "Testing: " << joint.getPositions() << ".."
              << joint.getVelocities() << std::endl;

    if (!verifyJacobianFiniteDifferencing(&joint))
    {
      return;
    }
  }
}
#endif

//==============================================================================
#ifdef SIM_TESTS
TEST(ConstantCurveIncompressibleJoint, SAVE_SIM_TO_GUI)
{
  std::shared_ptr<dynamics::Skeleton> skel = dynamics::Skeleton::create();

  Eigen::Vector3s jointUpperLimits = Eigen::Vector3s(M_PI, M_PI, M_PI);
  Eigen::Vector3s jointLowerLimits = -jointUpperLimits;

  s_t boxSize = 0.05;
  s_t springStiffness = 10;

  auto pair = skel->createJointAndBodyNodePair<
      dynamics::ConstantCurveIncompressibleJoint>();
  ConstantCurveIncompressibleJoint* joint = pair.first;
  joint->setLength(0.2);
  joint->setPositionLowerLimits(jointLowerLimits);
  joint->setPositionUpperLimits(jointUpperLimits);
  joint->setPositionLimitEnforced(true);
  joint->setRestPosition(0, 0);
  joint->setRestPosition(1, 0);
  joint->setRestPosition(2, 0);
  joint->setSpringStiffness(0, springStiffness);
  joint->setSpringStiffness(1, springStiffness);
  joint->setSpringStiffness(2, springStiffness);
  joint->setPosition(0, 0.05);
  BodyNode* body = pair.second;
  std::shared_ptr<dynamics::BoxShape> boxShape
      = std::make_shared<dynamics::BoxShape>(
          Eigen::Vector3s(boxSize, boxSize, boxSize));
  body->createShapeNodeWith<dynamics::VisualAspect>(boxShape);

  for (int i = 0; i < 3; i++)
  {
    auto childPair = body->createChildJointAndBodyNodePair<
        dynamics::ConstantCurveIncompressibleJoint>();
    ConstantCurveIncompressibleJoint* joint = childPair.first;
    joint->setLength(0.2);
    joint->setPositionLowerLimits(jointLowerLimits);
    joint->setPositionUpperLimits(jointUpperLimits);
    joint->setPositionLimitEnforced(true);
    joint->setRestPosition(0, 0);
    joint->setRestPosition(1, 0);
    joint->setRestPosition(2, 0);
    joint->setSpringStiffness(0, springStiffness);
    joint->setSpringStiffness(1, springStiffness);
    joint->setSpringStiffness(2, springStiffness);
    body = childPair.second;
    std::shared_ptr<dynamics::BoxShape> boxShape
        = std::make_shared<dynamics::BoxShape>(
            Eigen::Vector3s(boxSize, boxSize, boxSize));
    body->createShapeNodeWith<dynamics::VisualAspect>(boxShape);
  }

  std::shared_ptr<simulation::World> world = simulation::World::create();
  world->addSkeleton(skel);
  world->setTimeStep(1.0 / 100);

  server::GUIRecording server;
  server.setFramesPerSecond(100);

  // Render the ellipse that we'll be sliding the scapula along the top of
  server.renderBasis();
  server.renderWorld(world);
  server.saveFrame();

  for (int i = 0; i < 600; i++)
  {
    world->step();
    server.renderWorld(world);
    server.saveFrame();
  }

  server.writeFramesJson("../../../javascript/src/data/movement2.bin");
}
#endif

//==============================================================================
// #ifdef ALL_TESTS
TEST(ConstantCurveIncompressibleJoint, ConstantCurveJacobians)
{
  ConstantCurveIncompressibleJoint::Properties props;
  ConstantCurveIncompressibleJoint joint(props);
  joint.setAxisOrder(EulerJoint::AxisOrder::XZY);

  // Set the parameters of the example shoulder
  Eigen::Isometry3s transformFromParent = Eigen::Isometry3s::Identity();
  transformFromParent.translation() = Eigen::Vector3s(-0.02, -0.0173, 0.07);
  transformFromParent.linear()
      = math::eulerXYZToMatrix(Eigen::Vector3s(0, -0.87, 0));
  joint.setTransformFromParentBodyNode(transformFromParent);
  Eigen::Isometry3s transformFromChild = Eigen::Isometry3s::Identity();
  transformFromChild.translation()
      = Eigen::Vector3s(-0.05982, -0.03904, -0.056);
  transformFromChild.linear()
      = math::eulerXYZToMatrix(Eigen::Vector3s(-0.5181, -1.1416, -0.2854));

  joint.setPositions(Eigen::Vector3s::Zero());
  joint.setVelocities(Eigen::Vector3s::Zero());
  std::cout << "Testing zero pos and zero vel, with _no_ child transform"
            << std::endl;

  if (!verifyConstantCurveIncompressibleJoint(&joint, 1e-9))
  {
    return;
  }

  for (int i = 0; i < 3; i++)
  {
    joint.setPositions(Eigen::Vector3s::Unit(i));
    std::cout << "Testing euler pos(" << std::to_string(i)
              << ")=1, zero vel, with _no_ child transform" << std::endl;
    if (!verifyConstantCurveIncompressibleJoint(&joint, 1e-9))
    {
      return;
    }
  }

  joint.setPositions(Eigen::Vector3s::Zero());
  joint.setVelocities(Eigen::Vector3s::Zero());
  joint.setTransformFromChildBodyNode(transformFromChild);
  std::cout << "Testing zero pos and zero vel, _with_ a child transform"
            << std::endl;

  if (!verifyConstantCurveIncompressibleJoint(&joint, 1e-9))
  {
    return;
  }

  // Check at random positions
  for (int i = 0; i < 10; i++)
  {
    joint.setPositions(Eigen::Vector3s::Random());
    Eigen::Vector3s vel = Eigen::Vector3s::Random();
    joint.setVelocities(vel);

    std::cout << "Testing: " << joint.getPositions() << ".."
              << joint.getVelocities() << std::endl;

    if (!verifyConstantCurveIncompressibleJoint(&joint, 1e-9))
    {
      return;
    }
  }
}
// #endif
