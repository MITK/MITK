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


#include "mitkPlaneLandmarkProjector.h"

mitk::PlaneLandmarkProjector::PlaneLandmarkProjector() : m_ProjectionPlane(NULL)
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

void mitk::PlaneLandmarkProjector::ProjectLandmarks(
  const mitk::PointSet::DataType::PointsContainer* targetLandmarks)
{
  if(m_ProjectionPlane.IsNull())
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

  unsigned int size=targetLandmarks->Size();
  m_ProjectedLandmarks->Reserve(size);
  for(id=0, pointiterator=start;id<size;++id, ++pointiterator)
  { 
    point = pointiterator->Value();
    m_ProjectionPlane->Project(point, projectedPoint);
    
    m_ProjectedLandmarks->InsertElement(id, projectedPoint);
  }
}
