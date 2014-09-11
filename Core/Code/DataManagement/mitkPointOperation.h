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


#ifndef MITKPOINTOPERATION_H
#define MITKPOINTOPERATION_H

#include <MitkCoreExports.h>
#include "mitkOperation.h"
#include "mitkNumericTypes.h"



namespace mitk {

//##Documentation
//## @brief Operation that handles all actions on one Point.
//##
//## Stores everything for Adding, Moving and Deleting a Point.
//## @ingroup Undo
class MITK_CORE_EXPORT PointOperation : public Operation
{
  public:
  //##Documentation
  //##@brief Operation that handles all actions on one Point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  PointOperation(OperationType operationType,  Point3D point, int index = -1, bool selected = true, PointSpecificationType type = PTUNDEFINED);

  //##Documentation
  //##@brief Operation that handles all actions on one Point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  PointOperation(OperationType operationType,  ScalarType timeInMS, Point3D point, int index = -1, bool selected = true, PointSpecificationType type = PTUNDEFINED);

  virtual ~PointOperation();

  Point3D GetPoint();

  int GetIndex();

  bool GetSelected();

  PointSpecificationType GetPointType();

  ScalarType GetTimeInMS() const;

  private:
  Point3D m_Point;

  //##Documentation
  //##@brief to declare an index where to store the point in data
  int m_Index;

  //to declare weather the point is selected or deselected
  bool m_Selected;

  //##Documentation
  //##@brief to describe the type of the point. See enum PointSpecification for different types
  PointSpecificationType m_Type;

  ScalarType m_TimeInMS;
};
}//namespace mitk
#endif /* MITKPOINTOPERATION_H*/


