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
#include "mitkLogMacros.h"

namespace mitk
{

PlaneOperation
::PlaneOperation( OperationType operationType, Point3D point, Vector3D normal )
: PointOperation( operationType, point ), m_Normal( normal ), m_AreAxisDefined (false )
{
}

PlaneOperation
::PlaneOperation( OperationType operationType, Point3D point, Vector3D axisVec0, Vector3D axisVec1 )
: PointOperation( operationType, point ), m_AxisVec0( axisVec0 ), m_AxisVec1( axisVec1 ),  m_AreAxisDefined (true )
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

Vector3D mitk::PlaneOperation::GetAxisVec0()
{
   return m_AxisVec0;
}

Vector3D mitk::PlaneOperation::GetAxisVec1()
{
   return m_AxisVec1;
}

bool mitk::PlaneOperation::AreAxisDefined()
 {
    return m_AreAxisDefined;
 }

} // namespace mitk

