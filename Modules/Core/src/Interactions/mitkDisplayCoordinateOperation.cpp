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


#include "mitkDisplayCoordinateOperation.h"


mitk::DisplayCoordinateOperation::DisplayCoordinateOperation(mitk::OperationType operationType,
        mitk::BaseRenderer* renderer,
        const mitk::Point2D& startDisplayCoordinate,
        const mitk::Point2D& lastDisplayCoordinate,
        const mitk::Point2D& currentDisplayCoordinate
)
  : mitk::Operation(operationType),
    m_Renderer(renderer),
    m_StartDisplayCoordinate(startDisplayCoordinate),
    m_LastDisplayCoordinate(lastDisplayCoordinate),
    m_CurrentDisplayCoordinate(currentDisplayCoordinate)
{
}

mitk::DisplayCoordinateOperation::DisplayCoordinateOperation(mitk::OperationType operationType,
        mitk::BaseRenderer* renderer,
        const mitk::Point2D& startDisplayCoordinate,
        const mitk::Point2D& lastDisplayCoordinate,
        const mitk::Point2D& currentDisplayCoordinate,
        const mitk::Point2D& startCoordinateInMM
)
  : mitk::Operation(operationType),
    m_Renderer(renderer),
    m_StartDisplayCoordinate(startDisplayCoordinate),
    m_LastDisplayCoordinate(lastDisplayCoordinate),
    m_CurrentDisplayCoordinate(currentDisplayCoordinate),
    m_StartCoordinateInMM(startCoordinateInMM)
{
}


mitk::DisplayCoordinateOperation::~DisplayCoordinateOperation()
{
}

mitk::BaseRenderer* mitk::DisplayCoordinateOperation::GetRenderer()
{
    return m_Renderer;
}

mitk::Vector2D mitk::DisplayCoordinateOperation::GetLastToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_LastDisplayCoordinate;
}

mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToCurrentDisplayVector()
{
    return m_CurrentDisplayCoordinate-m_StartDisplayCoordinate;
}

mitk::Vector2D mitk::DisplayCoordinateOperation::GetStartToLastDisplayVector()
{
    return m_LastDisplayCoordinate-m_StartDisplayCoordinate;
}
