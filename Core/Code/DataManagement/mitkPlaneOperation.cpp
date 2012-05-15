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


#include "mitkPlaneOperation.h"

namespace mitk
{

PlaneOperation
::PlaneOperation( OperationType operationType, Point3D point, Vector3D normal )
: PointOperation( operationType, point ), m_Normal( normal )
{
}

PlaneOperation
::~PlaneOperation()
{
}

Vector3D mitk::PlaneOperation::GetNormal()
{
  return m_Normal;
}

} // namespace mitk

