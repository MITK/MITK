/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-11-06 14:14:29 +0100 (Fri, 06 Nov 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarRectangle.h"
#include "mitkGeometry2D.h"


mitk::PlanarRectangle::PlanarRectangle()
: FEATURE_ID_CIRCUMFERENCE( this->AddFeature( "Circumference", "mm" ) ),
  FEATURE_ID_AREA( this->AddFeature( "Area", "mm^2" ) ),
  m_Closed( true )
{
  // Polygon has at least two control points
  m_ControlPoints->Reserve( 4 );
  m_PolyLines->InsertElement( 0, VertexContainerType::New());
}


mitk::PlanarRectangle::~PlanarRectangle()
{
}


//void mitk::PlanarRectangle::Initialize()
//{
//  // Default initialization of circle control points
//
//  mitk::Geometry2D *geometry2D = 
//    dynamic_cast< mitk::Geometry2D * >( this->GetGeometry( 0 ) );
//
//  if ( geometry2D == NULL )
//  {
//    MITK_ERROR << "Missing Geometry2D for PlanarCircle";
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

bool mitk::PlanarRectangle::SetControlPoint( unsigned int index, const Point2D &point, bool createIfDoesNotExist )
{
  // heres the deal with the rectangle:
  // when a point is moved all corresponding corner points are moved with him
  // e.g. if the lower right point (index=3) is moved the upper right point (index=1)
  // is moved in the same x direction
  // and the lower left point (index=2) is moved in the same y direction
  // the upper left point (index=0) is left untouched
  bool set = false;

  if (createIfDoesNotExist)
  {
    m_ControlPoints->CreateIndex(index);
    m_ControlPoints->CreateElementAt(index) = point;
    set = true;
  }
  else if ( index < m_ControlPoints->Size() )
  {
    m_ControlPoints->ElementAt( index ) = point;
    set = true;
  }

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

    m_ControlPoints->ElementAt( verticalCorrespondingPointIndex ).SetElement(0, point[0]);
    m_ControlPoints->ElementAt( horizontalCorrespondingPointIndex ).SetElement(1, point[1]);
  }

  return set;
}

void mitk::PlanarRectangle::PlaceFigure( const mitk::Point2D &point )
{
  VertexContainerType::Iterator it;
  for ( it = m_ControlPoints->Begin(); it != m_ControlPoints->End(); ++it )
  {
    it->Value() = point;
  }

  m_FigurePlaced = true;
  m_SelectedControlPoint = 3;
}

void mitk::PlanarRectangle::GeneratePolyLine()
{
  // TODO: start polygon at specified initalize point...
  m_PolyLines->ElementAt( 0 )->Reserve( m_ControlPoints->Size() );
  for ( unsigned int i = 0; i < m_ControlPoints->Size(); ++i )
  {
    m_PolyLines->ElementAt( 0 )->ElementAt( i ) = m_ControlPoints->ElementAt( i );  
  }
}

void mitk::PlanarRectangle::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
{
  // A polygon does not require helper objects
}

void mitk::PlanarRectangle::EvaluateFeaturesInternal()
{
  const unsigned long &numberOfControlPoints = m_ControlPoints->Size();

  // Calculate circumference
  double circumference = 0.0;
  unsigned int i;
  for ( i = 0; i < numberOfControlPoints - 1; ++i )
  {
    circumference += this->GetWorldControlPoint( i ).EuclideanDistanceTo( 
      this->GetWorldControlPoint( i + 1 ) );
  }

  if ( this->IsClosed() )
  {
    circumference += this->GetWorldControlPoint( i ).EuclideanDistanceTo(
      this->GetWorldControlPoint( 0 ) );
  }

  this->SetQuantity( FEATURE_ID_CIRCUMFERENCE, circumference );


  // Calculate polygon area (if closed)
  double area = 0.0;
  if ( this->IsClosed() && (this->GetGeometry2D() != NULL) )
  {
    for ( i = 0; i < numberOfControlPoints; ++i )
    {
      Point2D p0 = this->GetControlPoint( i );
      Point2D p1 = this->GetControlPoint( (i + 1) % numberOfControlPoints );

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

  mitk::PlanarFigure::VertexContainerType::ConstIterator it;

  unsigned int i;
  for ( it = m_ControlPoints->Begin(), i = 0;
    it != m_ControlPoints->End();
    ++it, ++i )
  {
    os << indent << indent << i << ": " << it.Value() << std::endl;
  }
}
