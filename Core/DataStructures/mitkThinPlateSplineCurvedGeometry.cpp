/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkThinPlateSplineCurvedGeometry.h"
#include <vtkThinPlateSplineTransform.h>
#include <vtkPoints.h>

mitk::ThinPlateSplineCurvedGeometry::ThinPlateSplineCurvedGeometry()
{
  m_InterpolatingAbstractTransform = m_ThinPlateSplineTransform = vtkThinPlateSplineTransform::New();

  m_VtkTargetLandmarks = vtkPoints::New();
  m_VtkProjectedLandmarks = vtkPoints::New();
  m_ThinPlateSplineTransform->SetInverseIterations(5000);
}

mitk::ThinPlateSplineCurvedGeometry::~ThinPlateSplineCurvedGeometry()
{
}

bool mitk::ThinPlateSplineCurvedGeometry::IsValid() const
{
  return m_TargetLandmarks.IsNotNull() && (m_TargetLandmarks->Size() >= 3) && m_LandmarkProjector.IsNotNull();
}

void mitk::ThinPlateSplineCurvedGeometry::SetSigma(float sigma)
{
  m_ThinPlateSplineTransform->SetSigma(sigma);
}

float mitk::ThinPlateSplineCurvedGeometry::GetSigma() const
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

mitk::AffineGeometryFrame3D::Pointer mitk::ThinPlateSplineCurvedGeometry::Clone() const
{
  Self::Pointer newGeometry = Self::New();
  newGeometry->Initialize();
  InitializeGeometry(newGeometry);
  return newGeometry.GetPointer();
}

void mitk::ThinPlateSplineCurvedGeometry::InitializeGeometry(Self * newGeometry) const
{
  newGeometry->SetSigma(GetSigma());
  Superclass::InitializeGeometry(newGeometry);
}
