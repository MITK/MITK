/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPOINTOPERATION_H
#define MITKPOINTOPERATION_H

#include "mitkNumericTypes.h"
#include "mitkOperation.h"
#include <MitkCoreExports.h>

namespace mitk
{
  /** @brief Operation that handles all actions on one Point.
    * Stores everything for Adding, Moving and Deleting a Point.
    * @ingroup Undo
    */
  class MITKCORE_EXPORT PointOperation : public Operation
  {
  public:
    /** @brief Operation that handles all actions on one Point.
      * @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
      * @param point is the information of the point to add or is the information to change a point into
      * @param index is e.g. the position in a list which describes the element to change
      * @param selected
      * @param type
      */
    PointOperation(OperationType operationType,
                   Point3D point,
                   int index = -1,
                   bool selected = true,
                   PointSpecificationType type = PTUNDEFINED);

    /** @brief Operation that handles all actions on one Point.
      * @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
      * @param timeInMS
      * @param point is the information of the point to add or is the information to change a point into
      * @param index is e.g. the position in a list which describes the element to change
      * @param selected
      * @param type
      */
    PointOperation(OperationType operationType,
                   ScalarType timeInMS,
                   Point3D point,
                   int index = -1,
                   bool selected = true,
                   PointSpecificationType type = PTUNDEFINED);

    ~PointOperation() override;

    Point3D GetPoint();

    int GetIndex();

    bool GetSelected();

    PointSpecificationType GetPointType();

    ScalarType GetTimeInMS() const;

  private:
    Point3D m_Point;

    /** @brief to declare an index where to store the point in data */
    int m_Index;

    // to declare weather the point is selected or deselected
    bool m_Selected;

    /** @brief to describe the type of the point. See enum PointSpecification for different types */
    PointSpecificationType m_Type;

    ScalarType m_TimeInMS;
  };
} // namespace mitk
#endif /* MITKPOINTOPERATION_H*/
