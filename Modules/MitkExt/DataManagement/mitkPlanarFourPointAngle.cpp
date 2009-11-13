/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-10-29 12:11:25 +0100 (Do, 29 Okt 2009) $
Version:   $Revision: 18029 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlanarFourPointAngle.h"
#include "mitkGeometry2D.h"


mitk::PlanarFourPointAngle::PlanarFourPointAngle()
: FEATURE_ID_ANGLE( this->AddFeature( "Angle", "°" ) )
{
  // Four point angle has two control points
  m_ControlPoints->Reserve( 2 );
  m_PolyLines->InsertElement( 0, VertexContainerType::New());
  m_PolyLines->InsertElement( 1, VertexContainerType::New());
}


mitk::PlanarFourPointAngle::~PlanarFourPointAngle()
{
}


void mitk::PlanarFourPointAngle::GeneratePolyLine()
{
  // TODO: start line at specified start point...
  // Generate poly-line 
  m_PolyLines->ElementAt( 0 )->Reserve( 2 );
  if (m_ControlPoints->Size() > 2)
  {
    m_PolyLines->ElementAt( 1 )->Reserve( m_ControlPoints->Size() - 2 );
  }

  for ( unsigned int i = 0; i < m_ControlPoints->Size(); ++i )
  {
    if (i < 2)
    {
      m_PolyLines->ElementAt( 0 )->ElementAt( i ) = m_ControlPoints->ElementAt( i );
    }
    if (i > 1)
    {
      m_PolyLines->ElementAt( 1 )->ElementAt( i-2 ) = m_ControlPoints->ElementAt( i );
    }
  }
}

void mitk::PlanarFourPointAngle::GenerateHelperPolyLine(double mmPerDisplayUnit, unsigned int displayHeight)
{
  // Generate helper-poly-line for an four point angle
  // Need to discuss a sensible implementation
}
  
void mitk::PlanarFourPointAngle::EvaluateFeaturesInternal()
{
   // Calculate angle between lines
  const Point2D &p0 = this->GetControlPoint( 0 );
  const Point2D &p1 = this->GetControlPoint( 1 );
  const Point2D &p2 = this->GetControlPoint( 2 );
  const Point2D &p3 = this->GetControlPoint( 3 );

  Vector2D v0 = p1 - p0;
  Vector2D v1 = p3 - p2;

  v0.Normalize();
  v1.Normalize();
  double angle = acos( v0 * v1 );

  this->SetQuantity( FEATURE_ID_ANGLE, angle );
}


void mitk::PlanarFourPointAngle::PrintSelf( std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf( os, indent );

}
