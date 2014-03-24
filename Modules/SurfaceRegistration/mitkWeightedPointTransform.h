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

    /** @brief TODO */
class MitkSurfaceRegistration_EXPORT WeightedPointTransform : public itk::Object
{

  typedef itk::Matrix<double,3,3> Matrix3x3;
  typedef Matrix3x3 WeightMatrix;
  typedef Matrix3x3 Rotation;
  typedef itk::Vector<double,3> Translation;
  typedef std::vector<WeightMatrix> WeightMatrixList;
  typedef std::vector<Matrix3x3> CovarianceMatrixList;

public:

  mitkClassMacro(WeightedPointTransform, itk::Object);
  itkNewMacro(Self);

  /** @brief Method which registers both point sets. */
  void ComputeTransformation();

  /** @brief Sets the threshold of the registration. Default value is 0.0001.*/
  itkSetMacro(Threshold,double)

  /** @brief Sets the maximum number of iterations of the registration. Default value is 1000.*/
  itkSetMacro(MaxIterations,double)

  /** @return Returns the number of iterations of the last run of the registration algorithm. Returns -1 if there was no run of the registration yet. */
  itkGetMacro(Iterations,int);

  /** @return Returns the FRE of the last run of the registration algorithm. Returns -1 if there was no run of the registration yet. */
  itkGetMacro(FRE,double);

  /** @brief Sets the FRE normalization factor. Default value is 1.0. */
  itkSetMacro(FRENormalizationFactor,double);

  /** @return Returns the current FRE normalization factor.*/
  itkGetMacro(FRENormalizationFactor,double);

  void SetMovingPointSet(vtkSmartPointer<vtkPoints> p);

  void SetCovarianceMatricesMoving( const CovarianceMatrixList& matrices);

  void SetFixedPointSet(vtkSmartPointer<vtkPoints> p);

  void SetCovarianceMatricesFixed( const CovarianceMatrixList& matrices);

  const Translation& GetTransformT() const { return m_Translation; }

  const Rotation& GetTransformR() const { return m_Rotation; }


protected:
  WeightedPointTransform();
  ~WeightedPointTransform();

  double m_Threshold;

  int m_MaxIterations;

  int m_Iterations;

  double m_FRE;

  double m_FRENormalizationFactor;

  vtkSmartPointer<vtkPoints> m_FixedPointSet;

  vtkSmartPointer<vtkPoints> m_MovingPointSet;

  CovarianceMatrixList m_CovarianceMatricesMoving;

  CovarianceMatrixList m_CovarianceMatricesFixed;

  Translation m_Translation;

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
  void C_marker( vtkPoints* X, const WeightMatrixList &W, itk::VariableSizeMatrix< double >& returnValue);

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
  void E_marker( vtkPoints* X, vtkPoints* Y, const WeightMatrixList &W, vnl_vector< double >& returnValue);

  double CalculateConfigChange(vtkPoints* X, vtkPoints* X_new);

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
  bool WeightedPointRegisterInvNewVariant(  vtkPoints* X,
                                            vtkPoints* Y,
                                            const CovarianceMatrixList &Sigma_X,
                                            const CovarianceMatrixList &Sigma_Y,
                                            double Threshold,
                                            int MaxIterations,
                                            Rotation& TransformationR,
                                            Translation& TransformationT,
                                            double& FRE,
                                            int& n);

};

}
#endif
