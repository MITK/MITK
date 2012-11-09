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
: PointOperation( operationType, point ), m_Normal( normal ), m_AxisVec0IsDefined (false )
{
}

PlaneOperation
::PlaneOperation( OperationType operationType, Point3D point, Vector3D normal, Vector3D axisVec0 )
: PointOperation( operationType, point ), m_Normal( normal ), m_AxisVec0( axisVec0 ),  m_AxisVec0IsDefined (true )
{
   // axis vector HAS to be 90° to normalVector
   // check this..

   float angle = acos (m_Normal[0]*m_AxisVec0[0] + m_Normal[1]*m_AxisVec0[1] + m_Normal[2]*m_AxisVec0[2]);
   angle *= 180.0 / vnl_math::pi;
   if (  (abs(90-angle)) > 0.00001)
   {
      MITK_WARN << "Normal and Axisvector need to at be 90°! Here it is " << angle ;
   }



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

bool mitk::PlaneOperation::IsAxisVec0Defined()
{
   return m_AxisVec0IsDefined;
}

} // namespace mitk

