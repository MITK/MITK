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

#ifndef ANISOTROPICPOINTREGISTRATIONINTERFACE_H_
#define ANISOTROPICPOINTREGISTRATIONINTERFACE_H_


//EXPORTS
#include "MitkSurfaceRegistrationExports.h"

//ITK
#include <itkVariableSizeMatrix.h>
#include <mitkCommon.h>
#include <itkMatrix.h>
#include <vector>
#include <mitkPointSet.h>
#include <vtkSmartPointer.h>

class vtkPoints;

namespace mitk
{

class WeightedPointTransformData;

    /** @brief TODO */
class MitkSurfaceRegistration_EXPORT WeightedPointTransform : public itk::Object
{
public:

    mitkClassMacro(WeightedPointTransform, itk::Object);
    itkNewMacro(Self);


    /** @brief Method which registers both point sets. */
    virtual bool Update();

    /** @brief Covariance Matrices of both point sets. This matrices must be set before registration is started.
     *         The vectors must have the same size like the point sets!
     */
    void SetCovarianceMatrices(std::vector< itk::Matrix<double,3,3> > CovarianceMatricesM,std::vector< itk::Matrix<double,3,3> > CovarianceMatricesS);

    /** @brief Sets the threshold of the registration. Default value is 0.0001.*/
    void SetThreshold(double threshold);

    /** @brief Sets the maximum number of iterations of the registration. Default value is 1000.*/
    void SetMaxIterations(double value);

    /** @return Returns the number of iterations of the last run of the registration algorithm. Returns -1 if there was no run of the registration yet. */
    itkGetMacro(Iterations,int);

    /** @return Returns if the registration algorithm converged. Returns false if there was no run of the registration yet. */
    itkGetMacro(IsConverged,bool);

    /** @return Returns the FRE of the last run of the registration algorithm. Returns -1 if there was no run of the registration yet. */
    itkGetMacro(FRE,double);

    /** @brief Sets the FRE normalization factor. Default value is 1.0. */
    itkSetMacro(FRENormalizationFactor,double);

    /** @return Returns the current FRE normalization factor.*/
    itkGetMacro(FRENormalizationFactor,double);

    itkSetMacro( FixedPointSet, mitk::PointSet::Pointer )

    itkSetMacro( MovingPointSet, mitk::PointSet::Pointer )

    const mitk::Vector3D& GetTransformT() const { return m_TransformT; }

    const itk::Matrix<double,3,3>& GetTransformR() const { return m_TransformR; }

    //################## private methods for WeightedPointRegisterInv ####################
    //moved to public for testing purposes. Move back to private later.
    //Possible solution to make unit tests of private methods anyway:
    //define child-class in test class and make methods public only in child-class.

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
    itk::VariableSizeMatrix< double > C_marker(mitk::PointSet::Pointer X, const std::vector< itk::Matrix<double,3,3> > &W);

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
    vnl_vector< double > e_marker(mitk::PointSet::Pointer X, mitk::PointSet::Pointer Y, const std::vector< itk::Matrix<double,3,3> > &W);


    void SetVtkMovingPointSet(vtkSmartPointer<vtkPoints> p);

    void SetVtkFixedPointSet(vtkSmartPointer<vtkPoints> p);

protected:
  WeightedPointTransform();

  double m_Threshold;

  int m_MaxIterations;

  int m_Iterations;

  bool m_IsConverged;

  double m_FRE;

  double m_FRENormalizationFactor;

  std::vector< itk::Matrix<double,3,3> > m_CovarianceMatricesM;

  std::vector< itk::Matrix<double,3,3> > m_CovarianceMatricesS;

  mitk::PointSet::Pointer m_MovingPointSet;

  mitk::PointSet::Pointer m_FixedPointSet;

  mitk::Vector3D m_TransformT;

  itk::Matrix < double,3,3 > m_TransformR;

  vtkSmartPointer<vtkPoints> m_vtkFixedPointSet;

  vtkSmartPointer<vtkPoints> m_vtkMovingPointSet;

  double CalculateConfigChange(mitk::PointSet::Pointer MovingSetOld, mitk::PointSet::Pointer MovingSetNew);

  /** @brief Computes the mean of two point sets. The point sets must have the same size.
    * @return Returns the mean of both point sets. Returns an empty point set if the point sets do not have the same size.
    */
  mitk::PointSet::Pointer ComputeMean(mitk::PointSet::Pointer ptSet1, mitk::PointSet::Pointer ptSet2);

  /** Computes the mean of x- ,y- and z-component of the complete pointset */
  itk::Vector<double,3> ComputeMean(mitk::PointSet::Pointer pSet);

  /**
   * @brief               This method performs a variant of the weighted point register alogorithm presented by
   *                      A. Danilchenko, R. Balachandran and J. M. Fitzpatrick in January 2010. (Modified in January 2011)
   *
   * @param MovingPointSet               (input) the moving point set
   * @param FixedPointSet               (input) the fixed (static) point set
   * @param CovarianceMatricesMoving   (input) a 3-by-3-by-N array, each page containing the weighting matrix for the Nth pair of points in X (inverted and squared TODO: describe better)
   * @param CovarianceMatricesFixed   (input) a 3-by-3-by-N array, each page containing the weighting matrix for the Nth pair of points in Y (inverted and squared TODO: describe better)
   * @param Threshold               (input) the relative size of the change to the moving set above which the iteration continues
   * @param MaxIterations           (input) the maximum number of iterations allowed
   * @param TransformationR         (output) this variable will hold the computed rotation matrix
   * @param TransformationT         (output) this variable will hold the computed translation vector
   * @param TransformationR         (output) this variable will hold the computed rotation FRE of the transformation
   * @param FRE                     (output) this variable will hold the number of iterations which were done
   * @param n                       (output) this variable will hold the computed rotation matrix
   * @param isConverged             (output) this variable will hold the information of the alorithem converged
   *
   * @return  Returns true if the alorithm was computed without unexpected errors, false if not.
   */
  bool WeightedPointRegisterInvNewVariant(mitk::PointSet::Pointer MovingPointSet,
                                                  mitk::PointSet::Pointer FixedPointSet,
                                                  const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesMoving,
                                                  const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesFixed,
                                                  double Threshold,
                                                  int MaxIterations,
                                                  itk::Matrix<double,3,3>& TransformationR,
                                                  itk::Vector<double,3>& TransformationT,
                                                  double& FRE,
                                                  int& n,
                                                  bool& isConverged);

private:
  WeightedPointTransformData* d;
};

}
#endif
