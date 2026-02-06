/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkThinPlateSplineCurvedGeometry.h"
#include <vtkPoints.h>
#include <vtkThinPlateSplineTransform.h>

mitk::ThinPlateSplineCurvedGeometry::ThinPlateSplineCurvedGeometry() : Superclass()
{
  m_ThinPlateSplineTransform = vtkThinPlateSplineTransform::New();
  m_InterpolatingAbstractTransform.TakeReference(m_ThinPlateSplineTransform);

  m_VtkTargetLandmarks = vtkSmartPointer<vtkPoints>::New();
  m_VtkProjectedLandmarks = vtkSmartPointer<vtkPoints>::New();
  m_ThinPlateSplineTransform->SetInverseIterations(5000);
}

mitk::ThinPlateSplineCurvedGeometry::ThinPlateSplineCurvedGeometry(const ThinPlateSplineCurvedGeometry &other)
  : Superclass(other)
{
  m_ThinPlateSplineTransform = vtkThinPlateSplineTransform::New();
  m_InterpolatingAbstractTransform.TakeReference(m_ThinPlateSplineTransform);
  m_ThinPlateSplineTransform->SetInverseIterations(5000);

  m_VtkTargetLandmarks = vtkSmartPointer<vtkPoints>::New();
  m_VtkProjectedLandmarks = vtkSmartPointer<vtkPoints>::New();

  if (other.m_VtkTargetLandmarks != nullptr)
    m_VtkTargetLandmarks->DeepCopy(other.m_VtkTargetLandmarks);

  if (other.m_VtkProjectedLandmarks != nullptr)
    m_VtkProjectedLandmarks->DeepCopy(other.m_VtkProjectedLandmarks);

  this->SetSigma(other.GetSigma());

  if (m_LandmarkProjector.IsNotNull())
  {
    m_LandmarkProjector->SetInterpolatingAbstractTransform(m_ThinPlateSplineTransform);
    SetVtkAbstractTransform(m_LandmarkProjector->GetCompleteAbstractTransform());
  }
}

mitk::ThinPlateSplineCurvedGeometry::~ThinPlateSplineCurvedGeometry()
{
}

bool mitk::ThinPlateSplineCurvedGeometry::IsValid() const
{
  return m_TargetLandmarks.IsNotNull() && (m_TargetLandmarks->Size() >= 3) && m_LandmarkProjector.IsNotNull();
}

void mitk::ThinPlateSplineCurvedGeometry::SetSigma(double sigma)
{
  m_ThinPlateSplineTransform->SetSigma(sigma);
}

double mitk::ThinPlateSplineCurvedGeometry::GetSigma() const
{
  return m_ThinPlateSplineTransform->GetSigma();
}

void mitk::ThinPlateSplineCurvedGeometry::ComputeGeometry()
{
  Superclass::ComputeGeometry();

  const mitk::PointSet::DataType::PointsContainer *finalTargetLandmarks, *projectedTargetLandmarks;

  finalTargetLandmarks = m_LandmarkProjector->GetFinalTargetLandmarks();
  projectedTargetLandmarks = m_LandmarkProjector->GetProjectedLandmarks();

  mitk::PointSet::DataType::PointsContainer::ConstIterator targetIt, projectedIt;

  targetIt = finalTargetLandmarks->Begin();
  projectedIt = projectedTargetLandmarks->Begin();

  // initialize Thin-Plate-Spline
  m_VtkTargetLandmarks->Reset();
  m_VtkProjectedLandmarks->Reset();
  vtkIdType id;
  int size = finalTargetLandmarks->Size();
  for (id = 0; id < size; ++id, ++targetIt, ++projectedIt)
  {
    const mitk::PointSet::PointType &target = targetIt->Value();
    m_VtkTargetLandmarks->InsertPoint(id, target[0], target[1], target[2]);
    const mitk::PointSet::PointType &projected = projectedIt->Value();
    m_VtkProjectedLandmarks->InsertPoint(id, projected[0], projected[1], projected[2]);
  }
  m_VtkTargetLandmarks->Modified();
  m_VtkProjectedLandmarks->Modified();

  m_ThinPlateSplineTransform->SetSourceLandmarks(m_VtkProjectedLandmarks);
  m_ThinPlateSplineTransform->SetTargetLandmarks(m_VtkTargetLandmarks);
}
