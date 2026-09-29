/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDenseLinearSystemSolver.h>

#include <mitkExceptionMacro.h>

#include <itkMultiThreaderBase.h>

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

namespace
{
  // The panel is factorized on one thread, so it is kept narrow. Narrow tiles keep all threads
  // busy until the trailing matrix gets small. Narrower still would be faster in theory, but
  // every panel costs a round trip through the thread pool.
  constexpr Eigen::Index PanelWidth = 64;
  constexpr Eigen::Index TileWidth = 64;

  constexpr int MaxRefinementSteps = 10;

  itk::SizeValueType TilesOf(Eigen::Index columns)
  {
    return static_cast<itk::SizeValueType>((columns + TileWidth - 1) / TileWidth);
  }

  /** Right-looking blocked LU decomposition with partial pivoting in place, as LAPACK's getrf does it.
   *  Row j was swapped with row pivots[j] in step j.
   */
  void FactorizeLU(Eigen::MatrixXf& A, std::vector<Eigen::Index>& pivots, itk::MultiThreaderBase* threader)
  {
    const Eigen::Index n = A.rows();
    pivots.resize(n);

    for (Eigen::Index k = 0; k < n; k += PanelWidth)
    {
      const Eigen::Index width = std::min(PanelWidth, n - k);

      // Rows are swapped within the panel only: in a column-major matrix, a whole row is spread
      // over all columns. The other columns get the swaps of the panel below, column by column.
      for (Eigen::Index j = k; j < k + width; ++j)
      {
        Eigen::Index pivot = 0;
        A.col(j).tail(n - j).cwiseAbs().maxCoeff(&pivot);
        pivot += j;
        pivots[j] = pivot;

        if (pivot != j)
          A.block(j, k, 1, width).swap(A.block(pivot, k, 1, width));

        A.col(j).tail(n - j - 1) /= A(j, j);
        const Eigen::Index rest = k + width - j - 1;

        if (0 < rest)
          A.block(j + 1, j + 1, n - j - 1, rest).noalias() -= A.col(j).tail(n - j - 1) * A.row(j).segment(j + 1, rest);
      }

      const auto swapRows = [&](Eigen::Index firstColumn, Eigen::Index columns)
        {
          for (Eigen::Index c = firstColumn; c < firstColumn + columns; ++c)
          {
            auto column = A.col(c);

            for (Eigen::Index j = k; j < k + width; ++j)
            {
              if (pivots[j] != j)
                std::swap(column[j], column[pivots[j]]);
            }
          }
        };

      const Eigen::Index trailing = n - k - width;
      const auto leftTiles = TilesOf(k);
      const auto trailingTiles = TilesOf(trailing);

      if (0 == leftTiles + trailingTiles)
        break;

      threader->ParallelizeArray(0, leftTiles + trailingTiles, [&](itk::SizeValueType tile)
        {
          if (tile < leftTiles)
          {
            const Eigen::Index firstColumn = static_cast<Eigen::Index>(tile) * TileWidth;
            swapRows(firstColumn, std::min(TileWidth, k - firstColumn));
            return;
          }

          const Eigen::Index firstColumn = k + width + static_cast<Eigen::Index>(tile - leftTiles) * TileWidth;
          const Eigen::Index columns = std::min(TileWidth, n - firstColumn);
          swapRows(firstColumn, columns);

          auto upper = A.block(k, firstColumn, width, columns);
          A.block(k, k, width, width).triangularView<Eigen::UnitLower>().solveInPlace(upper);
          A.block(k + width, firstColumn, trailing, columns).noalias() -= A.block(k + width, k, trailing, width) * upper;
        }, nullptr);
    }
  }

  Eigen::VectorXf SolveLU(const Eigen::MatrixXf& lu, const std::vector<Eigen::Index>& pivots, Eigen::VectorXf x)
  {
    for (Eigen::Index j = 0; j < lu.rows(); ++j)
      std::swap(x[j], x[pivots[j]]);

    lu.triangularView<Eigen::UnitLower>().solveInPlace(x);
    lu.triangularView<Eigen::Upper>().solveInPlace(x);
    return x;
  }
}

Eigen::VectorXd mitk::SolveDenseLinearSystem(const Eigen::MatrixXd& A, const Eigen::VectorXd& b)
{
  if (A.rows() != A.cols() || A.rows() != b.size())
  {
    mitkThrow() << "Cannot solve a linear system with a " << A.rows() << " x " << A.cols()
                << " matrix and a right-hand side of size " << b.size() << ".";
  }

  const Eigen::Index n = A.rows();

  if (0 == n)
    return Eigen::VectorXd();

  // A is factorized in single precision and the solution is refined in double precision. If A is
  // too ill-conditioned for the refinement to reach the backward error of a double precision
  // solver, partialPivLu solves the system instead.
  auto threader = itk::MultiThreaderBase::New();

  // One pass over A: the copy to factorize, and the column sums for its norm.
  Eigen::MatrixXf lu(n, n);
  Eigen::VectorXd columnSums(n);

  threader->ParallelizeArray(0, static_cast<itk::SizeValueType>(n), [&](itk::SizeValueType column)
    {
      const auto j = static_cast<Eigen::Index>(column);
      lu.col(j) = A.col(j).cast<float>();
      columnSums[j] = A.col(j).cwiseAbs().sum();
    }, nullptr);

  std::vector<Eigen::Index> pivots;
  FactorizeLU(lu, pivots, threader);

  const auto solve = [&](const Eigen::VectorXd& rhs)
    {
      return Eigen::VectorXd(SolveLU(lu, pivots, rhs.cast<float>()).cast<double>());
    };

  Eigen::VectorXd x = solve(b);
  Eigen::VectorXd residual = b - A * x;
  double residualNorm = residual.lpNorm<1>();

  // Each step shrinks the error by about the condition number of A times the float precision,
  // until the residual reaches what double precision can resolve and stops getting smaller.
  // A NaN residual (singular or overflowing float factorization) skips the refinement.
  for (int step = 0; step < MaxRefinementSteps && 0.0 < residualNorm; ++step)
  {
    const Eigen::VectorXd refined = x + solve(residual);
    Eigen::VectorXd refinedResidual = b - A * refined;
    const double refinedNorm = refinedResidual.lpNorm<1>();

    if (!(refinedNorm < residualNorm))
      break;

    const bool halved = refinedNorm < 0.5 * residualNorm;

    x = refined;
    residual = std::move(refinedResidual);
    residualNorm = refinedNorm;

    if (!halved)
      break;
  }

  // Normwise backward error: the relative change of A and b that would make x exact. For a
  // backward stable double precision solver it stays below about n times the machine epsilon.
  const double normA = columnSums.maxCoeff();
  const double tolerance = static_cast<double>(n) * std::numeric_limits<double>::epsilon();

  if (residualNorm <= tolerance * (normA * x.lpNorm<1>() + b.lpNorm<1>()))
    return x;

  // The fallback makes a double precision copy of A of its own.
  lu.resize(0, 0);

  return A.partialPivLu().solve(b);
}
