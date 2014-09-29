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

#include "mitkThinPlateSplineCurvedGeometry.h"
#include <vtkThinPlateSplineTransform.h>
#include <vtkPoints.h>

mitk::ThinPlateSplineCurvedGeometry::ThinPlateSplineCurvedGeometry()
  : Superclass()
{
  m_InterpolatingAbstractTransform = m_ThinPlateSplineTransform = vtkThinPlateSplineTransform::New();

  m_VtkTargetLandmarks = vtkPoints::New();
  m_VtkProjectedLandmarks = vtkPoints::New();
  m_ThinPlateSplineTransform->SetInverseIterations(5000);
}

mitk::ThinPlateSplineCurvedGeometry::ThinPlateSplineCurvedGeometry(const ThinPlateSplineCurvedGeometry& other ) : Superclass(other)
{
  this->SetSigma(other.GetSigma());
}

mitk::ThinPlateSplineCurvedGeometry::~ThinPlateSplineCurvedGeometry()
{
  // don't need to delete m_ThinPlateSplineTransform, because it is
  // the same as m_InterpolatingAbstractTransform, which will be deleted
  // by the superclass.

  if(m_VtkTargetLandmarks!=NULL)
    m_VtkTargetLandmarks->Delete();
  if(m_VtkProjectedLandmarks!=NULL)
    m_VtkProjectedLandmarks->Delete();
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

  targetIt    = finalTargetLandmarks->Begin();
  projectedIt = projectedTargetLandmarks->Begin();

  //initialize Thin-Plate-Spline
  m_VtkTargetLandmarks->Reset();
  m_VtkProjectedLandmarks->Reset();
  vtkIdType id;
  int size=finalTargetLandmarks->Size();
  for(id=0; id < size; ++id, ++targetIt, ++projectedIt)
  {
    const mitk::PointSet::PointType& target = targetIt->Value();
    m_VtkTargetLandmarks->InsertPoint(id, target[0], target[1], target[2]);
    const mitk::PointSet::PointType& projected = projectedIt->Value();
    m_VtkProjectedLandmarks->InsertPoint(id, projected[0], projected[1], projected[2]);
  }
  m_VtkTargetLandmarks->Modified();
  m_VtkProjectedLandmarks->Modified();

  m_ThinPlateSplineTransform->SetSourceLandmarks(m_VtkProjectedLandmarks);
  m_ThinPlateSplineTransform->SetTargetLandmarks(m_VtkTargetLandmarks);
}

itk::LightObject::Pointer mitk::ThinPlateSplineCurvedGeometry::InternalClone() const
{
  mitk::BaseGeometry::Pointer newGeometry = new Self(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}
