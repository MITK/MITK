/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkWeightedPointTransform_h
#define mitkWeightedPointTransform_h

// EXPORTS
#include "MitkAlgorithmsExtExports.h"

// ITK
#include <itkMatrix.h>
#include <itkVariableSizeMatrix.h>
#include <mitkCommon.h>
#include <mitkPointSet.h>
#include <vector>
#include <vtkSmartPointer.h>

// forward declarations
class vtkPoints;
class vtkLandmarkTransform;

namespace mitk
{
  /**
   * \ingroup AnisotropicRegistration
   *
   * @brief This class implements an extension of the
   * weighted point based registration algorithm
   * from A. Danilchenko, R. Balachandran and J. M. Fitzpatrick.
   *
   * The class implements an extension of the weighted point based registration
   * from A. Danilchenko et al.
   * presented by L. Maier-Hein et al. in "Convergent Iterative Closest-Point Algorithm
   * to Accomodate Anisotropic and Inhomogenous Localization Error.",
   * IEEE T Pattern Anal 34 (8), 1520-1532, 2012. The extension computes, in order
   * to ensure the convergence of the algorithm, an isotropic estimation
   * by an unweighted point based registration algorithm as an initial estimate.
   * The implemantion was originally ported to C/C++ by A. Franz.
   *
   * \note Some methods are accelerated when OpenMP is enabled.
   *
   */
  class MITKALGORITHMSEXT_EXPORT WeightedPointTransform : public itk::Object
  {
    /** Definition of a 3x3 matrix.*/
    typedef itk::Matrix<double, 3, 3> Matrix3x3;
    /** Definition of a 3x3 Weighting matrix.*/
    typedef Matrix3x3 WeightMatrix;
    /** Definition of a Rotation matrix.*/
    typedef Matrix3x3 Rotation;
    /** Definition of a translation vector.*/
    typedef itk::Vector<double, 3> Translation;
    /** Definition of a weight matrix list.*/
    typedef std::vector<WeightMatrix> WeightMatrixList;
    /** Definition of a covariance matrix list.*/
    typedef std::vector<Matrix3x3> CovarianceMatrixList;

  public:
    mitkClassMacroItkParent(WeightedPointTransform, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** @brief Method which registers both point sets. */
      void ComputeTransformation();

    /** @brief Sets the threshold of the registration. Default value is 0.0001.*/
    itkSetMacro(Threshold, double);

      /** @brief Sets the maximum number of iterations of the registration.
        * Default value is 1000.
        */
      itkSetMacro(MaxIterations, double);

      /** @return Returns the number of iterations of the last run
        * of the registration algorithm. Returns -1 if there was no
        * run of the registration yet.
        */
      itkGetMacro(Iterations, int);

    /** @return Returns the FRE of the last run of the registration algorithm.
      * Returns -1 if there was no run of the registration yet.
      */
    itkGetMacro(FRE, double);

    /** @brief Sets the FRE normalization factor. Default value is 1.0. */
    itkSetMacro(FRENormalizationFactor, double);

    /** @return Returns the current FRE normalization factor.*/
    itkGetMacro(FRENormalizationFactor, double);

    /** Sets the moving point set used for the registration.
      * @param p The input point set.
      */
    void SetMovingPointSet(vtkSmartPointer<vtkPoints> p);

    /**
     * Set the list of 3x3 covariance matrices belonging to the moving point set.
     * @param matrices List of covariance matrices.
     */
    void SetCovarianceMatricesMoving(const CovarianceMatrixList &matrices);

    /** Sets the fixed point set used for the registration.
      * @param p The input point set.
      */
    void SetFixedPointSet(vtkSmartPointer<vtkPoints> p);

    /**
     * Set the list of 3x3 covariance matrices belonging to the fixed point set.
     * @param matrices List of covariance matrices.
     */
    void SetCovarianceMatricesFixed(const CovarianceMatrixList &matrices);

    /**
      * The translation vector computed by the algorithm.
      * @return 3x1 translation vector.
      */
    const Translation &GetTransformT() const { return m_Translation; }
    /**
      * The rotation matrix computed by the algorithm.
      */
    const Rotation &GetTransformR() const { return m_Rotation; }
  protected:
    WeightedPointTransform();
    ~WeightedPointTransform() override;

    /** Threshold used to terminate the algorithm.*/
    double m_Threshold;

    /** Max allowed iterations used by the algorithm.*/
    int m_MaxIterations;

    /** The amount of iterations needed by the algorithm.*/
    int m_Iterations;

    /** The fiducial registration error (FRE) used in the algorithm.*/
    double m_FRE;

    /** Normalization factor for the FRE.*/
    double m_FRENormalizationFactor;

    /** Isotropic point based registration used for initial estimate.*/
    vtkSmartPointer<vtkLandmarkTransform> m_LandmarkTransform;

    /** The fixed point set (Y).*/
    vtkSmartPointer<vtkPoints> m_FixedPointSet;

    /** Moving point set (X).*/
    vtkSmartPointer<vtkPoints> m_MovingPointSet;

    /** Covariance matrices of the moving point set (Sigma_X).*/
    CovarianceMatrixList m_CovarianceMatricesMoving;

    /** Covariance matrices of the moving point set (Sigma_Y).*/
    CovarianceMatrixList m_CovarianceMatricesFixed;

    /** 3x1 translation vector.*/
    Translation m_Translation;

    /** 3x3 rotation matrix.*/
    Rotation m_Rotation;

    /**
     *  original matlab-function:
     *
     *  Constructs the C matrix of the linear version of the registration
     *  problem, Cq = e, where q = [delta_angle(1:3),delta_translation(1:3)] and
     *  e is produced by e_maker(X,Y,W)
     *
     *  Authors: JM Fitzpatrick and R Balachandran
     *  Creation: February 2009
     *
     *  --------------------------------------------
     *
     *  converted to C++ by Alfred Franz in March/April 2010
     */
    void C_maker(vtkPoints *X, const WeightMatrixList &W, itk::VariableSizeMatrix<double> &returnValue);

    /**
     *  original matlab-function:
     *
     *  Constructs the e vector of the linear version of the registration
     *  problem, Cq = e, where q = [delta_angle(1:3),delta_translation(1:3)] and
     *  C is produced by C_maker(X,W)
     *
     *  Authors: JM Fitzpatrick and R Balachandran
     *  Creation: February 2009
     *
     *  --------------------------------------------
     *
     *  converted to C++ by Alfred Franz in March/April 2010
     */
    void E_maker(vtkPoints *X, vtkPoints *Y, const WeightMatrixList &W, vnl_vector<double> &returnValue);

    /**
      * This method computes the change in a root mean squared
      * sense between the previous and the actual iteration.
      * The computed value is used as a termination constraint of the algorithm and
      * compared against the threshold.
      *
      * @param X The moving point set in the previous iteration step.
      * @param X_new The moving point set in the actual step.
      *
      * @return The computed change between the two point sets.
      */
    double CalculateConfigChange(vtkPoints *X, vtkPoints *X_new);

    /**
     * @brief This method performs a variant of the weighted point register algorithm presented by
     *        A. Danilchenko, R. Balachandran and J. M. Fitzpatrick in January 2010. (Modified in January 2011)
     *        converted to C++ by Alfred Franz in March/April 2010
     *
     * @param X                (input) the moving point set
     * @param Y                (input) the fixed (static) point set
     * @param Sigma_X          (input) a 3-by-3-by-N array, each page containing the weighting matrix for the Nth pair
     * of points in X
     * @param Sigma_Y          (input) a 3-by-3-by-N array, each page containing the weighting matrix for the Nth pair
     * of points in Y
     * @param Threshold        (input) the relative size of the change to the moving set above which the iteration
     * continues
     * @param MaxIterations    (input) the maximum number of iterations allowed
     * @param TransformationR  (output) this variable will hold the computed rotation matrix
     * @param TransformationT  (output) this variable will hold the computed translation vector
     * @param FRE              (output) this variable will hold the computed rotation FRE of the transformation
     * @param n                (output) this variable will hold the number of iterations used by the algorithm
     */
    void WeightedPointRegister(vtkPoints *X,
                               vtkPoints *Y,
                               const CovarianceMatrixList &Sigma_X,
                               const CovarianceMatrixList &Sigma_Y,
                               double Threshold,
                               int MaxIterations,
                               Rotation &TransformationR,
                               Translation &TransformationT,
                               double &FRE,
                               int &n);
  };
}
#endif
