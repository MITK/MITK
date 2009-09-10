/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2007-12-11 14:46:19 +0100 (Di, 11 Dez 2007) $
Version:   $Revision: 13129 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPlane.h"
#include "vtkLinearTransform.h"
#include "mitkVector.h"
#include "vtkPlaneSource.h"


namespace mitk
{

Plane::Plane()
: BoundingObject()
{
  // Set up Plane Surface.
  m_Plane = vtkPlaneSource::New();
  m_Plane->SetOrigin( -32.0, -32.0, 0.0 );
  m_Plane->SetPoint1( 32.0, -32.0, 0.0 );
  m_Plane->SetPoint2( -32.0, 32.0, 0.0 );
  m_Plane->SetResolution( 128, 128 );
  m_Plane->Update();

  this->SetVtkPolyData( m_Plane->GetOutput() );
}


Plane::~Plane()
{
  m_Plane->Delete(); 
}


void Plane::SetExtent( const double x, const double y )
{
  m_Plane->SetOrigin( -x / 2.0, -y / 2.0, 0.0 );
  m_Plane->SetPoint1( x / 2.0, -y / 2.0, 0.0 );
  m_Plane->SetPoint2( -x / 2.0, y / 2.0, 0.0 );
  m_Plane->Update();
  this->Modified();
}

void Plane::GetExtent( double &x, double &y ) const
{
  x = m_Plane->GetPoint1()[0] - m_Plane->GetOrigin()[0];
  y = m_Plane->GetPoint2()[1] - m_Plane->GetOrigin()[1];
}

void Plane::SetResolution( const int xR, const int yR )
{
  m_Plane->SetResolution( xR, yR );
  m_Plane->Update();
  this->Modified();
}


void Plane::GetResolution( int &xR, int &yR ) const
{
  m_Plane->GetResolution( xR, yR );
}


bool Plane::IsInside( const Point3D &/*worldPoint*/ ) const
{
  // Plane does not have a volume
  return false;
}


ScalarType Plane::GetVolume()
{
  // Plane does not have a volume
  return 0.0;
}

}
