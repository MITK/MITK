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

// MITK
#include "mitkWeightedPointTransform.h"
#include "mitkAnisotropicRegistrationCommon.h"
#include <vtkLandmarkTransform.h>
#include <vtkMatrix4x4.h>
#include <vtkPoints.h>

typedef itk::Matrix<double, 3, 3> Matrix3x3;
typedef std::vector<Matrix3x3> Matrix3x3List;

///////////////////////////////////////////////
// forward declarations of private functions
///////////////////////////////////////////////

static double ComputeWeightedFRE(vtkPoints *X,
                                 vtkPoints *Y,
                                 const Matrix3x3List &CovarianceMatricesMoving,
                                 const Matrix3x3List &CovarianceMatricesFixed,
                                 double FRENormalizationFactor,
                                 Matrix3x3List &WeightMatrices,
                                 const Matrix3x3 &rotation,
                                 const itk::Vector<double, 3> &translation);

static void calculateWeightMatrices(const Matrix3x3List &X,
                                    const Matrix3x3List &Y,
                                    Matrix3x3List &result,
                                    const Matrix3x3 &rotation);

static void IsotropicRegistration(vtkPoints *X,
                                  vtkPoints *Y,
                                  vtkLandmarkTransform *landmarkTransform,
                                  Matrix3x3 &rotation,
                                  itk::Vector<double, 3> &translation);

mitk::WeightedPointTransform::WeightedPointTransform()
  : m_Threshold(1.0e-4),
    m_MaxIterations(1000),
    m_Iterations(-1),
    m_FRE(-1.0),
    m_FRENormalizationFactor(1.0),
    m_LandmarkTransform(vtkSmartPointer<vtkLandmarkTransform>::New())
{
}

mitk::WeightedPointTransform::~WeightedPointTransform()
{
  m_FixedPointSet = nullptr;
  m_MovingPointSet = nullptr;
  m_LandmarkTransform = nullptr;
}

void mitk::WeightedPointTransform::ComputeTransformation()
{
  WeightedPointRegister(m_MovingPointSet,
                        m_FixedPointSet,
                        m_CovarianceMatricesMoving,
                        m_CovarianceMatricesFixed,
                        m_Threshold,
                        m_MaxIterations,
                        m_Rotation,
                        m_Translation,
                        m_FRE,
                        m_Iterations);
}

// computes the weightmatrix with 2 covariance matrices
// and a given transformation
void calculateWeightMatrices(const Matrix3x3List &X,
                             const Matrix3x3List &Y,
                             Matrix3x3List &result,
                             const Matrix3x3 &rotation)
{
  const vnl_matrix_fixed<double, 3, 3> rotation_T = rotation.GetTranspose();

#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(X.size()); ++i)
  {
    const Matrix3x3 w = rotation * X[i] * rotation_T;
    result[i] = mitk::AnisotropicRegistrationCommon::CalculateWeightMatrix(w, Y[i]);
  }
}

// computes the weighted fiducial registration error
double ComputeWeightedFRE(vtkPoints *X,
                          vtkPoints *Y,
                          const Matrix3x3List &CovarianceMatricesMoving,
                          const Matrix3x3List &CovarianceMatricesFixed,
                          double FRENormalizationFactor,
                          Matrix3x3List &WeightMatrices,
                          const Matrix3x3 &rotation,
                          const itk::Vector<double, 3> &translation)
{
  double FRE = 0;
  // compute weighting matrices
  calculateWeightMatrices(CovarianceMatricesMoving, CovarianceMatricesFixed, WeightMatrices, rotation);

#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(WeightMatrices.size()); ++i)
  {
    // convert to itk data types (nessecary since itk 4 migration)
    itk::Vector<double, 3> converted_MovingPoint;
    double point[3];
    X->GetPoint(i, point);
    converted_MovingPoint[0] = point[0];
    converted_MovingPoint[1] = point[1];
    converted_MovingPoint[2] = point[2];

    // transform point
    itk::Vector<double, 3> p = rotation * converted_MovingPoint + translation;

    Y->GetPoint(i, point);
    p[0] -= point[0];
    p[1] -= point[1];
    p[2] -= point[2];

    // do calculation
    const itk::Vector<double, 3> D = WeightMatrices.at(i) * p;
#pragma omp critical
    FRE += (D[0] * D[0] + D[1] * D[1] + D[2] * D[2]);
  }

  FRE /= WeightMatrices.size();
  FRE = FRENormalizationFactor * sqrt(FRE);

  return FRE;
}

// registers two pointsets with an isotropic landmark transform
void IsotropicRegistration(vtkPoints *X,
                           vtkPoints *Y,
                           vtkLandmarkTransform *landmarkTransform,
                           Matrix3x3 &rotation,
                           itk::Vector<double, 3> &translation)
{
  landmarkTransform->SetSourceLandmarks(X);
  landmarkTransform->SetTargetLandmarks(Y);
  landmarkTransform->SetModeToRigidBody();
  landmarkTransform->Modified();
  landmarkTransform->Update();

  vtkMatrix4x4 *m = landmarkTransform->GetMatrix();

  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      rotation[i][j] = m->GetElement(i, j);

  translation[0] = m->GetElement(0, 3);
  translation[1] = m->GetElement(1, 3);
  translation[2] = m->GetElement(2, 3);
}

void mitk::WeightedPointTransform::C_maker(vtkPoints *X,
                                           const WeightMatrixList &W,
                                           itk::VariableSizeMatrix<double> &returnValue)
{
#pragma omp parallel for
  for (int i = 0; i < X->GetNumberOfPoints(); ++i)
  {
    unsigned int index = 3u * i;
    double point[3];
    X->GetPoint(i, point);

    for (int j = 0; j < 3; ++j)
    {
      returnValue[index][0] = -W.at(i)[j][1] * point[2] + W.at(i)[j][2] * point[1];
      returnValue[index][1] = W.at(i)[j][0] * point[2] - W.at(i)[j][2] * point[0];
      returnValue[index][2] = -W.at(i)[j][0] * point[1] + W.at(i)[j][1] * point[0];
      returnValue[index][3] = W.at(i)[j][0];
      returnValue[index][4] = W.at(i)[j][1];
      returnValue[index][5] = W.at(i)[j][2];
      index += 1;
    }
  }
}

void mitk::WeightedPointTransform::E_maker(vtkPoints *X,
                                           vtkPoints *Y,
                                           const WeightMatrixList &W,
                                           vnl_vector<double> &returnValue)
{
#pragma omp parallel for
  for (int i = 0; i < X->GetNumberOfPoints(); ++i)
  {
    unsigned int index = 3u * i;
    double pX[3];
    double pY[3];
    Matrix3x3 M;

    X->GetPoint(i, pX);
    Y->GetPoint(i, pY);

    M[0][0] = pY[0] - pX[0];
    M[0][1] = pY[1] - pX[1];
    M[0][2] = pY[2] - pX[2];
    M[1][0] = M[0][0];
    M[1][1] = M[0][1];
    M[1][2] = M[0][2];
    M[2][0] = M[0][0];
    M[2][1] = M[0][1];
    M[2][2] = M[0][2];

    for (unsigned int j = 0; j < 3; ++j)
    {
      returnValue[index + j] = W.at(i)[j][0] * M[j][0] + W.at(i)[j][1] * M[j][1] + W.at(i)[j][2] * M[j][2];
    }
  }
}

void mitk::WeightedPointTransform::WeightedPointRegister(vtkPoints *X,
                                                         vtkPoints *Y,
                                                         const CovarianceMatrixList &Sigma_X,
                                                         const CovarianceMatrixList &Sigma_Y,
                                                         double Threshold,
                                                         int MaxIterations,
                                                         Rotation &TransformationR,
                                                         Translation &TransformationT,
                                                         double &FRE,
                                                         int &n)
{
  double FRE_identity = 0.0;
  double FRE_isotropic_weighted = 0.0;
  double initialFRE = 0.0;
  // set config_change to infinite (max double) at start
  double config_change = std::numeric_limits<double>::max();
  Rotation initial_TransformationR;
  initial_TransformationR.SetIdentity();
  Translation initial_TransformationT;
  initial_TransformationT.Fill(0.0);
  // Weightmatrices
  Matrix3x3List W;
  vtkPoints *X_transformed = vtkPoints::New();
  vtkPoints *X_transformedNew = vtkPoints::New();
  vnl_vector<double> oldq;
  itk::VariableSizeMatrix<double> iA;
  vnl_vector<double> iB;

  // initialize memory
  W.resize(X->GetNumberOfPoints());
  X_transformed->SetNumberOfPoints(X->GetNumberOfPoints());
  X_transformedNew->SetNumberOfPoints(X->GetNumberOfPoints());
  iA.SetSize(3u * X->GetNumberOfPoints(), 6u);
  iB.set_size(3u * X->GetNumberOfPoints());

  // calculate FRE_0 with identity transform
  FRE_identity = ComputeWeightedFRE(
    X, Y, Sigma_X, Sigma_Y, m_FRENormalizationFactor, W, initial_TransformationR, initial_TransformationT);

  MITK_DEBUG << "FRE for identity transform: " << FRE_identity;

  // compute isotropic transformation as initial estimate
  IsotropicRegistration(X, Y, m_LandmarkTransform, initial_TransformationR, initial_TransformationT);

  // result of unweighted registration algorithm
  TransformationR = initial_TransformationR;
  TransformationT = initial_TransformationT;

  // calculate FRE_0 with isotropic transform
  FRE_isotropic_weighted =
    ComputeWeightedFRE(X, Y, Sigma_X, Sigma_Y, m_FRENormalizationFactor, W, TransformationR, TransformationT);
  MITK_DEBUG << "FRE for transform obtained with unweighted registration: " << FRE_isotropic_weighted;

  // if R,t is worse than the identity, use the identity as initial transform
  if (FRE_isotropic_weighted < FRE_identity)
  {
    initialFRE = FRE_isotropic_weighted;
  }
  else
  {
    initialFRE = FRE_identity;
    TransformationR.SetIdentity(); // set rotation to identity element
    TransformationT.Fill(0.0);     // set translation to identity element
    initial_TransformationR.SetIdentity();
    initial_TransformationT.Fill(0.0);
  }

  // apply transform to moving set:
  mitk::AnisotropicRegistrationCommon::TransformPoints(X, X_transformed, TransformationR, TransformationT);

  // start with iteration 0
  n = 0;

  do
  {
    n++;

    calculateWeightMatrices(Sigma_X, Sigma_Y, W, TransformationR);

    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''
    // PROBLEM:  no square matrix but the backslash operator in matlab does solve the system anyway. How to convert this
    // to C++?
    //          good descriptons to the "backslash"-operator (in german):
    //          http://www.tm-mathe.de/Themen/html/matlab__zauberstab__backslash-.html
    //                                                                    http://www.tm-mathe.de/Themen/html/matlab__matrix-division__vorsi.html#HoheMatrixA
    //
    //          current method: treat the problem as a minimization problem, because this is what the
    //          "backslash"-operator also does with "high" matrices.
    //                          (and we will have those matrices in most cases)

    C_maker(X_transformed, W, iA);
    E_maker(X_transformed, Y, W, iB);

    vnl_matrix_inverse<double> myInverse(iA.GetVnlMatrix());
    vnl_vector<double> q = myInverse.pinverse(iB.size()) * iB;
    //'''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

    if (n > 1)
      q = (q + oldq) / 2;
    oldq = q;

    itk::Vector<double, 3> delta_t;
    delta_t[0] = q[3];
    delta_t[1] = q[4];
    delta_t[2] = q[5];

    Matrix3x3 delta_theta;
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

    // convert vnl matrices to itk matrices...
    Matrix3x3 U;
    Matrix3x3 V;

    for (int i = 0; i < 3; ++i)
    {
      for (int j = 0; j < 3; ++j)
      {
        U[i][j] = svd_delta_theta.U()[i][j];
        V[i][j] = svd_delta_theta.V()[i][j];
      }
    }

    Matrix3x3 delta_R = U * V.GetTranspose();

    // update rotation
    TransformationR = delta_R * TransformationR;

    // update translation
    TransformationT = delta_R * TransformationT + delta_t;

    // update moving points
    mitk::AnisotropicRegistrationCommon::TransformPoints(X, X_transformedNew, TransformationR, TransformationT);
    // calculate config change
    config_change = CalculateConfigChange(X_transformed, X_transformedNew);

    // swap the pointers the old set for the next iteration is
    // the new set of the last iteration
    vtkPoints *tmp = X_transformed;
    X_transformed = X_transformedNew;
    X_transformedNew = tmp;

  } while (config_change > Threshold && n < MaxIterations);

  // calculate FRE with current transform
  FRE = ComputeWeightedFRE(X, Y, Sigma_X, Sigma_Y, m_FRENormalizationFactor, W, TransformationR, TransformationT);

  MITK_DEBUG << "FRE after algorithm (prior to check with initial): " << FRE;

  // compare with FRE_initial
  if (initialFRE < FRE)
  {
    MITK_WARN << "FRE did not improve in anisotropic point registration function";
    TransformationR = initial_TransformationR;
    TransformationT = initial_TransformationT;
    FRE = initialFRE;
  }

  MITK_DEBUG << "FRE final: " << FRE;

  X_transformed->Delete();
  X_transformedNew->Delete();
}

void mitk::WeightedPointTransform::SetMovingPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_MovingPointSet = p;
}

void mitk::WeightedPointTransform::SetCovarianceMatricesMoving(const CovarianceMatrixList &matrices)
{
  m_CovarianceMatricesMoving = matrices;
}

void mitk::WeightedPointTransform::SetFixedPointSet(vtkSmartPointer<vtkPoints> p)
{
  m_FixedPointSet = p;
}

void mitk::WeightedPointTransform::SetCovarianceMatricesFixed(const CovarianceMatrixList &matrices)
{
  m_CovarianceMatricesFixed = matrices;
}

double mitk::WeightedPointTransform::CalculateConfigChange(vtkPoints *X, vtkPoints *X_new)
{
  double sum[3] = {0.0, 0.0, 0.0};
  double mean[3] = {0.0, 0.0, 0.0};
  double pX[3] = {0.0, 0.0, 0.0};
  double pX_new[3] = {0.0, 0.0, 0.0};

  // compute mean of the old point set and the first sum
  for (vtkIdType i = 0; i < X->GetNumberOfPoints(); ++i)
  {
    X->GetPoint(i, pX);
    X_new->GetPoint(i, pX_new);

    // first sum
    sum[0] += (pX_new[0] - pX[0]) * (pX_new[0] - pX[0]);
    sum[1] += (pX_new[1] - pX[1]) * (pX_new[1] - pX[1]);
    sum[2] += (pX_new[2] - pX[2]) * (pX_new[2] - pX[2]);

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

  for (vtkIdType i = 0; i < X->GetNumberOfPoints(); ++i)
  {
    X->GetPoint(i, pX);

    sum[0] += (pX[0] - mean[0]) * (pX[0] - mean[0]);
    sum[1] += (pX[1] - mean[1]) * (pX[1] - mean[1]);
    sum[2] += (pX[2] - mean[2]) * (pX[2] - mean[2]);
  }

  const double ls = sum[0] + sum[1] + sum[2];

  return sqrt(us / ls);
}
