/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKAXISROTATIONOPERATION_H
#define MITKAXISROTATIONOPERATION_H

#include "mitkCommon.h"
#include "mitkPointOperation.h"
#include "mitkVector.h"



namespace mitk {

//##Documentation
//## @brief Operation that handles all actions on one Point.
//##
//## Stores everything for Adding, Moving and Deleting a Point.
//## @ingroup Undo
class MITK_CORE_EXPORT AxisRotationOperation : public PointOperation
{
  public:
  //##Documentation
  //##@brief Operation that handles all actions on one Point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  //PointOperation(OperationType operationType,  Point3D point, int index = -1, bool selected = true, PointSpecificationType type = PTUNDEFINED);

  AxisRotationOperation(OperationType operationType, Point3D center, Vector3D xaxis, Vector3D yaxis, float width, float height, Vector3D spacing/*, Point3D pointOfRotation*/);
 
  //##Documentation
  //##@brief Operation that handles all actions on one Point.
  //##
  //## @param operationType is the type of the operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
  //## @param point is the information of the point to add or is the information to change a point into
  //## @param index is e.g. the position in a list which describes the element to change
  //PointOperation(OperationType operationType,  ScalarType timeInMS, Point3D point, int index = -1, bool selected = true, PointSpecificationType type = PTUNDEFINED);

  virtual ~AxisRotationOperation();

  Vector3D GetXAxis();

  Vector3D GetYAxis();

  float GetWidth();

  float GetHeight();

  Vector3D GetSpacing();

  Point3D GetPointOfRotation();

  private:

    
    Vector3D m_XAxis;

    Vector3D m_YAxis;

    Vector3D m_Spacing;

    float m_Width;

    float m_Height;

    Point3D m_PointOfRotation;

};
}//namespace mitk
#endif /* MITKAXISROTATIONOPERATION_H*/


