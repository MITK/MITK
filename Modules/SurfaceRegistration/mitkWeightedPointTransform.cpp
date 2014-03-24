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

//MITK
#include "mitkWeightedPointTransform.h"
#include "mitkAnisotropicRegistrationCommon.h"
#include <itkLandmarkBasedTransformInitializer.h>
#include <itkImage.h>
#include <vtkLandmarkTransform.h>
#include <vtkPoints.h>
#include <vtkMatrix4x4.h>

mitk::WeightedPointTransform::WeightedPointTransform()
{
  m_Threshold = 1.0e-4;
  m_Iterations = -1;
  m_IsConverged = false;
  m_FRE = -1;
  m_FRENormalizationFactor = 1.0;
}

void mitk::WeightedPointTransform::SetThreshold(double threshold)
{
  m_Threshold = threshold;
}

void mitk::WeightedPointTransform::SetMaxIterations(double value)
{
  m_MaxIterations = value;
}

void mitk::WeightedPointTransform::SetCovarianceMatrices(std::vector< itk::Matrix<double,3,3> > CovarianceMatricesM,std::vector< itk::Matrix<double,3,3> > CovarianceMatricesS)
{
  m_CovarianceMatricesM = CovarianceMatricesM;
  m_CovarianceMatricesS = CovarianceMatricesS;
}

bool mitk::WeightedPointTransform::Update()
{
   return WeightedPointRegisterInvNewVariant(m_MovingPointSet,m_FixedPointSet,m_CovarianceMatricesM,m_CovarianceMatricesS,m_Threshold,m_MaxIterations,m_TransformR,m_TransformT,m_FRE,m_Iterations,m_IsConverged);
}

typedef itk::Matrix < double,3,3 > Matrix3x3;
typedef std::vector < Matrix3x3 > Matrix3x3List;
static Matrix3x3List calculateWeightMatrices(const Matrix3x3List& X, const Matrix3x3List& Y, const Matrix3x3& rotation)
{
  const vnl_matrix_fixed<double,3,3> rotation_T = rotation.GetTranspose();
  Matrix3x3List result;
  result.resize(X.size());

#pragma omp parallel for
  for ( size_t i = 0; i < X.size(); ++i )
    {
      const Matrix3x3 w = rotation * X[i] * rotation_T;
      result[i] = mitk::AnisotropicRegistrationCommon::CalculateWeightMatrix(w,Y[i]);
    }

  return result;
}

double ComputeWeightedFRE (  vtkPoints* X,
                             vtkPoints* Y,
                             const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesMoving,
                             const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesFixed,
                             double FRENormalizationFactor,
                             const Matrix3x3& rotation,
                             const itk::Vector<double,3>& translation
                           )
{
  double FRE = 0;
  //compute weighting matrices
  Matrix3x3List WeightMatrices = calculateWeightMatrices(CovarianceMatricesMoving,CovarianceMatricesFixed,rotation);

#pragma omp parallel for reduction(+:FRE)
  for (unsigned int i = 0; i < WeightMatrices.size(); ++i)
  {
    //convert to itk data types (nessecary since itk 4 migration)
    itk::Vector<double,3> converted_MovingPoint;
    double point[3];
    X->GetPoint(i,point);
    converted_MovingPoint[0] = point[0];
    converted_MovingPoint[1] = point[1];
    converted_MovingPoint[2] = point[2];

    // transform point
    itk::Vector<double,3> p = rotation * converted_MovingPoint + translation;

    Y->GetPoint(i,point);
    p[0] -= point[0];
    p[1] -= point[1];
    p[2] -= point[2];

    //do calculation
    const itk::Vector<double,3> D = WeightMatrices.at(i) * p;
    FRE += (D[0] * D[0] + D[1] * D[1] + D[2] * D[2]);
  }

  FRE /= WeightMatrices.size();
  FRE = FRENormalizationFactor * sqrt(FRE);

  return FRE;
}

static void IsotropicRegistration( vtkPoints* X, vtkPoints* Y, Matrix3x3& rotation, itk::Vector<double,3>& translation)
{
  vtkSmartPointer<vtkLandmarkTransform> landmarkTransform = vtkSmartPointer<vtkLandmarkTransform>::New();
  landmarkTransform->SetSourceLandmarks(X);
  landmarkTransform->SetTargetLandmarks(Y);
  landmarkTransform->SetModeToRigidBody();
  landmarkTransform->Modified();
  landmarkTransform->Update();

  vtkMatrix4x4* m = landmarkTransform->GetMatrix();

  for ( int i = 0; i < 3; ++i )
    for ( int j = 0; j < 3;++j )
      rotation[i][j] = m->GetElement(i,j);

  translation[0] = m->GetElement(0,3);
  translation[1] = m->GetElement(1,3);
  translation[2] = m->GetElement(2,3);
}

void mitk::WeightedPointTransform::C_marker( vtkPoints* X,
                                            const std::vector< itk::Matrix<double,3,3> > &W,
                                            itk::VariableSizeMatrix< double >& returnValue)
{
  for(vtkIdType i = 0; i < X->GetNumberOfPoints(); ++i )
  {
    unsigned int index = 3u * i;
    double point[3];
    X->GetPoint(i,point);

    returnValue[index][0] = -W.at(i)[0][1] * point[2] + W.at(i)[0][2] * point[1];
    returnValue[index][1] = W.at(i)[0][0] * point[2] - W.at(i)[0][2] * point[0];
    returnValue[index][2] = - W.at(i)[0][0] * point[1] + W.at(i)[0][1] * point[0];
    returnValue[index][3] = W.at(i)[0][0];
    returnValue[index][4] = W.at(i)[0][1];
    returnValue[index][5] = W.at(i)[0][2];

    ++index;

    returnValue[index][0] = -W.at(i)[1][1] * point[2] + W.at(i)[1][2] * point[1];
    returnValue[index][1] = W.at(i)[1][0] * point[2] - W.at(i)[1][2] * point[0];
    returnValue[index][2] = - W.at(i)[1][0] * point[1] + W.at(i)[1][1] * point[0];
    returnValue[index][3] = W.at(i)[1][0];
    returnValue[index][4] = W.at(i)[1][1];
    returnValue[index][5] = W.at(i)[1][2];

    ++index;

    returnValue[index][0] = -W.at(i)[2][1] *  point[2] + W.at(i)[2][2] * point[1];
    returnValue[index][1] = W.at(i)[2][0] *   point[2] - W.at(i)[2][2] * point[0];
    returnValue[index][2] = - W.at(i)[2][0] * point[1] + W.at(i)[2][1] * point[0];
    returnValue[index][3] = W.at(i)[2][0];
    returnValue[index][4] = W.at(i)[2][1];
    returnValue[index][5] = W.at(i)[2][2];
  }
}

void mitk::WeightedPointTransform::E_marker( vtkPoints* X,
                                             vtkPoints* Y,
                                             const std::vector< itk::Matrix<double,3,3> > &W,
                                             vnl_vector< double >& returnValue
                                            )
{
  unsigned int size = X->GetNumberOfPoints();

  std::vector< itk::Matrix<double,3,3> > D;

  D.reserve(size);

  for(unsigned int i = 0; i < size; ++i)
  {
    itk::Matrix<double,3,3> newMatrix;
    double pX[3];
    double pY[3];

    X->GetPoint(i,pX);
    Y->GetPoint(i,pY);
    newMatrix[0][0] = pY[0] - pX[0];
    newMatrix[0][1] = pY[1] - pX[1];
    newMatrix[0][2] = pY[2] - pX[2];
    newMatrix[1][0] = newMatrix[0][0];
    newMatrix[1][1] = newMatrix[0][1];
    newMatrix[1][2] = newMatrix[0][2];
    newMatrix[2][0] = newMatrix[0][0];
    newMatrix[2][1] = newMatrix[0][1];
    newMatrix[2][2] = newMatrix[0][2];
    D.push_back(newMatrix);
  }

  for(int i=0; i<size; ++i)
    {
      unsigned int index = 3u * i;

      returnValue[index++] = W.at(i)[0][0] * D.at(i)[0][0] + W.at(i)[0][1] * D.at(i)[0][1] + W.at(i)[0][2] * D.at(i)[0][2];
      returnValue[index++] = W.at(i)[1][0] * D.at(i)[1][0] + W.at(i)[1][1] * D.at(i)[1][1] + W.at(i)[1][2] * D.at(i)[1][2];
      returnValue[index] = W.at(i)[2][0] * D.at(i)[2][0] + W.at(i)[2][1] * D.at(i)[2][1] + W.at(i)[2][2] * D.at(i)[2][2];
    }
}

bool mitk::WeightedPointTransform::WeightedPointRegisterInvNewVariant(
    mitk::PointSet::Pointer MovingPointSet,
    mitk::PointSet::Pointer FixedPointSet,
    const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesMoving,
    const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesFixed,
    double Threshold,
    int MaxIterations,
    itk::Matrix<double,3,3>& TransformationR,
    itk::Vector<double,3>& TransformationT,
    double& FRE,
    int& n,
    bool& isConverged
    )
{
  //test fitzpatricks new variant (2011 03 08)


  //calculate unweighted registration as initial estimate:
  itk::Matrix<double,3,3> initial_TransformationR;
  itk::Vector<double,3> initial_TransformationT;

  m_vtkFixedPointSet  = mitk::AnisotropicRegistrationCommon::GetVTKPoints(FixedPointSet);
  m_vtkMovingPointSet = mitk::AnisotropicRegistrationCommon::GetVTKPoints(MovingPointSet);

  IsotropicRegistration(m_vtkMovingPointSet, m_vtkFixedPointSet,initial_TransformationR, initial_TransformationT);

  //result of unweighted registration algorithm
  TransformationR = initial_TransformationR;
  TransformationT = initial_TransformationT;

  //apply transform to moving set:
  mitk::PointSet::Pointer MovingSetTrans; //= mitk::PointSet::New();

  //###################### compare with identity ############################

  //calculate FRE0 with identity transform
  double FRE_identity = 0.0;
  itk::Matrix<double,3,3> identityR; identityR.SetIdentity();
  itk::Vector<double,3> identityT; identityT.Fill(0);
  FRE_identity = ComputeWeightedFRE(m_vtkMovingPointSet,m_vtkFixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,identityR,identityT);
  std::cout <<"\nFRE for identity transform: "<<FRE_identity<<std::endl; //TODO TEST

  //TODO TEST
  //calculate FRE with current transform
  double FRE_isotropic_weighted = 0;
  FRE_isotropic_weighted = ComputeWeightedFRE(m_vtkMovingPointSet,m_vtkFixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,TransformationR,TransformationT);
  std::cout <<"FRE for transform obtained with unweighted registration: "<<FRE_isotropic_weighted<<std::endl;

  //TODO END TEST

  double initialFRE; //FRE of initial transform
  if (FRE_isotropic_weighted < FRE_identity)  //if R,t is worse than the identity, use the identity as initial transform
  {
      initialFRE = FRE_isotropic_weighted;
  }
  else
  {
      initialFRE = FRE_identity;
      TransformationR = identityR; //set rotation to identity element
      TransformationT = identityT; //set translation to identity element
      initial_TransformationR = identityR;
      initial_TransformationT = identityT;
  }


  vtkPoints* vtkMovingSetTrans = vtkPoints::New();
  vtkMovingSetTrans->SetNumberOfPoints(m_vtkMovingPointSet->GetNumberOfPoints());
  vtkPoints* vtkMovingSetTransNew = vtkPoints::New();
  vtkMovingSetTransNew->SetNumberOfPoints(m_vtkMovingPointSet->GetNumberOfPoints());

  //apply transform to moving set:
  MovingSetTrans = mitk::PointSet::New();
 // for (int i=0; i<MovingPointSet->GetSize(); i++)
 // {
 //   mitk::Point3D transformedPoint = (TransformationR * MovingPointSet->GetPoint(i)) + TransformationT;
 //   MovingSetTrans->InsertPoint(i,transformedPoint);
 // }

  mitk::AnisotropicRegistrationCommon::TransformPoints( m_vtkMovingPointSet, vtkMovingSetTrans,
                                                        TransformationR, TransformationT);

  //set config_change to infinite (max double) at start
  double config_change = std::numeric_limits<double>::max();

  //start with iteration 0
  n = 0;

  //declarate needed variables
  vnl_vector< double > oldq;
  std::vector< itk::Matrix<double,3,3> > W;

  itk::VariableSizeMatrix< double > iA;
  iA.SetSize(3u * m_vtkMovingPointSet->GetNumberOfPoints(), 6u);
  vnl_vector< double > iB;
  iB.set_size(3u * m_vtkMovingPointSet->GetNumberOfPoints());

  while (config_change>Threshold)
  {
      //check wether maximum iterations is reached
//      if (n > MaxIterations)
//        {
//          isConverged = false;
//
//          double currentFRE;
//          currentFRE = ComputeWeightedFREItk(MovingPointSet,FixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed, this->m_FRENormalizationFactor,TransformationR,TransformationT);
//
//
//          if (currentFRE > initialFRE)
//            {
//              std::cout <<"Current FRE ("<<currentFRE<<") is larger than initial FRE ("<<initialFRE<<std::endl;
//
//              //in this case use the initial transform
//              TransformationR = initial_TransformationR;
//              TransformationT = initial_TransformationT;
//              //transform moving points with initial transform
//              mitk::PointSet::Pointer MovingSetTransNew = mitk::PointSet::New();
//              for (int i=0; i<MovingSetTrans->GetSize(); i++)
//                {
//                  itk::Point<double,3> newPointItk = TransformationR * MovingPointSet->GetPoint(i) + TransformationT;
//                  mitk::Point3D newPoint;
//                  newPoint[0] = newPointItk[0];
//                  newPoint[1] = newPointItk[1];
//                  newPoint[2] = newPointItk[2];
//                  MovingSetTransNew->InsertPoint(i,newPoint);
//                }
//              MovingSetTrans = MovingSetTransNew;
//              FRE = initialFRE;
//            }
//          else //Algorithm improved FRE -> use current transform
//            FRE = currentFRE;
//
//          break;
//        }

      n++;

      //TODO RUN-TIME: is calculated twice in first iteration
      W = calculateWeightMatrices(CovarianceMatricesMoving,CovarianceMatricesFixed,TransformationR);
      //        mitk::WeightMatricesCalculator::CalculateWeightMatrices(CovarianceMatricesMoving,TransformationR,CovarianceMatricesFixed);

      //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
      //PROBLEM:  no square matrix but the backslash operator in matlab does solve the system anyway. How to convert this to C++?
      //          good descriptons to the "backslash"-operator (in german): http://www.tm-mathe.de/Themen/html/matlab__zauberstab__backslash-.html
      //                                                                    http://www.tm-mathe.de/Themen/html/matlab__matrix-division__vorsi.html#HoheMatrixA
      //
      //          current method: treat the problem as a minimization problem, because this is what the "backslash"-operator also does with "high" matrices.
      //                          (and we will have those matrices in most cases)

      C_marker(vtkMovingSetTrans,W,iA);
      E_marker(vtkMovingSetTrans,m_vtkFixedPointSet,W,iB);

      vnl_matrix_inverse<double> myInverse(iA.GetVnlMatrix());
      vnl_vector< double > q = myInverse.pinverse(iB.size()) * iB;
      //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

      if (n>1)
        q = (q+oldq)/2;
      oldq = q;

      itk::Vector<double,3> delta_t;
      delta_t[0] = q[3];
      delta_t[1] = q[4];
      delta_t[2] = q[5];

      itk::Matrix<double,3,3> delta_theta;
      delta_theta[0][0] = 1;
      delta_theta[0][1] = -q[2];
      delta_theta[0][2] = q[1];
      delta_theta[1][0] = q[2];
      delta_theta[1][1] = 1;
      delta_theta[1][2] = -q[0];
      delta_theta[2][0] = -q[1];
      delta_theta[2][1] = q[0];
      delta_theta[2][2] = 1;

      vnl_svd<double> svd_delta_theta(delta_theta.GetVnlMatrix());

      //convert vnl matrices to itk matrices...
      itk::Matrix<double,3,3> U;
      itk::Matrix<double,3,3> V;

      for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
          U[i][j] = svd_delta_theta.U()[i][j];
          V[i][j] = svd_delta_theta.V()[i][j];
        }
      }

      itk::Matrix<double,3,3> delta_R = U * V.GetTranspose();

      //update rotation
      TransformationR = delta_R * TransformationR;

      //update translation
      TransformationT = delta_R * TransformationT + delta_t;

      //update moving points
      mitk::AnisotropicRegistrationCommon::TransformPoints( m_vtkMovingPointSet,
                                                            vtkMovingSetTransNew,
                                                            TransformationR,
                                                            TransformationT );
      //calculate config change
      config_change = CalculateConfigChange(vtkMovingSetTrans,vtkMovingSetTransNew);

      //the old set for the next iteration is the new set of the last
      vtkPoints* tmp = vtkMovingSetTrans;
      vtkMovingSetTrans = vtkMovingSetTransNew;
      vtkMovingSetTransNew = tmp;

      if ( n > MaxIterations )
      {
          MITK_ERROR << "max iteration reached";
          break;
      }
  }

  //calculate FRE with current transform
  FRE = ComputeWeightedFRE(m_vtkMovingPointSet,m_vtkFixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,TransformationR,TransformationT);

  std::cout <<"FRE after algorithm (prior to check with initial): "<<FRE<<std::endl;

  //compare with FRE_initial
  if (initialFRE<FRE)
    {
      std::cout<<"WARNING: FRE did not improve in anisotropic point registration function\n";
      TransformationR = initial_TransformationR;
      TransformationT = initial_TransformationT;
      FRE = initialFRE;
    }

  std::cout <<"FRE final: "<<FRE<<std::endl;

  std::cout.precision(2);

  vtkMovingSetTrans->Delete();
  vtkMovingSetTransNew->Delete();
  return true;
}

void mitk::WeightedPointTransform::SetVtkMovingPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_vtkMovingPointSet = p;
}

void mitk::WeightedPointTransform::SetVtkFixedPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_vtkFixedPointSet = p;
}

double mitk::WeightedPointTransform::CalculateConfigChange(vtkPoints* X, vtkPoints* X_new)
{
  double sum[3] = { 0.0,0.0,0.0 };
  double mean[3] = { 0.0,0.0,0.0 };
  double pX[3] = { 0.0,0.0,0.0 };
  double pX_new[3] = { 0.0,0.0,0.0 };

  // compute mean of the old point set and the first sum
  for ( vtkIdType i = 0; i < X->GetNumberOfPoints(); ++i )
  {
    X->GetPoint(i,pX);
    X_new->GetPoint(i,pX_new);

    // first sum
    sum[0] += ( pX_new[0] - pX[0] ) * ( pX_new[0] - pX[0] );
    sum[1] += ( pX_new[1] - pX[1] ) * ( pX_new[1] - pX[1] );
    sum[2] += ( pX_new[2] - pX[2] ) * ( pX_new[2] - pX[2] );

    // mean
    mean[0] += pX[0];
    mean[1] += pX[1];
    mean[2] += pX[2];
  }

  mean[0] /= X->GetNumberOfPoints();
  mean[1] /= X->GetNumberOfPoints();
  mean[2] /= X->GetNumberOfPoints();

  const double us = sum[0] + sum[1] + sum[2];

  // reset sum
  sum[0] = sum[1] = sum[2] = 0.0;

  for ( vtkIdType i = 0; i < X->GetNumberOfPoints(); ++i )
  {
    X->GetPoint(i,pX);

    sum[0] += (pX[0] - mean[0]) * (pX[0] - mean[0]);
    sum[1] += (pX[1] - mean[1]) * (pX[1] - mean[1]);
    sum[2] += (pX[2] - mean[2]) * (pX[2] - mean[2]);
  }

  const double ls = sum[0] + sum[1] + sum[2];

  return sqrt(us/ls);
}
