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


#include "mitkLandmarkBasedCurvedGeometry.h"
#include <vtkAbstractTransform.h>

mitk::LandmarkBasedCurvedGeometry::LandmarkBasedCurvedGeometry()
  : m_TargetLandmarks(NULL)
{
}

mitk::LandmarkBasedCurvedGeometry::LandmarkBasedCurvedGeometry(const LandmarkBasedCurvedGeometry& other)
  : Superclass(other)
{
  SetTargetLandmarks(other.m_TargetLandmarks);
}

mitk::LandmarkBasedCurvedGeometry::~LandmarkBasedCurvedGeometry()
{

}
