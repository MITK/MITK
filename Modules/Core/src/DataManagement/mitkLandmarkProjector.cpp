/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkLandmarkProjector.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkProjector::LandmarkProjector()
  : m_InterpolatingAbstractTransform(nullptr),
    m_CompleteAbstractTransform(nullptr),
    m_FrameGeometry(nullptr),
    m_ParameterPlane(nullptr)
{
  m_FinalTargetLandmarks = m_WritableFinalTargetLandmarks = mitk::PointSet::DataType::PointsContainer::New();
  m_ProjectedLandmarks = mitk::PointSet::DataType::PointsContainer::New();
}

mitk::LandmarkProjector::~LandmarkProjector()
{
  if (m_InterpolatingAbstractTransform != nullptr)
    m_InterpolatingAbstractTransform->Delete();
}

void mitk::LandmarkProjector::SetInterpolatingAbstractTransform(vtkAbstractTransform *anInterpolatingAbstractTransform)
{
  if (m_InterpolatingAbstractTransform != anInterpolatingAbstractTransform)
  {
    m_InterpolatingAbstractTransform = anInterpolatingAbstractTransform;
    m_InterpolatingAbstractTransform->Register(nullptr);
    Modified();
    ComputeCompleteAbstractTransform();
  }
}

vtkAbstractTransform *mitk::LandmarkProjector::GetCompleteAbstractTransform() const
{
  return m_CompleteAbstractTransform;
}
