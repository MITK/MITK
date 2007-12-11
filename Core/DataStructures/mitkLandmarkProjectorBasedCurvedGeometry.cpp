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


#include "mitkLandmarkProjectorBasedCurvedGeometry.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkProjectorBasedCurvedGeometry::LandmarkProjectorBasedCurvedGeometry() 
  : m_LandmarkProjector(NULL), m_InterpolatingAbstractTransform(NULL)
{
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

void mitk::LandmarkProjectorBasedCurvedGeometry::SetFrameGeometry(const mitk::Geometry3D* frameGeometry)
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

void mitk::LandmarkProjectorBasedCurvedGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);
  newGeometry->SetLandmarkProjector(m_LandmarkProjector);
  newGeometry->ComputeGeometry();
}
