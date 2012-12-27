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

#include "mitkProperties.h"

#include "mitkPlanarRectangle.h"
#include "mitkGeometry2D.h"


mitk::PlanarRectangle::PlanarRectangle()
: FEATURE_ID_CIRCUMFERENCE( this->AddFeature( "Circumference", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm2" ) )
{
  // Rectangle has four control points
  this->ResetNumberOfControlPoints( 4 );
  this->SetProperty( "closed", mitk::BoolProperty::New(true) );
  this->SetNumberOfPolyLines( 1 );
}


mitk::PlanarRectangle::~PlanarRectangle()
{
}


bool mitk::PlanarRectangle::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
  // heres the deal with the rectangle:
  // when a point is moved all corresponding corner points are moved with him
  // e.g. if the lower right point (index=3) is moved the upper right point (index=1)
  // is moved in the same x direction
  // and the lower left point (index=2) is moved in the same y direction
  // the upper left point (index=0) is left untouched
  bool set = PlanarFigure::SetControlPoint( index, point, createIfDoesNotExist );

  if(set)
  {
    // can be made better ...
    unsigned int horizontalCorrespondingPointIndex = 1;
    unsigned int verticalCorrespondingPointIndex = 3;
    if(index == 1)
    {
      horizontalCorrespondingPointIndex = 0;
      verticalCorrespondingPointIndex = 2;
    }
    else if(index == 2)
    {
      horizontalCorrespondingPointIndex = 3;
      verticalCorrespondingPointIndex = 1;
    }
    else if(index == 3)
    {
      horizontalCorrespondingPointIndex = 2;
      verticalCorrespondingPointIndex = 0;
    }

    Point2D verticalCorrespondingPoint = GetControlPoint( verticalCorrespondingPointIndex );
    verticalCorrespondingPoint[0] = point[0];
    PlanarFigure::SetControlPoint( verticalCorrespondingPointIndex, verticalCorrespondingPoint );

    Point2D horizontalCorrespondingPoint = GetControlPoint( horizontalCorrespondingPointIndex );
    horizontalCorrespondingPoint[1] = point[1];
    PlanarFigure::SetControlPoint( horizontalCorrespondingPointIndex, horizontalCorrespondingPoint );
  }

  return set;
}

void mitk::PlanarRectangle::PlaceFigure( const mitk::Point2D &point )
{
  PlanarFigure::PlaceFigure( point );
  m_SelectedControlPoint = 3;
}

void mitk::PlanarRectangle::GeneratePolyLine()
{
  // TODO: start polygon at specified initalize point...

  ClearPolyLines();

  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    AppendPointToPolyLine( 0, PolyLineElement( GetControlPoint(i), i ) );
  }
}

void mitk::PlanarRectangle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A polygon does not require helper objects
}

void mitk::PlanarRectangle::EvaluateFeaturesInternal()
{
  // Calculate circumference
  double circumference = 0.0;
  unsigned int i;
  for ( i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    circumference += this->GetWorldControlPoint( i ).EuclideanDistanceTo(
      this->GetWorldControlPoint( (i + 1) % this->GetNumberOfControlPoints() ) );
  }

  this->SetQuantity( FEATURE_ID_CIRCUMFERENCE, circumference );


  // Calculate rectangle area (well, done a bit clumsy...)
  double area = 0.0;
  if ( this->GetGeometry2D() != NULL )
  {
    for ( i = 0; i < this->GetNumberOfControlPoints(); ++i )
    {
      Point2D p0 = this->GetControlPoint( i );
      Point2D p1 = this->GetControlPoint( (i + 1) % this->GetNumberOfControlPoints() );

      area += p0[0] * p1[1] - p1[0] * p0[1];
    }
    area /= 2.0;
  }

  this->SetQuantity( FEATURE_ID_AREA, abs(area) );

}


void mitk::PlanarRectangle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Number of control points: " << this->GetNumberOfControlPoints() << std::endl;

  os << indent << "Control points:" << std::endl;

  for ( unsigned int i = 0; i < this->GetNumberOfControlPoints(); ++i )
  {
    os << indent << indent << i << ": " <<GetControlPoint( i ) << std::endl;
  }
}
