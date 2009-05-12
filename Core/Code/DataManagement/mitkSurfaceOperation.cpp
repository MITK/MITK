/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSurfaceOperation.h"

mitk::SurfaceOperation::SurfaceOperation(mitk::OperationType operationType, vtkPolyData* polyData, unsigned int t)
: mitk::Operation(operationType), m_polyData(polyData), m_timeStep(t)
{
}


mitk::SurfaceOperation::~SurfaceOperation()
{
}

vtkPolyData* mitk::SurfaceOperation::GetVtkPolyData()
{
  return m_polyData;
}

unsigned int mitk::SurfaceOperation::GetTimeStep()
{
  return m_timeStep;
}
