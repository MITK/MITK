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


#include "mitkPointOperation.h"

mitk::PointOperation::PointOperation(OperationType operationType, Point3D point, int index, bool selected, PointSpecificationType type)
: mitk::Operation(operationType), m_Point(point), m_Index(index), m_Selected(selected), m_Type(type), m_TimeInMS(0)
{}

mitk::PointOperation::PointOperation(OperationType operationType, ScalarType timeInMS, Point3D point, int index, bool selected, PointSpecificationType type)
: mitk::Operation(operationType), m_Point(point), m_Index(index), m_Selected(selected), m_Type(type), m_TimeInMS(timeInMS)
{}

mitk::PointOperation::~PointOperation()
{
}

mitk::Point3D mitk::PointOperation::GetPoint()
{
  return m_Point;
}

int mitk::PointOperation::GetIndex()
{
  return m_Index;
}

bool mitk::PointOperation::GetSelected()
{
  return m_Selected;
}

mitk::PointSpecificationType mitk::PointOperation::GetPointType()
{
  return m_Type;
}

mitk::ScalarType mitk::PointOperation::GetTimeInMS() const
{
  return m_TimeInMS;
}
