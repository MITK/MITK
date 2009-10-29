/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-08 11:04:08 +0200 (Mi, 08 Jul 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarCircle.h"
#include "mitkGeometry2D.h"


mitk::PlanarCircle::PlanarCircle()
: FEATURE_ID_RADIUS( this->AddFeature( "Radius", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm^2" ) )
{
  // Circle has two control points
  m_ControlPoints->Reserve( 2 );
}


mitk::PlanarCircle::~PlanarCircle()
{
}


//void mitk::PlanarCircle::Initialize()
//{
//  // Default initialization of circle control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    LOG_ERROR << "Missing Geometry2D for PlanarCircle";
//    return;
//  }
//
//  mitk::ScalarType width = geometry2D->GetBounds()[1];
//  mitk::ScalarType height = geometry2D->GetBounds()[3];
//  
//  mitk::Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
//  mitk::Point2D &boundaryPoint = m_ControlPoints->ElementAt( 1 );
//
//  centerPoint[0] = width / 2.0;
//  centerPoint[1] = height / 2.0;
//
//  boundaryPoint[0] = centerPoint[0] + 20.0;
//  boundaryPoint[1] = centerPoint[1];
//}


void mitk::PlanarCircle::GeneratePolyLine()
{
  // TODO: start circle at specified boundary point...

  mitk::Point2D &centerPoint = m_ControlPoints->ElementAt( 0 );
  mitk::Point2D &boundaryPoint = m_ControlPoints->ElementAt( 1 );

  double radius = centerPoint.EuclideanDistanceTo( boundaryPoint );

  // Generate poly-line with 256 segments
  m_PolyLine->Reserve( 256 );
  for ( int t = 0; t < 256; ++t )
  {
    double alpha = (double) t * vnl_math::pi / 128.0;

    m_PolyLine->ElementAt( t )[0] = centerPoint[0] + radius * cos( alpha );
    m_PolyLine->ElementAt( t )[1] = centerPoint[1] + radius * sin( alpha );
  }
}


void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  // Calculate circle radius and area
  Point3D &p0 = this->GetWorldControlPoint( 0 );
  Point3D &p1 = this->GetWorldControlPoint( 1 );

  double radius = p0.EuclideanDistanceTo( p1 );
  double area = vnl_math::pi * radius * radius;

  this->SetQuantity( FEATURE_ID_RADIUS, radius );
  this->SetQuantity( FEATURE_ID_AREA, area );
}


void mitk::PlanarCircle::PrintSelf( std::ostream& os, itk::Indent indent) const
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
