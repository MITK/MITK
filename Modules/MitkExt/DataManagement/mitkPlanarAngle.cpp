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


#include "mitkPlanarAngle.h"
#include "mitkGeometry2D.h"


mitk::PlanarAngle::PlanarAngle()
{
  // Start with two control points
  m_ControlPoints->Reserve( 2 );
}


mitk::PlanarAngle::~PlanarAngle()
{
}


//void mitk::PlanarAngle::Initialize()
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


void mitk::PlanarAngle::GeneratePolyLine()
{
  // Generate poly-line for angle
  m_PolyLine->Reserve( m_ControlPoints->Size() );

  for ( unsigned int i = 0; i < m_ControlPoints->Size(); ++i )
  {
    m_PolyLine->ElementAt( i ) = m_ControlPoints->ElementAt( i );
  }
}


void mitk::PlanarAngle::PrintSelf( std::ostream& os, itk::Indent indent) const
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
