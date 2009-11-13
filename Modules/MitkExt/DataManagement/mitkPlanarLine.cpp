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
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) )
{
  // Line has two control points
  m_ControlPoints->Reserve( 2 );
  m_PolyLines->InsertElement( 0, VertexContainerType::New());
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
  m_PolyLines->ElementAt( 0 )->Reserve( 2 );
  m_PolyLines->ElementAt( 0 )->ElementAt( 0 ) = m_ControlPoints->ElementAt( 0 );
  m_PolyLines->ElementAt( 0 )->ElementAt( 1 ) = m_ControlPoints->ElementAt( 1 );
}

void mitk::PlanarLine::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
  // A line does not require a helper object
}

  
void mitk::PlanarLine::EvaluateFeaturesInternal()
{
  // Calculate line length
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );
  double length = p0.EuclideanDistanceTo( p1 );

  this->SetQuantity( FEATURE_ID_LENGTH, length );
}


void mitk::PlanarLine::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}
