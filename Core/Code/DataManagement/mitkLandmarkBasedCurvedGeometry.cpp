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


#include "mitkLandmarkBasedCurvedGeometry.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkBasedCurvedGeometry::LandmarkBasedCurvedGeometry() 
  : m_TargetLandmarks(NULL)
{
}

mitk::LandmarkBasedCurvedGeometry::~LandmarkBasedCurvedGeometry()
{

}

void mitk::LandmarkBasedCurvedGeometry::InitializeGeometry(Self * newGeometry) const
{
  Superclass::InitializeGeometry(newGeometry);
  newGeometry->SetTargetLandmarks(m_TargetLandmarks);
}
