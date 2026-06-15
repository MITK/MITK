/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCovarianceMatrixCalculator_h
#define mitkCovarianceMatrixCalculator_h

// exports
#include <MitkAlgorithmsExtExports.h>

#include <mitkCommon.h>

#include <itkMatrix.h>
#include <itkObjectFactory.h>
#include <vector>

namespace mitk
{
  // forward declarations
  class Surface;
  struct CovarianceMatrixCalculatorData;

  /**
   * \brief Computes covariance matrices for every vertex of a Surface,
   * for use in the anisotropic ICP (A-ICP) algorithm.
   *
   * This class computes a 3x3 covariance matrix for each vertex in a given
   * Surface based on its direct neighbours and stores them in a CovarianceMatrixList.
   * The implementation follows the CM_PCA method presented by
   * L. Maier-Hein et al. in "Convergent Iterative Closest-Point Algorithm
   * to Accommodate Anisotropic and Inhomogenous Localization Error.",
   * IEEE T Pattern Anal 34 (8), 1520-1532, 2012. The algorithm requires
   * a clean Surface without non-manifold edges and without duplicated vertices.
   * Use vtkCleanPolyData to ensure a correct Surface representation.
   *
   * \sa AnisotropicIterativeClosestPointRegistration
   * \sa AnisotropicRegistrationCommon
   * \sa WeightedPointTransform
   */
  class MITKALGORITHMSEXT_EXPORT CovarianceMatrixCalculator : public itk::Object
  {
  private:
    /** Pimpl to hold private data.*/
    CovarianceMatrixCalculatorData *d;

  protected:
    // local typedefs

    /** Definition of the covariance matrix.*/
    typedef itk::Matrix<double, 3, 3> CovarianceMatrix;
    /** Definition of a list of covariance matrices */
    typedef std::vector<CovarianceMatrix> CovarianceMatrixList;
    typedef double Vertex[3];

    /** List that stores the computed covariance matrices. */
    CovarianceMatrixList m_CovarianceMatrixList;

    /** This method projects all surrounding vertices of given vertex in a Surface
      * in the normal direction onto a plane and computes a primary component
      * analysis on the projected vertices. In the next step a orthonormal
      * system is created.
      *
      * \param index The index of the input Vertex in the Surface.
      * \param normal The normal of the input Vertex.
      * \param principalComponents CovarianceMatrix of the principal component analysis.
      * \param variances Variances along the axes of the createt Orthonormal system.
      * \param curVertex The current Vertex in the surface
      *
      */
    void ComputeOrthonormalCoordinateSystem(
      const int index, Vertex normal, CovarianceMatrix &principalComponents, Vertex variances, Vertex curVertex);
    CovarianceMatrixCalculator();
    ~CovarianceMatrixCalculator() override;

  public:
    mitkClassMacroItkParent(CovarianceMatrixCalculator, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** Sets the scaling factor for the voronoi area.
      *  \param factor The scaling factor.
      */
    void SetVoronoiScalingFator(const double factor);

    /** Enables/disables the covariance matrix normalization.
      * \param state Enables the covariance matrix normalization.
      */
    void EnableNormalization(bool state);

    /** Returns the mean of variance of all computed covariance matrices.
      * \return The mean variance.
      */
    double GetMeanVariance() const;

    /** Returns a reference to the CovarianceMatrixList with the computed covariance matrices.
      * \return A CovarianceMatrixList.
      */
    const CovarianceMatrixList &GetCovarianceMatrices() const;

    /** Sets the input {@link Surface} for which the covariance matrices will be calculated.
      * \param input A {@link Surface}.
      */
    void SetInputSurface(Surface *input);

    /** Method that computes the covariance matrices for the input surface.
      * \throws std::exception If the input surface is not set.
      */
    void ComputeCovarianceMatrices();
  };
}

#endif
