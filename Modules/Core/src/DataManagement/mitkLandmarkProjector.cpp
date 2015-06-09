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

#include "mitkLandmarkProjector.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkProjector::LandmarkProjector()
  : m_InterpolatingAbstractTransform(nullptr), m_CompleteAbstractTransform(nullptr), m_FrameGeometry(nullptr), m_ParameterPlane(nullptr)
{
  m_FinalTargetLandmarks = m_WritableFinalTargetLandmarks = mitk::PointSet::DataType::PointsContainer::New();
  m_ProjectedLandmarks   = mitk::PointSet::DataType::PointsContainer::New();
}

mitk::LandmarkProjector::~LandmarkProjector()
{
  if(m_InterpolatingAbstractTransform != nullptr)
    m_InterpolatingAbstractTransform->Delete();
}

void mitk::LandmarkProjector::SetInterpolatingAbstractTransform(vtkAbstractTransform* anInterpolatingAbstractTransform)
{
  if(m_InterpolatingAbstractTransform != anInterpolatingAbstractTransform)
  {
    m_InterpolatingAbstractTransform = anInterpolatingAbstractTransform;
    m_InterpolatingAbstractTransform->Register(nullptr);
    Modified();
    ComputeCompleteAbstractTransform();
  }
}

vtkAbstractTransform* mitk::LandmarkProjector::GetCompleteAbstractTransform() const
{
  return m_CompleteAbstractTransform;
}
