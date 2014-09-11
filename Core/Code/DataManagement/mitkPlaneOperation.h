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


#ifndef MITKPlaneOperation_H
#define MITKPlaneOperation_H

#include <MitkCoreExports.h>
#include "mitkPointOperation.h"
#include "mitkNumericTypes.h"



namespace mitk {

/**
 * @brief Operation for setting a plane (defined by its origin and normal)
 *
 * @ingroup Undo
 */
class MITK_CORE_EXPORT PlaneOperation : public PointOperation
{
public:
  PlaneOperation( OperationType operationType, Point3D point, Vector3D normal );
  PlaneOperation( OperationType operationType, Point3D point, Vector3D axisVec0, Vector3D axisVec1 );

  virtual ~PlaneOperation();

  Vector3D GetNormal();
  Vector3D GetAxisVec0();
  Vector3D GetAxisVec1();
  bool AreAxisDefined();

private:
  Vector3D m_Normal;
  Vector3D m_AxisVec0;
  Vector3D m_AxisVec1;
  bool m_AreAxisDefined;

};

} //namespace mitk


#endif /* MITKPlaneOperation_H */


