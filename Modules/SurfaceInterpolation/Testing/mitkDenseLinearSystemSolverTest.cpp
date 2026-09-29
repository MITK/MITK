/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDenseLinearSystemSolver.h>
#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <vector>

namespace
{
  /** Uniform in [-1, 1]. The generator's output is mapped directly, as std::uniform_real_distribution
   *  differs between standard libraries, and every platform should test the same matrices.
   */
  Eigen::MatrixXd RandomMatrix(Eigen::Index rows, Eigen::Index cols, std::mt19937& random)
  {
    return Eigen::MatrixXd::NullaryExpr(rows, cols, [&]() { return 2.0 * random() / std::mt19937::max() - 1.0; });
  }

  /** The relative change of A and b that would make x exact. */
  double BackwardError(const Eigen::MatrixXd& A, const Eigen::VectorXd& x, const Eigen::VectorXd& b)
  {
    return (b - A * x).lpNorm<1>() / (A.cwiseAbs().colwise().sum().maxCoeff() * x.lpNorm<1>() + b.lpNorm<1>());
  }

  double RelativeDeviation(const Eigen::VectorXd& x, const Eigen::VectorXd& reference)
  {
    return (x - reference).norm() / reference.norm();
  }

  /** Solves A x = b through the single precision factorization and checks x against Eigen's double
   *  precision LU decomposition.
   */
  void CheckSolution(const Eigen::MatrixXd& A, const Eigen::VectorXd& b, double maxDeviation)
  {
    const Eigen::VectorXd x = mitk::SolveDenseLinearSystem(A, b);
    const Eigen::VectorXd reference = A.partialPivLu().solve(b);

    const auto n = A.rows();
    const double backwardError = BackwardError(A, x, b);
    const double deviation = RelativeDeviation(x, reference);

    std::ostringstream message;
    message << "n = " << n << ": backward error " << backwardError << " (reference " << BackwardError(A, reference, b)
            << "), deviation from the reference " << deviation;

    CPPUNIT_ASSERT_EQUAL_MESSAGE(message.str(), n, x.size());
    CPPUNIT_ASSERT_MESSAGE(message.str(), backwardError <= n * std::numeric_limits<double>::epsilon());
    CPPUNIT_ASSERT_MESSAGE(message.str(), deviation <= maxDeviation);

    // The fallback returns the reference bit for bit, which the refined solution of more than two
    // unknowns never does.
    if (2 < n)
      CPPUNIT_ASSERT_MESSAGE(message.str() + ", so the solver fell back to partialPivLu", 0.0 < deviation);
  }
}

class mitkDenseLinearSystemSolverTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDenseLinearSystemSolverTestSuite);
  MITK_TEST(TestRandomSystems);
  MITK_TEST(TestRadialBasisFunctionSystem);
  MITK_TEST(TestIllConditionedSystem);
  MITK_TEST(TestZeroRightHandSide);
  MITK_TEST(TestEmptySystem);
  MITK_TEST(TestMismatchedSizesThrow);
  CPPUNIT_TEST_SUITE_END();

public:
  /** Sizes around the panel width (128) and the tile width (256) of the factorization. */
  void TestRandomSystems()
  {
    std::mt19937 random(42);

    for (const Eigen::Index n : { 1, 2, 127, 128, 129, 385, 1000 })
    {
      const Eigen::MatrixXd A = RandomMatrix(n, n, random);
      const Eigen::VectorXd b = RandomMatrix(n, 1, random);
      CheckSolution(A, b, 1e-10);
    }
  }

  /** The indefinite system of the surface interpolation: phi(r) = r on points of contours in
   *  parallel planes and on points offset along their normals, with the offset as value.
   */
  void TestRadialBasisFunctionSystem()
  {
    constexpr int Contours = 3;
    constexpr int PointsPerContour = 100;
    constexpr double Offset = 0.5;
    const double pi = std::acos(-1.0);

    std::vector<Eigen::Vector3d> centers;
    std::vector<double> values;

    for (const double offset : { 0.0, -Offset, Offset })
    {
      for (int c = 0; c < Contours; ++c)
      {
        const double radius = 20.0 + 4.0 * c;

        for (int i = 0; i < PointsPerContour; ++i)
        {
          const double angle = 2.0 * pi * i / PointsPerContour;
          const Eigen::Vector3d normal(std::cos(angle), std::sin(angle), 0.0);
          centers.push_back(Eigen::Vector3d(0.0, 0.0, 5.0 * c) + (radius + offset) * normal);
          values.push_back(offset);
        }
      }
    }

    const auto n = static_cast<Eigen::Index>(centers.size());
    Eigen::MatrixXd A(n, n);

    for (Eigen::Index i = 0; i < n; ++i)
    {
      for (Eigen::Index j = 0; j < n; ++j)
        A(i, j) = (centers[i] - centers[j]).norm();
    }

    CheckSolution(A, Eigen::Map<const Eigen::VectorXd>(values.data(), n), 1e-8);
  }

  /** A condition number of 1e10 is out of reach for a single precision factorization, so the
   *  solver has to fall back to partialPivLu.
   */
  void TestIllConditionedSystem()
  {
    constexpr Eigen::Index n = 200;
    std::mt19937 random(7);

    const Eigen::MatrixXd U = RandomMatrix(n, n, random).householderQr().householderQ();
    const Eigen::MatrixXd V = RandomMatrix(n, n, random).householderQr().householderQ();
    const Eigen::VectorXd singularValues =
      Eigen::VectorXd::LinSpaced(n, 0.0, -10.0).unaryExpr([](double exponent) { return std::pow(10.0, exponent); });

    const Eigen::MatrixXd A = U * singularValues.asDiagonal() * V.transpose();
    const Eigen::VectorXd b = RandomMatrix(n, 1, random);
    const Eigen::VectorXd reference = A.partialPivLu().solve(b);

    CPPUNIT_ASSERT(reference == mitk::SolveDenseLinearSystem(A, b));
  }

  void TestZeroRightHandSide()
  {
    std::mt19937 random(3);
    const Eigen::VectorXd x = mitk::SolveDenseLinearSystem(RandomMatrix(300, 300, random), Eigen::VectorXd::Zero(300));

    CPPUNIT_ASSERT_EQUAL(Eigen::Index(300), x.size());
    CPPUNIT_ASSERT(x.isZero(0.0));
  }

  void TestEmptySystem()
  {
    CPPUNIT_ASSERT_EQUAL(Eigen::Index(0), mitk::SolveDenseLinearSystem(Eigen::MatrixXd(), Eigen::VectorXd()).size());
  }

  void TestMismatchedSizesThrow()
  {
    CPPUNIT_ASSERT_THROW(mitk::SolveDenseLinearSystem(Eigen::MatrixXd::Zero(3, 4), Eigen::VectorXd::Zero(3)), mitk::Exception);
    CPPUNIT_ASSERT_THROW(mitk::SolveDenseLinearSystem(Eigen::MatrixXd::Zero(3, 3), Eigen::VectorXd::Zero(4)), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDenseLinearSystemSolver)
