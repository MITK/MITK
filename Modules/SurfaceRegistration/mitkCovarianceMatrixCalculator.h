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
 * Computes covariance matrices for every vertex in a given {@link Surface} based on it's
 * direct neighbours. Three different computation are available:
 * <ul>
 * <li> <code>CM_PCA</code> computes a covariance matrix based a principal component analysis
 * of the adjacent vertices.
 * <li> <code>CM_VORONOI</code> gets a covariance matrix based on voronoi regions.
 * <li> <code>CM_TOF</code> a covariance matrix that represents the error in the viewing
 * of a time of flight camera.
 * </ul>
 */
class MitkSurfaceRegistration_EXPORT CovarianceMatrixCalculator : public itk::Object
{

public:
  enum CalculationMethod
  {
    CM_PCA = 1,
    CM_VORONOI = 2,
    CM_TOF = 3
  };

protected:

  // local typedefs
  typedef itk::Matrix<double,3,3> CovarianceMatrix;
  typedef std::vector<CovarianceMatrix> CovarianceMatrixList;
  typedef double Vertex[3];

  CovarianceMatrixCalculatorData* d;

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

  /** Sets the covariance matrix calculation method.
    * Available methods are <code>CM_PCA</code>, <code>CM_VORONOI</code>, <code>CM_TOF</code>.
    * @param method The computation method.
    */
  void SetCalculationMethod ( CalculationMethod method );

  /** Returns the mean of variance of all computed covariance matrices.
    * @return The mean variance.
    */
  double GetMeanVariance() const;

  /** Returns a reference to the CovarianceMatrixList with the computed covariance matrices.
    * @return A <code>CovarianceMatrixList></code>.
    */
  const CovarianceMatrixList& GetCovarianceMatrices() const;

  /** Sets the input {@link Surface} for which the covariance matrices will be calculated.
    * @param input A {@link Surface}.
    */
  void SetInputSurface (Surface *input);


  /** Calculates covariance matrices.
    * @throws std::exception If the input surface is not set.
    */
  void ComputeCovarianceMatrices();

};

}

#endif
