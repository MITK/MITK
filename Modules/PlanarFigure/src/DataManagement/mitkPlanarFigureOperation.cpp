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


#include "mitkPlanarFigureOperation.h"

mitk::PlanarFigureOperation::PlanarFigureOperation(OperationType operationType, Point2D point, int index)
: mitk::Operation(operationType), m_Point(point), m_Index(index)
{}

mitk::PlanarFigureOperation::~PlanarFigureOperation()
{
}

mitk::Point2D mitk::PlanarFigureOperation::GetPoint()
{
  return m_Point;
}

int mitk::PlanarFigureOperation::GetIndex()
{
  return m_Index;
}