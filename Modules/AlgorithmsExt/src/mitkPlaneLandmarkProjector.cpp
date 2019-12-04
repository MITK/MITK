/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPlaneLandmarkProjector.h"

mitk::PlaneLandmarkProjector::PlaneLandmarkProjector() : m_ProjectionPlane(nullptr)
{
}

mitk::PlaneLandmarkProjector::~PlaneLandmarkProjector()
{
}

void mitk::PlaneLandmarkProjector::ComputeCompleteAbstractTransform()
{
  // we do not need any special pre- or post-transform, thus simply set
  // the InterpolatingAbstractTransform as m_CompleteAbstractTransform.
  m_CompleteAbstractTransform = GetInterpolatingAbstractTransform();
}

void mitk::PlaneLandmarkProjector::ProjectLandmarks(const mitk::PointSet::DataType::PointsContainer *targetLandmarks)
{
  if (m_ProjectionPlane.IsNull())
  {
    itkExceptionMacro(<< "m_ProjectionPlane is not set.");
  }

  m_ParameterPlane = m_ProjectionPlane;

  m_ProjectedLandmarks->Initialize();
  m_FinalTargetLandmarks = targetLandmarks;

  mitk::PointSet::DataType::PointsContainer::ConstIterator pointiterator, start = targetLandmarks->Begin();
  mitk::PointSet::DataType::PointsContainer::ElementIdentifier id;

  mitk::Point3D point;
  mitk::Point3D projectedPoint;

  unsigned int size = targetLandmarks->Size();
  m_ProjectedLandmarks->Reserve(size);
  for (id = 0, pointiterator = start; id < size; ++id, ++pointiterator)
  {
    point = pointiterator->Value();
    m_ProjectionPlane->Project(point, projectedPoint);

    m_ProjectedLandmarks->InsertElement(id, projectedPoint);
  }
}
