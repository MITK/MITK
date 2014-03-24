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

//namespace mitk {
//
//class WeightedPointTransformData
//{
//
//};
//
//}

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

mitk::PointSet::Pointer DeepCopyPoints(mitk::PointSet::Pointer original)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  for (int i=0; i<original->GetSize(); i++)
      {
      mitk::Point3D newPoint;
      newPoint[0] = original->GetPoint(i)[0];
      newPoint[1] = original->GetPoint(i)[1];
      newPoint[2] = original->GetPoint(i)[2];
      returnValue->InsertPoint(i,newPoint);
      }
  return returnValue;
}

Matrix3x3List DeepCopyMatrices(const Matrix3x3List& original)
{
  Matrix3x3List returnValue;
  for (unsigned int i=0; i<original.size(); i++)
    {
    itk::Matrix<double,3,3> copy;
    copy[0][0] = original.at(i)[0][0];
    copy[0][1] = original.at(i)[0][1];
    copy[0][2] = original.at(i)[0][2];
    copy[1][0] = original.at(i)[1][0];
    copy[1][1] = original.at(i)[1][1];
    copy[1][2] = original.at(i)[1][2];
    copy[2][0] = original.at(i)[2][0];
    copy[2][1] = original.at(i)[2][1];
    copy[2][2] = original.at(i)[2][2];
    returnValue.push_back(copy);
    }
  return returnValue;
}

double ComputeWeightedFREItk (  mitk::PointSet::Pointer MovingPointSet,
                                mitk::PointSet::Pointer FixedPointSet,
                                const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesMoving,
                                const std::vector< itk::Matrix<double,3,3> > &CovarianceMatricesFixed,
                                double FRENormalizationFactor,
                                const Matrix3x3& rotation,
                                const itk::Vector<double,3>& translation
                             )
{
  //compute weighting matrices
  Matrix3x3List WeightMatrices = calculateWeightMatrices(CovarianceMatricesMoving,CovarianceMatricesFixed,rotation);

  double FRE = 0;
#pragma omp parallel for reduction(+:FRE)
  for (unsigned int ii = 0; ii < WeightMatrices.size(); ii++)
  {
    //convert to itk data types (nessecary since itk 4 migration)
    itk::Vector<double,3> converted_MovingPoint;
    converted_MovingPoint[0] = MovingPointSet->GetPoint(ii)[0];
    converted_MovingPoint[1] = MovingPointSet->GetPoint(ii)[1];
    converted_MovingPoint[2] = MovingPointSet->GetPoint(ii)[2];

    const itk::Vector<double,3> p = rotation * converted_MovingPoint + translation;

    itk::Vector<double,3> converted_FixedPoint;
    converted_FixedPoint[0] = FixedPointSet->GetPoint(ii)[0];
    converted_FixedPoint[1] = FixedPointSet->GetPoint(ii)[1];
    converted_FixedPoint[2] = FixedPointSet->GetPoint(ii)[2];
    //do calculation
    itk::Vector<double,3> D = WeightMatrices.at(ii) * (p - converted_FixedPoint);
    FRE += D[0] * D[0] + D[1] * D[1] + D[2] * D[2];
  }

  FRE /= WeightMatrices.size();
  FRE = FRENormalizationFactor * sqrt(FRE);

  return FRE;
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
  FRE_identity = ComputeWeightedFREItk(MovingPointSet,FixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,identityR,identityT);
  std::cout <<"\nFRE for identity transform: "<<FRE_identity<<std::endl; //TODO TEST
  FRE_identity = ComputeWeightedFRE(m_vtkMovingPointSet,m_vtkFixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,identityR,identityT);
  std::cout <<"\nFRE for identity transform: "<<FRE_identity<<std::endl; //TODO TEST

  //TODO TEST
  //calculate FRE with current transform
  double FRE_isotropic_weighted = 0;
  FRE_isotropic_weighted = ComputeWeightedFREItk(MovingPointSet,FixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,TransformationR,TransformationT);
  std::cout <<"FRE for transform obtained with unweighted registration: "<<FRE_isotropic_weighted<<std::endl;
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

  //apply transform to moving set:
  MovingSetTrans = mitk::PointSet::New();
  for (int i=0; i<MovingPointSet->GetSize(); i++)
  {
    mitk::Point3D transformedPoint = (TransformationR * MovingPointSet->GetPoint(i)) + TransformationT;
    MovingSetTrans->InsertPoint(i,transformedPoint);
  }

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
      if (n > MaxIterations)
        {
          isConverged = false;

          double currentFRE;
          currentFRE = ComputeWeightedFREItk(MovingPointSet,FixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed, this->m_FRENormalizationFactor,TransformationR,TransformationT);


          if (currentFRE > initialFRE)
            {
              std::cout <<"Current FRE ("<<currentFRE<<") is larger than initial FRE ("<<initialFRE<<std::endl;

              //in this case use the initial transform
              TransformationR = initial_TransformationR;
              TransformationT = initial_TransformationT;
              //transform moving points with initial transform
              mitk::PointSet::Pointer MovingSetTransNew = mitk::PointSet::New();
              for (int i=0; i<MovingSetTrans->GetSize(); i++)
                {
                  itk::Point<double,3> newPointItk = TransformationR * MovingPointSet->GetPoint(i) + TransformationT;
                  mitk::Point3D newPoint;
                  newPoint[0] = newPointItk[0];
                  newPoint[1] = newPointItk[1];
                  newPoint[2] = newPointItk[2];
                  MovingSetTransNew->InsertPoint(i,newPoint);
                }
              MovingSetTrans = MovingSetTransNew;
              FRE = initialFRE;
            }
          else //Algorithm improved FRE -> use current transform
            FRE = currentFRE;

          break;
        }

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

      //convert to needed data types
      C_marker(MovingSetTrans,W,iA);
      e_marker(MovingSetTrans,FixedPointSet,W,iB);

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
      for (int i=0; i<3; i++) for (int j=0; j<3; j++) U[i][j] = svd_delta_theta.U()[i][j];
      itk::Matrix<double,3,3> V;
      for (int i=0; i<3; i++) for (int j=0; j<3; j++) V[i][j] = svd_delta_theta.V()[i][j];

      itk::Matrix<double,3,3> delta_R = U * V.GetTranspose();

      //update rotation
      TransformationR = delta_R * TransformationR;

      //update translation
      TransformationT = delta_R * TransformationT + delta_t;

      //update moving points
      mitk::PointSet::Pointer MovingSetTransNew = mitk::PointSet::New();
      for (int i=0; i<MovingSetTrans->GetSize(); i++)
        {
          itk::Point<double,3> newPointItk = TransformationR * MovingPointSet->GetPoint(i) + TransformationT;
          mitk::Point3D newPoint;
          newPoint[0] = newPointItk[0];
          newPoint[1] = newPointItk[1];
          newPoint[2] = newPointItk[2];
          MovingSetTransNew->InsertPoint(i,newPoint);
        }

      //calculate config change
      config_change = CalculateConfigChange(MovingSetTrans,MovingSetTransNew);

      //the old set for the next iteration is the new set of the last
      MovingSetTrans = MovingSetTransNew;

    }

  //calculate FRE with current transform
  FRE = ComputeWeightedFREItk(MovingPointSet,FixedPointSet,CovarianceMatricesMoving,CovarianceMatricesFixed,m_FRENormalizationFactor,TransformationR,TransformationT);

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
  return true;
}

void mitk::WeightedPointTransform::C_marker(mitk::PointSet::Pointer X, const std::vector< itk::Matrix<double,3,3> > &W, itk::VariableSizeMatrix< double >& returnValue)
{
  unsigned int size = X->GetSize();

  if (size == W.size())
    {

      for(unsigned int i=0; i<size; i++)
        {
          unsigned int index = 3u * i;

          returnValue[index][0] = -W.at(i)[0][1] * X->GetPoint(i)[2] + W.at(i)[0][2] * X->GetPoint(i)[1];
          returnValue[index][1] = W.at(i)[0][0] * X->GetPoint(i)[2] - W.at(i)[0][2] * X->GetPoint(i)[0];
          returnValue[index][2] = - W.at(i)[0][0] * X->GetPoint(i)[1] + W.at(i)[0][1] * X->GetPoint(i)[0];
          returnValue[index][3] = W.at(i)[0][0];
          returnValue[index][4] = W.at(i)[0][1];
          returnValue[index][5] = W.at(i)[0][2];

          ++index;

          returnValue[index][0] = -W.at(i)[1][1] * X->GetPoint(i)[2] + W.at(i)[1][2] * X->GetPoint(i)[1];
          returnValue[index][1] = W.at(i)[1][0] * X->GetPoint(i)[2] - W.at(i)[1][2] * X->GetPoint(i)[0];
          returnValue[index][2] = - W.at(i)[1][0] * X->GetPoint(i)[1] + W.at(i)[1][1] * X->GetPoint(i)[0];
          returnValue[index][3] = W.at(i)[1][0];
          returnValue[index][4] = W.at(i)[1][1];
          returnValue[index][5] = W.at(i)[1][2];

          ++index;

          returnValue[index][0] = -W.at(i)[2][1] * X->GetPoint(i)[2] + W.at(i)[2][2] * X->GetPoint(i)[1];
          returnValue[index][1] = W.at(i)[2][0] * X->GetPoint(i)[2] - W.at(i)[2][2] * X->GetPoint(i)[0];
          returnValue[index][2] = - W.at(i)[2][0] * X->GetPoint(i)[1] + W.at(i)[2][1] * X->GetPoint(i)[0];
          returnValue[index][3] = W.at(i)[2][0];
          returnValue[index][4] = W.at(i)[2][1];
          returnValue[index][5] = W.at(i)[2][2];
        }
    }
}

void mitk::WeightedPointTransform::e_marker( mitk::PointSet::Pointer X,
                                                             mitk::PointSet::Pointer Y,
                                                             const std::vector< itk::Matrix<double,3,3> > &W,
                                                             vnl_vector< double >& returnValue
                                                            )
{
  unsigned int size = X->GetSize();

  if ((size == Y->GetSize())&&(size == W.size()))
    {
      std::vector< itk::Matrix<double,3,3> > D;

      D.reserve(size);

      for(int i=0; i<size; i++)
        {
          itk::Matrix<double,3,3> newMatrix;
          newMatrix[0][0] = Y->GetPoint(i)[0] - X->GetPoint(i)[0];
          newMatrix[0][1] = Y->GetPoint(i)[1] - X->GetPoint(i)[1];
          newMatrix[0][2] = Y->GetPoint(i)[2] - X->GetPoint(i)[2];
          newMatrix[1][0] = newMatrix[0][0];
          newMatrix[1][1] = newMatrix[0][1];
          newMatrix[1][2] = newMatrix[0][2];
          newMatrix[2][0] = newMatrix[0][0];
          newMatrix[2][1] = newMatrix[0][1];
          newMatrix[2][2] = newMatrix[0][2];
          D.push_back(newMatrix);
        }

      for(int i=0; i<size; i++)
        {
          unsigned int index = 3u * i;

          returnValue[index++] = W.at(i)[0][0] * D.at(i)[0][0] + W.at(i)[0][1] * D.at(i)[0][1] + W.at(i)[0][2] * D.at(i)[0][2];
          returnValue[index++] = W.at(i)[1][0] * D.at(i)[1][0] + W.at(i)[1][1] * D.at(i)[1][1] + W.at(i)[1][2] * D.at(i)[1][2];
          returnValue[index] = W.at(i)[2][0] * D.at(i)[2][0] + W.at(i)[2][1] * D.at(i)[2][1] + W.at(i)[2][2] * D.at(i)[2][2];
        }
    }
}

void mitk::WeightedPointTransform::SetVtkMovingPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_vtkMovingPointSet = p;
}

void mitk::WeightedPointTransform::SetVtkFixedPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_vtkFixedPointSet = p;
}

double mitk::WeightedPointTransform::CalculateConfigChange(mitk::PointSet::Pointer MovingSetOld,mitk::PointSet::Pointer MovingSetNew)
{
  itk::Vector<double,3> mean = ComputeMean(MovingSetOld);
  //std::cout <<"MEan old"<<mean<<std::endl;
  itk::Vector<double,3> sqrSum = itk::Vector<double,3>(); sqrSum.Fill(0);
  for(int i=0; i<MovingSetOld->GetSize(); i++)
    {
      sqrSum[0] += (pow((MovingSetNew->GetPoint(i)[0] - MovingSetOld->GetPoint(i)[0]),2));
      sqrSum[1] += (pow((MovingSetNew->GetPoint(i)[1] - MovingSetOld->GetPoint(i)[1]),2));
      sqrSum[2] += (pow((MovingSetNew->GetPoint(i)[2] - MovingSetOld->GetPoint(i)[2]),2));
    }
  double zaehler = sqrSum[0]+sqrSum[1]+sqrSum[2];

  itk::Vector<double,3> sqrSum2 = itk::Vector<double,3>(); sqrSum2.Fill(0);
  for(int i=0; i<MovingSetOld->GetSize(); i++)
    {
      sqrSum2[0] += (pow((MovingSetOld->GetPoint(i)[0] - mean[0]),2));
      sqrSum2[1] += (pow((MovingSetOld->GetPoint(i)[1] - mean[1]),2));
      sqrSum2[2] += (pow((MovingSetOld->GetPoint(i)[2] - mean[2]),2));
    }
  double nenner = sqrSum2[0]+sqrSum2[1]+sqrSum2[2];
  double config_change = sqrt(zaehler / nenner);

  return config_change;
}

itk::Vector<double,3> mitk::WeightedPointTransform::ComputeMean(mitk::PointSet::Pointer pSet)
{
  itk::Vector<double,3> returnValue = itk::Vector<double,3>();  returnValue.Fill(0);
  for (int i=0; i<pSet->GetSize(); i++)
    {
      returnValue[0] = returnValue[0] + pSet->GetPoint(i)[0];
      returnValue[1] = returnValue[1] + pSet->GetPoint(i)[1];
      returnValue[2] = returnValue[2] + pSet->GetPoint(i)[2];
    }
  returnValue = returnValue / pSet->GetSize();

  return returnValue;
}

mitk::PointSet::Pointer mitk::WeightedPointTransform::ComputeMean(mitk::PointSet::Pointer ptSet1, mitk::PointSet::Pointer ptSet2)
{
  mitk::PointSet::Pointer returnValue = mitk::PointSet::New();
  if (ptSet1->GetSize() == ptSet2->GetSize())
    {
      for (int i=0; i<ptSet1->GetSize(); i++)
        {
          //compute mean of current points
          mitk::Point3D point1 = ptSet1->GetPoint(i);
          mitk::Point3D point2 = ptSet2->GetPoint(i);
          mitk::Point3D mean;
          mean[0] = (point1[0] + point2[0])/2;
          mean[1] = (point1[1] + point2[1])/2;
          mean[2] = (point1[2] + point2[2])/2;

          //add the mean to the returnvalue
          returnValue->InsertPoint(i,mean);
        }
    }
  return returnValue;
}
