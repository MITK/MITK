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


#ifndef mitkRestorePlanePositionOperation_h_Included
#define mitkRestorePlanePositionOperation_h_Included

#include "mitkCommon.h"
#include "mitkPointOperation.h"
#include "mitkNumericTypes.h"



namespace mitk {

//##Documentation
//## TODO

class MITKCORE_EXPORT RestorePlanePositionOperation : public Operation
{
  public:
  //##Documentation
  //##@brief Operation that handles all actions on one Point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  //PointOperation(OperationType operationType,  Point3D point, int index = -1, bool selected = true, PointSpecificationType type = PTUNDEFINED);

    RestorePlanePositionOperation(OperationType operationType, ScalarType width, ScalarType height, Vector3D spacing, unsigned int pos, Vector3D direction, AffineTransform3D::Pointer transform);

  virtual ~RestorePlanePositionOperation();

  Vector3D GetDirectionVector();

  ScalarType GetWidth();

  ScalarType GetHeight();

  Vector3D GetSpacing();

  unsigned int GetPos();

  AffineTransform3D::Pointer GetTransform();

  private:

    Vector3D m_Spacing;

    Vector3D m_DirectionVector;

    ScalarType m_Width;

    ScalarType m_Height;

    unsigned int m_Pos;

    AffineTransform3D::Pointer m_Transform;
};
}//namespace mitk
#endif


