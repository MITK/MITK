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


#ifndef MITKPLANARFIGUREOPERATION_H
#define MITKPLANARFIGUREOPERATION_H

#include <MitkPlanarFigureExports.h>
#include "mitkOperation.h"
#include "mitkPoint.h"



namespace mitk {

//##Documentation
//## @brief Operation that handles all actions on one PlanarFigure control point.
//##
//## Stores everything for Adding, Moving and Deleting a Point.
//## @ingroup Undo
class MITKPLANARFIGURE_EXPORT PlanarFigureOperation : public Operation
{
public:
  //##Documentation
  //##@brief Operation that handles all actions on one control point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  PlanarFigureOperation(OperationType operationType,  Point2D point, int index = -1);
  virtual ~PlanarFigureOperation();

  Point2D GetPoint();

  int GetIndex();

private:
  Point2D m_Point;

  //##Documentation
  //##@brief to declare an index where to store the point in data
  int m_Index;
};

}//namespace mitk
#endif /* MITKPLANARFIGUREOPERATION_H*/


