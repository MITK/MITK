/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlaneOperation_h
#define mitkPlaneOperation_h

#include "mitkNumericTypes.h"
#include "mitkPointOperation.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * @brief Operation for setting a plane (defined by its origin and normal)
   *
   * @ingroup Undo
   */
  class MITKCORE_EXPORT PlaneOperation : public PointOperation
  {
  public:
    PlaneOperation(OperationType operationType, Point3D point, Vector3D normal);
    PlaneOperation(OperationType operationType, Point3D point, Vector3D axisVec0, Vector3D axisVec1);

    ~PlaneOperation() override;

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

} // namespace mitk

#endif
