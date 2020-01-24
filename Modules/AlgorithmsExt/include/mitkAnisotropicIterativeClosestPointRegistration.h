/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef __ANISOTROPICITERATIVECLOSESTPOINTREGISTRATION_H__
#define __ANISOTROPICITERATIVECLOSESTPOINTREGISTRATION_H__

// MITK
#include <mitkCommon.h>
#include <mitkVector.h>

// EXPORTS
#include "MitkAlgorithmsExtExports.h"

// STL
#include <vector>

// ITK
#include <itkMatrix.h>

// forward declarations
class vtkPoints;
class vtkKdTreePointLocator;

namespace mitk
{
  class Surface;
  class WeightedPointTransform;

  /**
    * \ingroup AnisotropicRegistration
    *
    * @brief Implementation of the anisotropic iterative closest point (A-ICP)
    * algoritm.
    *
    * This class implements the anisotropic interative closest point (A-ICP)
    * algorithm presented in L. Maier-Hein et al. in "Convergent Iterative
    * Closest-Point Algorithm to Accomodate Anisotropic and Inhomogenous
    * Localization Error.", IEEE T Pattern Anal 34 (8), 1520-1532, 2012.
    * The algorithm computes the optimal transformation to align two surfaces.
    * In addition to the surfaces a list of covariance matrices is used as
    * input for every surface. Each covariance matrix represents the error of
    * a specific vertex in the Surface. The covariance matrices
    * for each surface can be defined by the user, or calculated
    * by the CovarianceMatrixCalculator. In addition a trimmed algorithm version
    * is provided to compute the registration of partial overlapping surfaces.
    * The algorithm needs a clean surface non manifold edges and without duplicated
    * vertices. In addition vtkCleanPolyData can be used to ensure a correct
    * Surface representation.
    *
    * \note The correspondence search is accelerated when OpenMP is enabled.
    *
    * \b Example:
    *
    *
    * \code
    * typedef itk::Matrix < double, 3, 3 > Matrix3x3;
    * typedef itk::Vector < double, 3 > Vector3;
    * typedef std::vector < Matrix3x3 > CovarianceMatrixList;
    *
    * // compute the covariance matrices
    * mitk::CovarianceMatrixCalculator::Pointer matrixCalculator =
    *                                    mitk::CovarianceMatrixCalculator::New();
    *
    * // compute the covariance matrices for the moving surface (X)
    * matrixCalculator->SetInputSurface(movingSurface);
    * matrixCalculator->ComputeCovarianceMatrices();
    * CovarianceMatrixList sigmas_X = matrixCalculator->GetCovarianceMatrices();
    * double meanVarX = matrixCalculator->GetMeanVariance();
    *
    * // compute the covariance matrices for the fixed surface (Y)
    * matrixCalculator->SetInputSurface(fixedSurface);
    * matrixCalculator->ComputeCovarianceMatrices();
    * CovarianceMatrixList sigmas_Y = matrixCalculator->GetCovarianceMatrices();
    * double meanVarY = matrixCalculator->GetMeanVariance();
    *
    * // the FRE normalization factor
    * double normalizationFactor = sqrt( meanVarX + meanVarY);
    *
    * // A-ICP algorithm
    * mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
    *    mitk::AnisotropicIterativeClosestPointRegistration::New();
    *
    * // set up parameters
    * aICP->SetMovingSurface(movingSurface);
    * aICP->SetFixedSurface(fixedSurface);
    * aICP->SetCovarianceMatricesMovingSurface(sigmas_X);
    * aICP->SetCovarianceMatricesFixedSurface(sigmas_Y);
    * aICP->SetFRENormalizationFactor(normalizationFactor);
    *
    * // Trimming is enabled if a fator > 0.0 is set.
    * // 40 percent of the moving point set
    * // will be used for registration in this example.
    * // To disable trimming set the trim factor back to 0.0
    * aICP->SetTrimmFactor(0.4);
    *
    * // run the algorithm
    * aICP->Update();
    *
    * // retrieve the computed transformation
    * Matrix3x3 rotation = aICP->GetRotation();
    * Vector3 translation = aICP->GetTranslation();
    * \endcode
    *
    */
  class MITKALGORITHMSEXT_EXPORT AnisotropicIterativeClosestPointRegistration : public itk::Object
  {
  protected:
    /** Definition of a 3x3 covariance matrix.*/
    typedef itk::Matrix<double, 3, 3> CovarianceMatrix;
    /** Definition of a list of covariance matrices.*/
    typedef std::vector<CovarianceMatrix> CovarianceMatrixList;
    /** Definition of a translation vector.*/
    typedef mitk::Vector3D Translation;
    /** Definition of a 3x3 rotation matrix.*/
    typedef CovarianceMatrix Rotation;
    /** Definition of a correspondeces, index and distance.*/
    typedef std::pair<unsigned int, double> Correspondence;
    /** Definition of a list of correspondences.*/
    typedef std::vector<Correspondence> CorrespondenceList;

    AnisotropicIterativeClosestPointRegistration();
    ~AnisotropicIterativeClosestPointRegistration() override;

    /** Max amount of iterations. Default is 1000.*/
    unsigned int m_MaxIterations;

    /** Threshold used for termination. Default is 1.0e-6.*/
    double m_Threshold;

    /** Normalization factor for the feducial registration error. default is 0.0.*/
    double m_FRENormalizationFactor;

    /** Search radius for the correspondence search. Default is 30.*/
    double m_SearchRadius;

    /** The maximum number of iterations in the weighted point based
      * registration. Default is 1000.
      */
    double m_MaxIterationsInWeightedPointTransform;

    /** The fiducial registration error (FRE).*/
    double m_FRE;

    /** Trimmfactor for partial overlapping registration. Default is 0.*/
    double m_TrimmFactor;

    /** Amount of iterations used by the algorithm.*/
    unsigned int m_NumberOfIterations;

    /** Moving surface that is transformed on the fixed surface.*/
    itk::SmartPointer<Surface> m_MovingSurface;
    /** The fixed / target surface.*/
    itk::SmartPointer<Surface> m_FixedSurface;

    /** The weighted point based registration algorithm.*/
    itk::SmartPointer<WeightedPointTransform> m_WeightedPointTransform;

    /** The covariance matrices belonging to the moving surface (X).*/
    CovarianceMatrixList m_CovarianceMatricesMovingSurface;

    /** The covariance matrices belonging to the moving surface (Y).*/
    CovarianceMatrixList m_CovarianceMatricesFixedSurface;

    /** The computed 3x1 translation vector.*/
    Translation m_Translation;
    /** The computed 3x3 rotation matrix.*/
    Rotation m_Rotation;

    /**
      * Method that computes the correspondences between the moving point set X
      * and the fixed point set Y. The distances between the points
      * are weighted with weight matrices that are computed from the covariances
      * along the surfaces axes. This method implements the runtime optimization
      * presented by L. Maier-Hein et al.. The correspondences are computed with
      * the help of a kd tree. The correspondences are searched in a given radius
      * in the euklidian space. Every correspondence found in this radius is
      * weighted based on the covariance matrices and the best weighting will be
      * used as a correspondence.
      *
      * @param X The moving point set.
      * @param Z The returned correspondences from the fixed point set.
      * @param Y The fixed point set saved in a kd tree.
      * @param sigma_X Covariance matrices belonging to the moving point set.
      * @param sigma_Y Covariance matrices belonging to the fixed point set.
      * @param sigma_Z Covariance matrices belonging to the correspondences found.
      * @param correspondences Saved correspondences, in a pair containing the
      *        their index in Y and distance.
      * @param radius The search radius used in in kd tree.
      *
      */
    void ComputeCorrespondences(vtkPoints *X,
                                vtkPoints *Z,
                                vtkKdTreePointLocator *Y,
                                const CovarianceMatrixList &sigma_X,
                                const CovarianceMatrixList &sigma_Y,
                                CovarianceMatrixList &sigma_Z,
                                CorrespondenceList &correspondences,
                                const double radius);

  public:
    mitkClassMacroItkParent(AnisotropicIterativeClosestPointRegistration, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Set the maximum amount of iterations used by the algorithm. */
      itkSetMacro(MaxIterations, unsigned int);

      /** Set the threshold used to terminate the algorithm.*/
      itkSetMacro(Threshold, double);

      /** Set the normalization factor for the fiducial registration error (FRE).
       *  The normalization factor is computed with the help of the mean variance
       *  of both CovarianceMatrixList that can be obtained when the covariance
       *  matrices are calculated with the CovarianceMatrixCalculator:
       *
       *  \code{.cpp}
       *  double FRENormalizationFactor = sqrt ( MeanVarianceX + MeanVarianceY );
       *  \endcode
       *
       *  if no FRE normalization is used the normalization factor is set to 1.0
       *  as default value.
       */
      itkSetMacro(FRENormalizationFactor, double);

      /** Set search radius for the correspondence search.*/
      itkSetMacro(SearchRadius, double);

      /** Set the maximim number of iterations used by the point based registration
        * algorithm.
        */
      itkSetMacro(MaxIterationsInWeightedPointTransform, double);

      /** Get the fiducial registration error (FRE).*/
      itkGetMacro(FRE, double);

      /** Get the number of iterations used by the algorithm.*/
      itkGetMacro(NumberOfIterations, unsigned int);

      /**
        * Factor that trimms the point set in percent for
        * partial overlapping surfaces. E.g. 0.4 will use 40 precent
        * of the point set. To enable the trimmed version a
        * factor > 0 and < 1 must be set. The default value is 0.0.
        */
      itkSetMacro(TrimmFactor, double);

      /**
        * Set moving surface that includes the point set (X).
        */
      itkSetMacro(MovingSurface, itk::SmartPointer<Surface>);

      /**
        * Set fixed surface that includes the point set (Y).
        */
      itkSetMacro(FixedSurface, itk::SmartPointer<Surface>);

      /**
        * Returns the 3x1 translation vector computed by the algorithm.
        */
      itkGetConstReferenceMacro(Translation, Translation);

      /**
        * Returns the 3x3 rotation matrix computed by the algorithm.
        */
      itkGetConstReferenceMacro(Rotation, Rotation);

      /**
        * Set the covariance matrices of the moving surface. The algorithm
        * need the same amount of covariance and points available in the surface.
        * The covariance matrix for every vertex in a Surface can be calculated by
        * the CovarianceMatrixCalculator. It is also possible to define
        * arbitrary matrices by hand.
        */
      void SetCovarianceMatricesMovingSurface(CovarianceMatrixList &list)
    {
      m_CovarianceMatricesMovingSurface = list;
    }

    /**
      * Set the covariance matrices of the fixed surface. The algorithm
      * need the same amount of covariance and points available in the surface.
      * The covariance matrix for every vertex in a Surface can be calculated by
      * the CovarianceMatrixCalculator. It is also possible to define
      * arbitrary matrices by hand.
      */
    void SetCovarianceMatricesFixedSurface(CovarianceMatrixList &list) { m_CovarianceMatricesFixedSurface = list; }
    /**
      * This method executes the algorithm.
      *
      * @warning The algorithm is only a simple calculation filter and can not be
      * used in a mitk filter pipline.
      *
      * @throws Exception if the search radius was doubled more than 20 times to
      * prevent endless loops. Re-run the with a different search radius that
      * will find the correspondences.
      */
    void Update();
  };
}
#endif
