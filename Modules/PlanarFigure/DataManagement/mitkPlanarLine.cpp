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


#include "mitkPlanarLine.h"
#include "mitkGeometry2D.h"


mitk::PlanarLine::PlanarLine()
: FEATURE_ID_LENGTH( this->AddFeature( "Length", "mm" ) )
{
  // Line has two control points
  this->ResetNumberOfControlPoints( 2 );

  this->SetNumberOfPolyLines( 1 );
}


mitk::PlanarLine::~PlanarLine()
{
}


void mitk::PlanarLine::GeneratePolyLine()
{
  this->ClearPolyLines();
  // TODO: start line at specified start point...
  // Generate poly-line
  this->AppendPointToPolyLine( 0 , mitk::PlanarFigure::PolyLineElement( this->GetControlPoint(0), 0) );
  this->AppendPointToPolyLine( 0 , mitk::PlanarFigure::PolyLineElement( this->GetControlPoint(1), 0) );
}

void mitk::PlanarLine::GenerateHelperPolyLine(double /*mmPerDisplayUnit*/, unsigned int /*displayHeight*/)
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
