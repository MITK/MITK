/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef __MITK_COVARIANCEMATRIXCALCULATOR_H__
#define __MITK_COVARIANCEMATRIXCALCULATOR_H__

// exports
#include <MitkSurfaceRegistrationExports.h>

#include <mitkCommon.h>

#include <itkObjectFactory.h>
#include <vector>
#include <itkMatrix.h>

namespace mitk
{

class Surface;
struct CovarianceMatrixCalculatorData;

/**
 * \ingroup AnisotropicRegistration
 *
 * @brief Class that computes the covariance matrices for every point in a
 * {@link Surface} used in the A-ICP algorithm.
 *
 * Computes a covariance matrix for every vertex in a given {@link Surface}
 * based on it's direct neighbours and saves them in a CovarianceMatrixList.
 * The Class implements the CM_PCA method presented by
 * L. Maier-Hein et al. in "Convergent Iterative Closest-Point Algorithm
 * to Accomodate Anisotropic and Inhomogenous Localization Error.",
 * IEEE T Pattern Anal 34 (8), 1520-1532, 2012.
 */
class MitkSurfaceRegistration_EXPORT CovarianceMatrixCalculator : public itk::Object
{

protected:

  // local typedefs

  /** Definition of the covariance matrix.*/
  typedef itk::Matrix<double,3,3> CovarianceMatrix;
  /** Definition of a list of covariance matrices */
  typedef std::vector<CovarianceMatrix> CovarianceMatrixList;
  typedef double Vertex[3];

  CovarianceMatrixCalculatorData* d;

  /** List that stores the computed covariance matrices. */
  CovarianceMatrixList m_CovarianceMatrixList;

  void ComputeOrthonormalCoordinateSystem( const int index,
                                           Vertex normal,
                                           CovarianceMatrix &principalComponents,
                                           Vertex variances,
                                           Vertex curVertex);
  CovarianceMatrixCalculator();
  ~CovarianceMatrixCalculator();


public:
  mitkClassMacro(CovarianceMatrixCalculator, itk::Object)
  itkNewMacro (Self)

  /** Sets the scaling factor for the voronoi area.
   *  @param factor The scaling factor.
   */
  void SetVoronoiScalingFator( const double factor );

  /** Enables/disables the covariance matrix normalization.
    * @param state Enables the covariance matrix normalization.
    */
  void EnableNormalization( bool state );

  /** Returns the mean of variance of all computed covariance matrices.
    * @return The mean variance.
    */
  double GetMeanVariance() const;

  /** Returns a reference to the CovarianceMatrixList with the computed covariance matrices.
    * @return A CovarianceMatrixList.
    */
  const CovarianceMatrixList& GetCovarianceMatrices() const;

  /** Sets the input {@link Surface} for which the covariance matrices will be calculated.
    * @param input A {@link Surface}.
    */
  void SetInputSurface (Surface *input);


  /** Method that compites the covariance matrices for the input surface.
    * @throws std::exception If the input surface is not set.
    */
  void ComputeCovarianceMatrices();

};

}

#endif
