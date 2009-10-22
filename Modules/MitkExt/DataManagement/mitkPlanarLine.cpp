/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarLine.h"
#include "mitkGeometry2D.h"


mitk::PlanarLine::PlanarLine()
{
  // Line has two control points
  m_ControlPoints->Reserve( 2 );
}


mitk::PlanarLine::~PlanarLine()
{
}


//void mitk::PlanarLine::Initialize()
//{
//  // Default initialization of line control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    LOG_ERROR << "Missing Geometry2D for PlanarLine";
//    return;
//  }
//
//  mitk::ScalarType width = geometry2D->GetBounds()[1];
//  mitk::ScalarType height = geometry2D->GetBounds()[3];
//  
//  mitk::Point2D &startPoint = m_ControlPoints->ElementAt( 0 );
//  mitk::Point2D &endPoint = m_ControlPoints->ElementAt( 1 );
//
//  startPoint[0] = width / 2.0;
//  startPoint[1] = height / 2.0;
//
//  endPoint[0] = startPoint[0] + 20.0;
//  endPoint[1] = startPoint[1] + 20.0;
//}


void mitk::PlanarLine::GeneratePolyLine()
{
  // TODO: start line at specified start point...
  // Generate poly-line 
  m_PolyLine->Reserve( 2 );
  m_PolyLine->ElementAt( 0 ) = m_ControlPoints->ElementAt( 0 );
  m_PolyLine->ElementAt( 1 ) = m_ControlPoints->ElementAt( 1 );
}


void mitk::PlanarLine::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of control points: " << this->GetNumberOfControlPoints() << std::endl;

  os << indent << "Control points:" << std::endl;

  mitk::PlanarFigure::VertexContainerType::ConstIterator it;

  unsigned int i;
  for ( it = m_ControlPoints->Begin(), i = 0;
    it != m_ControlPoints->End();
    ++it, ++i )
  {
    os << indent << indent << i << ": " << it.Value() << std::endl;
  }
}
