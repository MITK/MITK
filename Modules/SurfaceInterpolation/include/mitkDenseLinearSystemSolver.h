/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDenseLinearSystemSolver_h
#define mitkDenseLinearSystemSolver_h

#include <MitkSurfaceInterpolationExports.h>

#include <itkeigen/Eigen/Dense>

namespace mitk
{
  /** \brief Solves the dense linear system A x = b for a square, possibly indefinite matrix A.
   *
   * x is backward stable in double precision: its normwise backward error, the relative change of
   * A and b that would make it exact, is at most n times the double machine epsilon. That is the
   * bound of a double precision LU decomposition with partial pivoting (Eigen's partialPivLu),
   * though x may differ from partialPivLu's solution within it.
   *
   * Runs on ITK's thread pool.
   *
   * \throw mitk::Exception if A is not square or the size of b does not match.
   */
  MITKSURFACEINTERPOLATION_EXPORT Eigen::VectorXd SolveDenseLinearSystem(const Eigen::MatrixXd& A, const Eigen::VectorXd& b);
}

#endif
