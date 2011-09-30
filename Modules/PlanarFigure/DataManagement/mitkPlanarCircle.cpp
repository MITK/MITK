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

#include "mitkProperties.h"


mitk::PlanarCircle::PlanarCircle()
: FEATURE_ID_RADIUS( this->AddFeature( "Radius", "mm" ) ),
  FEATURE_ID_DIAMETER( this->AddFeature( "Diameter", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) ),
  m_MinRadius(0),
  m_MaxRadius(100),
  m_MinMaxRadiusContraintsActive(false)
{
  // Circle has two control points
  this->ResetNumberOfControlPoints( 2 );
  this->SetNumberOfPolyLines( 1 );
  this->SetProperty( "closed", mitk::BoolProperty::New(true) );
}


mitk::PlanarCircle::~PlanarCircle()
{
}

bool mitk::PlanarCircle::SetControlPoint( unsigned int index, const Point2D &point, bool /*createIfDoesNotExist*/ )
{
  // moving center point
  if(index == 0)
  {
    const Point2D &centerPoint = GetControlPoint( 0 );
    Point2D boundaryPoint = GetControlPoint( 1 );
    vnl_vector<float> vec = (point.GetVnlVector() - centerPoint.GetVnlVector());

    boundaryPoint[0] += vec[0];
    boundaryPoint[1] += vec[1];
    PlanarFigure::SetControlPoint( 0, point );
    PlanarFigure::SetControlPoint( 1, boundaryPoint );
  }
  else if ( index == 1 )
  {
    PlanarFigure::SetControlPoint( index, point );
    return true;
  }
  return false;
}

mitk::Point2D mitk::PlanarCircle::ApplyControlPointConstraints(unsigned int index, const Point2D &point)
{
  if ( this->GetGeometry2D() ==  NULL )
  {
    return point;
  }

  Point2D indexPoint;
  this->GetGeometry2D()->WorldToIndex( point, indexPoint );

  BoundingBox::BoundsArrayType bounds = this->GetGeometry2D()->GetBounds();
  if ( indexPoint[0] < bounds[0] ) { indexPoint[0] = bounds[0]; }
  if ( indexPoint[0] > bounds[1] ) { indexPoint[0] = bounds[1]; }
  if ( indexPoint[1] < bounds[2] ) { indexPoint[1] = bounds[2]; }
  if ( indexPoint[1] > bounds[3] ) { indexPoint[1] = bounds[3]; }

  Point2D constrainedPoint;
  this->GetGeometry2D()->IndexToWorld( indexPoint, constrainedPoint );

  if(m_MinMaxRadiusContraintsActive)
  {
    if( index != 0)
    {
      const Point2D &centerPoint = this->GetControlPoint(0);
      double euclideanDinstanceFromCenterToPoint1 = centerPoint.EuclideanDistanceTo(point);

      Vector2D vectorProjectedPoint;
      vectorProjectedPoint = point - centerPoint;
      vectorProjectedPoint.Normalize();

      if( euclideanDinstanceFromCenterToPoint1 > m_MaxRadius )
      {
        vectorProjectedPoint *= m_MaxRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
      else if( euclideanDinstanceFromCenterToPoint1 < m_MinRadius )
      {
        vectorProjectedPoint *= m_MinRadius;
        constrainedPoint = centerPoint;
        constrainedPoint += vectorProjectedPoint;
      }
    }
  }

  return constrainedPoint;
}

void mitk::PlanarCircle::GeneratePolyLine()
{
  // TODO: start circle at specified boundary point...
  
  // clear the PolyLine-Contrainer, it will be reconstructed soon enough...
  this->ClearPolyLines();

  const Point2D &centerPoint = GetControlPoint( 0 );
  const Point2D &boundaryPoint = GetControlPoint( 1 );

  double radius = centerPoint.EuclideanDistanceTo( boundaryPoint );

  // Generate poly-line with 64 segments
  for ( int t = 0; t < 64; ++t )
  {
    double alpha = (double) t * vnl_math::pi / 32.0;

    // construct the new polyline point ...
    Point2D polyLinePoint;
    polyLinePoint[0] = centerPoint[0] + radius * cos( alpha );
    polyLinePoint[1] = centerPoint[1] + radius * sin( alpha );

    // ... and append it to the PolyLine. 
    // No extending supported here, so we can set the index of the PolyLineElement to '0'
    AppendPointToPolyLine( 0, PolyLineElement( polyLinePoint, 0 ) );
  }
}

void mitk::PlanarCircle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A circle does not require a helper object
}

void mitk::PlanarCircle::EvaluateFeaturesInternal()
{
  // Calculate circle radius and area
  const Point3D &p0 = this->GetWorldControlPoint( 0 );
  const Point3D &p1 = this->GetWorldControlPoint( 1 );

  double radius = p0.EuclideanDistanceTo( p1 );
  double area = vnl_math::pi * radius * radius;

  this->SetQuantity( FEATURE_ID_RADIUS, radius );
  this->SetQuantity( FEATURE_ID_DIAMETER, 2*radius );
  this->SetQuantity( FEATURE_ID_AREA, area );
}


void mitk::PlanarCircle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );
}
