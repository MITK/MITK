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
#include "mitkVector.h"

#include <vtkLinearTransform.h>
#include <vtkPlaneSource.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>


namespace mitk
{

Plane::Plane()
: BoundingObject()
{
  // Set up Plane Surface.
  m_PlaneSource = vtkPlaneSource::New();
  m_PlaneSource->SetOrigin( -32.0, -32.0, 0.0 );
  m_PlaneSource->SetPoint1( 32.0, -32.0, 0.0 );
  m_PlaneSource->SetPoint2( -32.0, 32.0, 0.0 );
  m_PlaneSource->SetResolution( 128, 128 );
  m_PlaneSource->Update();

  m_PlaneNormal = vtkDoubleArray::New();
  m_PlaneNormal->SetNumberOfComponents( 3 );
  m_PlaneNormal->SetNumberOfTuples( m_PlaneSource->GetOutput()->GetNumberOfPoints() );
  m_PlaneNormal->SetTuple3( 0, 0.0, 0.0, 1.0 );
  m_PlaneNormal->SetName( "planeNormal" );

  m_Plane = vtkPolyData::New();
  m_Plane->DeepCopy( m_PlaneSource->GetOutput() );
  m_Plane->GetPointData()->SetVectors( m_PlaneNormal );

  this->SetVtkPolyData( m_Plane );
}


Plane::~Plane()
{
  m_PlaneSource->Delete();
  m_Plane->Delete();
  m_PlaneNormal->Delete();
}


void Plane::SetExtent( const double x, const double y )
{
  m_PlaneSource->SetOrigin( -x / 2.0, -y / 2.0, 0.0 );
  m_PlaneSource->SetPoint1( x / 2.0, -y / 2.0, 0.0 );
  m_PlaneSource->SetPoint2( -x / 2.0, y / 2.0, 0.0 );
  m_PlaneSource->Update();

  m_Plane->DeepCopy( m_PlaneSource->GetOutput() );
  m_Plane->GetPointData()->SetVectors( m_PlaneNormal );

  this->Modified();
}

void Plane::GetExtent( double &x, double &y ) const
{
  x = m_PlaneSource->GetPoint1()[0] - m_PlaneSource->GetOrigin()[0];
  y = m_PlaneSource->GetPoint2()[1] - m_PlaneSource->GetOrigin()[1];
}

void Plane::SetResolution( const int xR, const int yR )
{
  m_PlaneSource->SetResolution( xR, yR );
  m_PlaneSource->Update();

  m_Plane->DeepCopy( m_PlaneSource->GetOutput() );
  m_Plane->GetPointData()->SetVectors( m_PlaneNormal );

  this->Modified();
}


void Plane::GetResolution( int &xR, int &yR ) const
{
  m_PlaneSource->GetResolution( xR, yR );
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
