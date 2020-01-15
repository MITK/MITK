/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __ANISOTROPICREGISTRATIONCOMMON_H__
#define __ANISOTROPICREGISTRATIONCOMMON_H__

#include <itkMatrix.h>
#include <mitkCommon.h>
#include <mitkVector.h>

#include "MitkAlgorithmsExtExports.h"

// forward declarations
class vtkPoints;

namespace mitk
{
  class PointSet;

  /**
   * \ingroup AnisotropicRegistration
   * \brief A Class that provides common static functions used by all classes
   *  and tests in the anisotropic iterative closest point algorithm
   *  (AnisotropicIterativeClosestPointRegistration).
   *
   * The class provides common functionality used by the A-ICP algorithm like:
   * compute weightmatrices (@ref CalculateWeightMatrix()),
   * transform points (@ref TransformPoints()), propagate 3 x 3 matrices
   * (@ref PropagateMatrices()) and compute the target registration error (TRE)
   * (@ref ComputeTargetRegistrationError()).
   */
  class MITKALGORITHMSEXT_EXPORT AnisotropicRegistrationCommon
  {
  protected:
    // local typedefs

    /** Definition of the 3 x 3 weight matrix.*/
    typedef itk::Matrix<double, 3, 3> WeightMatrix;
    /** Definition of a rotation matrix.*/
    typedef WeightMatrix Rotation;
    /** Definition of the covariance matrix.*/
    typedef WeightMatrix CovarianceMatrix;
    /** Definition of the translation vector.*/
    typedef mitk::Vector3D Translation;
    /** Definition of the weight matrix list.*/
    typedef std::vector<WeightMatrix> MatrixList;

    AnisotropicRegistrationCommon() {}
    ~AnisotropicRegistrationCommon() {}
  public:
    /** @brief Method that computes a WeightMatrix with two CovarianceMatrices.
     *  @param sigma_X CovarianceMatrix from the moving point set.
     *  @param sigma_Y CovarianceMatrix from the fixed point set.
     *  @return The computed WeighMatrix.
     */
    static WeightMatrix CalculateWeightMatrix(const CovarianceMatrix &sigma_X, const CovarianceMatrix &sigma_Y);

    /**
     * @brief Transforms a point cloud with a Rotation and Translation.
     *
     * The method uses two point sets as input. It transforms every point from the
     * source point set and saves the result in the destination. The soure is not
     * modified. If the same point set is used as source and destination. The method
     * will modify the source and the transformation is done in place.
     *
     * @warning No bound check is done. Ensure that source and destination are
     * allocated and have the same size.
     *
     * @param src The source point set.
     * @param dst The destination point set.
     * @param rotation The rotation matrix.
     * @param translation The translation vector.
     *
     */
    static void TransformPoints(vtkPoints *src,
                                vtkPoints *dst,
                                const Rotation &rotation,
                                const Translation &translation);

    /**
     * @brief Propagate a list of matrices with a rotation matrix.
     *
     * Method that propagate the source list and saves the result in the destination.
     * If the source and destination lists are the same the matrices will be computed
     * in place.
     *
     * @warning No bound check is done. Make sure that both lists are allocated and
     * have the same size.
     *
     * @param src Reference to the source matrices list.
     * @param dst Reference to the destination list
     * @param rotation Reference to a rotation matrix.
     */
    static void PropagateMatrices(const MatrixList &src, MatrixList &dst, const Rotation &rotation);

    /**
     * @brief Compute the target registration error between two point sets.
     *
     * Method that is used for testing and evaluation. It computes the target
     * registration error (TRE) between two point sets with a rotation matrix and
     * a translation vector.
     *
     * @param movingTargets The target points of the moving point set.
     * @param fixedTargets The target points of the fixed point set.
     * @param rotation A 3x3 rotation matrix.
     * @param translation A 3x1 translation vector.
     *
     * @return The Target Registration Error (TRE).
     */
    static double ComputeTargetRegistrationError(const mitk::PointSet *movingTargets,
                                                 const mitk::PointSet *fixedTargets,
                                                 const Rotation &rotation,
                                                 const Translation &translation);
  };
}

#endif
