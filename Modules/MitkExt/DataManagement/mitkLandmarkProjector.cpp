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

#include "mitkLandmarkProjector.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkProjector::LandmarkProjector()
  : m_InterpolatingAbstractTransform(NULL), m_CompleteAbstractTransform(NULL), m_FrameGeometry(NULL), m_ParameterPlane(NULL)
{
  m_FinalTargetLandmarks = m_WritableFinalTargetLandmarks = mitk::PointSet::DataType::PointsContainer::New();
  m_ProjectedLandmarks   = mitk::PointSet::DataType::PointsContainer::New();
}

mitk::LandmarkProjector::~LandmarkProjector()
{
  if(m_InterpolatingAbstractTransform != NULL)
    m_InterpolatingAbstractTransform->Delete();
}

void mitk::LandmarkProjector::SetInterpolatingAbstractTransform(vtkAbstractTransform* anInterpolatingAbstractTransform)
{
  if(m_InterpolatingAbstractTransform != anInterpolatingAbstractTransform)
  {
    m_InterpolatingAbstractTransform = anInterpolatingAbstractTransform;
    m_InterpolatingAbstractTransform->Register(NULL);
    Modified();
    ComputeCompleteAbstractTransform();
  }
}

vtkAbstractTransform* mitk::LandmarkProjector::GetCompleteAbstractTransform() const
{
  return m_CompleteAbstractTransform;
}
