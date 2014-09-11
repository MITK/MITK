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


#include "mitkLandmarkProjectorBasedCurvedGeometry.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkProjectorBasedCurvedGeometry::LandmarkProjectorBasedCurvedGeometry()
  : AbstractTransformGeometry(), m_LandmarkProjector(NULL), m_InterpolatingAbstractTransform(NULL), m_TargetLandmarks(NULL)
{
}

mitk::LandmarkProjectorBasedCurvedGeometry::LandmarkProjectorBasedCurvedGeometry(const mitk::LandmarkProjectorBasedCurvedGeometry& other) : Superclass(other)
{
  SetTargetLandmarks(other.m_TargetLandmarks);
  this->SetLandmarkProjector(other.m_LandmarkProjector);
  this->ComputeGeometry();
}

mitk::LandmarkProjectorBasedCurvedGeometry::~LandmarkProjectorBasedCurvedGeometry()
{
  if(m_InterpolatingAbstractTransform!=NULL)
    m_InterpolatingAbstractTransform->Delete();
}

void mitk::LandmarkProjectorBasedCurvedGeometry::SetLandmarkProjector(mitk::LandmarkProjector* aLandmarkProjector)
{
  itkDebugMacro("setting LandmarkProjector to " << aLandmarkProjector );
  if(m_LandmarkProjector != aLandmarkProjector)
  {
    m_LandmarkProjector = aLandmarkProjector;
    if(m_LandmarkProjector.IsNotNull())
    {
      if(m_FrameGeometry.IsNotNull())
        m_LandmarkProjector->SetFrameGeometry(m_FrameGeometry);

      if(m_InterpolatingAbstractTransform == NULL)
      {
        itkWarningMacro(<<"m_InterpolatingAbstractTransform not set.");
      }
      m_LandmarkProjector->SetInterpolatingAbstractTransform(GetInterpolatingAbstractTransform());

      SetVtkAbstractTransform(m_LandmarkProjector->GetCompleteAbstractTransform());
    }
    Modified();
  }
}

void mitk::LandmarkProjectorBasedCurvedGeometry::SetFrameGeometry(const mitk::BaseGeometry* frameGeometry)
{
  Superclass::SetFrameGeometry(frameGeometry);
  if(m_LandmarkProjector.IsNotNull())
    m_LandmarkProjector->SetFrameGeometry(frameGeometry);
}

void mitk::LandmarkProjectorBasedCurvedGeometry::ComputeGeometry()
{
  if(m_LandmarkProjector.IsNull())
  {
    itkExceptionMacro(<< "m_LandmarkProjector is not set.");
  }
  m_LandmarkProjector->ProjectLandmarks(m_TargetLandmarks);
  SetPlane(m_LandmarkProjector->GetParameterPlane());
}
itk::LightObject::Pointer mitk::LandmarkProjectorBasedCurvedGeometry::InternalClone() const
{
  mitk::BaseGeometry::Pointer newGeometry = new LandmarkProjectorBasedCurvedGeometry(*this);
  newGeometry->UnRegister();
  return newGeometry.GetPointer();
}
