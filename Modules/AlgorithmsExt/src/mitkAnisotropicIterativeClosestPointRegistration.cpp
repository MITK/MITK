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
#include "mitkAnisotropicIterativeClosestPointRegistration.h"
#include "mitkAnisotropicRegistrationCommon.h"
#include "mitkWeightedPointTransform.h"
#include <mitkProgressBar.h>
#include <mitkSurface.h>
// VTK
#include <vtkIdList.h>
#include <vtkKdTree.h>
#include <vtkKdTreePointLocator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
// STL pair
#include <utility>

/** \brief Comperator implementation used to sort the CorrespondenceList in the
  *        trimmed version of the AnisotropicIterativeClosestPointRegistration.
  */
struct AICPComperator
{
  typedef std::pair<unsigned int, double> Correspondence;

  bool operator()(const Correspondence &a, const Correspondence &b) { return (a.second < b.second); }
} AICPComp;

mitk::AnisotropicIterativeClosestPointRegistration::AnisotropicIterativeClosestPointRegistration()
  : m_MaxIterations(1000),
    m_Threshold(0.000001),
    m_FRENormalizationFactor(1.0),
    m_SearchRadius(30.0),
    m_MaxIterationsInWeightedPointTransform(1000),
    m_FRE(0.0),
    m_TrimmFactor(0.0),
    m_NumberOfIterations(0),
    m_MovingSurface(nullptr),
    m_FixedSurface(nullptr),
    m_WeightedPointTransform(mitk::WeightedPointTransform::New())
{
}

mitk::AnisotropicIterativeClosestPointRegistration::~AnisotropicIterativeClosestPointRegistration()
{
}

void mitk::AnisotropicIterativeClosestPointRegistration::ComputeCorrespondences(vtkPoints *X,
                                                                                vtkPoints *Z,
                                                                                vtkKdTreePointLocator *Y,
                                                                                const CovarianceMatrixList &sigma_X,
                                                                                const CovarianceMatrixList &sigma_Y,
                                                                                CovarianceMatrixList &sigma_Z,
                                                                                CorrespondenceList &correspondences,
                                                                                const double radius)
{
  typedef itk::Matrix<double, 3, 3> WeightMatrix;

#pragma omp parallel for
  for (int i = 0; i < X->GetNumberOfPoints(); ++i)
  {
    vtkIdType bestIdx = 0;
    mitk::Vector3D x;
    mitk::Vector3D y;
    double bestDist = std::numeric_limits<double>::max();
    vtkIdList *ids = vtkIdList::New();
    double r = radius;
    double p[3];
    // get point
    X->GetPoint(i, p);
    // fill vector
    x[0] = p[0];
    x[1] = p[1];
    x[2] = p[2];

    // double the radius till we find at least one point
    while (ids->GetNumberOfIds() <= 0)
    {
      Y->FindPointsWithinRadius(r, p, ids);
      r *= 2.0;
    }

    // loop over the points in the sphere and find the point with the
    // minimal weighted squared distance
    for (vtkIdType j = 0; j < ids->GetNumberOfIds(); ++j)
    {
      // get id
      const vtkIdType id = ids->GetId(j);
      // compute weightmatrix
      WeightMatrix m = mitk::AnisotropicRegistrationCommon::CalculateWeightMatrix(sigma_X[i], sigma_Y[id]);
      // point of the fixed data set
      Y->GetDataSet()->GetPoint(id, p);

      // fill mitk vector
      y[0] = p[0];
      y[1] = p[1];
      y[2] = p[2];

      const mitk::Vector3D res = m * (x - y);

      const double dist = res[0] * res[0] + res[1] * res[1] + res[2] * res[2];

      if (dist < bestDist)
      {
        bestDist = dist;
        bestIdx = id;
      }
    }

    // save correspondences of the fixed point set
    Y->GetDataSet()->GetPoint(bestIdx, p);
    Z->SetPoint(i, p);
    sigma_Z[i] = sigma_Y[bestIdx];

    Correspondence _pair(i, bestDist);
    correspondences[i] = _pair;

    ids->Delete();
  }
}

void mitk::AnisotropicIterativeClosestPointRegistration::Update()
{
  unsigned int k = 0;
  unsigned int numberOfTrimmedPoints = 0;
  double diff = 0.0;
  double FRE_new = std::numeric_limits<double>::max();
  // Moving pointset
  vtkPoints *X = vtkPoints::New();
  // Correspondences
  vtkPoints *Z = vtkPoints::New();
  // Covariance matrices of the pointset X
  CovarianceMatrixList &Sigma_X = m_CovarianceMatricesMovingSurface;
  // Covariance matrices of the pointset Y
  CovarianceMatrixList &Sigma_Y = m_CovarianceMatricesFixedSurface;
  // Covariance matrices of the correspondences
  CovarianceMatrixList Sigma_Z;
  // transform of the current iteration
  Rotation RotationNew;
  Translation TranslationNew;
  // corresponding indizes with distance
  CorrespondenceList distanceList;
  // sorted datasets used if trimming is enabled
  vtkPoints *X_sorted = vtkPoints::New();
  vtkPoints *Z_sorted = vtkPoints::New();
  CovarianceMatrixList Sigma_X_sorted;
  CovarianceMatrixList Sigma_Z_sorted;

  // create kdtree for correspondence search
  vtkKdTreePointLocator *Y = vtkKdTreePointLocator::New();
  Y->SetDataSet(m_FixedSurface->GetVtkPolyData());
  Y->BuildLocator();

  // initialize local variables
  // copy the moving pointset to prevent to modify it
  X->DeepCopy(m_MovingSurface->GetVtkPolyData()->GetPoints());
  // initialize size of the correspondences
  Z->SetNumberOfPoints(X->GetNumberOfPoints());
  // size of the corresponding matrices
  Sigma_Z.resize(X->GetNumberOfPoints());
  distanceList.resize(X->GetNumberOfPoints());
  RotationNew.SetIdentity();
  TranslationNew.Fill(0.0);

  // reset members
  m_FRE = std::numeric_limits<double>::max();
  m_Rotation.SetIdentity();
  m_Translation.Fill(0.0);

  // compute number of correspondences based
  // on the trimmfactor
  if (m_TrimmFactor > 0.0)
  {
    numberOfTrimmedPoints = X->GetNumberOfPoints() * m_TrimmFactor;
  }

  // initialize the sizes of the sorted datasets
  // used in the trimmed version of the algorithm
  Sigma_Z_sorted.resize(numberOfTrimmedPoints);
  Sigma_X_sorted.resize(numberOfTrimmedPoints);
  X_sorted->SetNumberOfPoints(numberOfTrimmedPoints);
  Z_sorted->SetNumberOfPoints(numberOfTrimmedPoints);

  // initialize the progress bar
  unsigned int steps = m_MaxIterations;
  unsigned int stepSize = m_MaxIterations / 10;
  mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);

  do
  {
    // reset innerloop
    double currSearchRadius = m_SearchRadius;
    unsigned int radiusDoubled = 0;

    k = k + 1;

    MITK_DEBUG << "iteration: " << k;

    do
    {
      // search correspondences
      ComputeCorrespondences(X, Z, Y, Sigma_X, Sigma_Y, Sigma_Z, distanceList, currSearchRadius);

      // tmp pointers
      vtkPoints *X_k = X;
      vtkPoints *Z_k = Z;
      CovarianceMatrixList *Sigma_Z_k = &Sigma_Z;
      CovarianceMatrixList *Sigma_X_k = &Sigma_X;

      // sort the correspondences depending on their
      // distance, if trimming is enabled
      if (m_TrimmFactor > 0.0)
      {
        std::sort(distanceList.begin(), distanceList.end(), AICPComp);
        // map correspondences to the data arrays
        for (unsigned int i = 0; i < numberOfTrimmedPoints; ++i)
        {
          const int idx = distanceList[i].first;
          Sigma_Z_sorted[i] = Sigma_Z[idx];
          Sigma_X_sorted[i] = Sigma_X[idx];
          Z_sorted->SetPoint(i, Z->GetPoint(idx));
          X_sorted->SetPoint(i, X->GetPoint(idx));
        }
        // assign pointers
        X_k = X_sorted;
        Z_k = Z_sorted;
        Sigma_X_k = &Sigma_X_sorted;
        Sigma_Z_k = &Sigma_Z_sorted;
      }

      // compute weighted transformation
      // set parameters
      m_WeightedPointTransform->SetMovingPointSet(X_k);
      m_WeightedPointTransform->SetFixedPointSet(Z_k);
      m_WeightedPointTransform->SetCovarianceMatricesMoving(*Sigma_X_k);
      m_WeightedPointTransform->SetCovarianceMatricesFixed(*Sigma_Z_k);
      m_WeightedPointTransform->SetMaxIterations(m_MaxIterationsInWeightedPointTransform);
      m_WeightedPointTransform->SetFRENormalizationFactor(m_FRENormalizationFactor);

      // run computation
      m_WeightedPointTransform->ComputeTransformation();
      // retrieve result
      RotationNew = m_WeightedPointTransform->GetTransformR();
      TranslationNew = m_WeightedPointTransform->GetTransformT();
      FRE_new = m_WeightedPointTransform->GetFRE();

      // double the radius
      radiusDoubled += 1;
      currSearchRadius *= 2.0;

      // sanity check to prevent endless loop
      if (radiusDoubled >= 20)
      {
        mitkThrow() << "Radius doubled 20 times, preventing endless loop, check input and search radius";
      }

      // termination constraint
      diff = m_FRE - FRE_new;

    } while (diff < -1.0e-3); // increase radius as long as the FRE grows

    MITK_DEBUG << "FRE:" << m_FRE << ", FRE_new: " << FRE_new;
    // transform points and propagate matrices
    mitk::AnisotropicRegistrationCommon::TransformPoints(X, X, RotationNew, TranslationNew);
    mitk::AnisotropicRegistrationCommon::PropagateMatrices(Sigma_X, Sigma_X, RotationNew);

    // update global transformation
    m_Rotation = RotationNew * m_Rotation;
    m_Translation = RotationNew * m_Translation + TranslationNew;

    MITK_DEBUG << "diff:" << diff;
    // update FRE
    m_FRE = FRE_new;

    // update the progressbar. Just use the half every 2nd iteration
    // to use a simulated endless progress bar since we don't have
    // a fixed amount of iterations
    stepSize = (k % 2 == 0) ? stepSize / 2 : stepSize;
    stepSize = (stepSize == 0) ? 1 : stepSize;
    mitk::ProgressBar::GetInstance()->Progress(stepSize);

  } while (diff > m_Threshold && k < m_MaxIterations);

  m_NumberOfIterations = k;

  // finish the progress bar if there are more steps
  // left than iterations used
  if (k < steps)
    mitk::ProgressBar::GetInstance()->Progress(steps);

  // free memory
  Y->Delete();
  Z->Delete();
  X->Delete();
  X_sorted->Delete();
  Z_sorted->Delete();
}
