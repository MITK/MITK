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
