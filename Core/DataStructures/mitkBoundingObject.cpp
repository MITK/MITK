/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkBoundingObject.h"
#include "mitkBaseProcess.h"
#include "mitkXMLWriter.h"

mitk::BoundingObject::BoundingObject()
  : Surface(), m_Positive(true)
{
  mitk::Geometry3D::Pointer geometry3D = mitk::Geometry3D::New();
  (static_cast<mitk::TimeSlicedGeometry*>(m_Geometry3D.GetPointer()))->InitializeEvenlyTimed( geometry3D, 1);
  GetGeometry()->Initialize();
}

mitk::BoundingObject::~BoundingObject() 
{
} 

mitk::Geometry3D* mitk::BoundingObject::GetGeometry() const
{
  TimeSlicedGeometry* timeSlicedGeometry = dynamic_cast<mitk::TimeSlicedGeometry*>( m_Geometry3D.GetPointer() );

  if ( timeSlicedGeometry )
    return timeSlicedGeometry->GetGeometry3D( 0 );

  return m_Geometry3D;
}

void mitk::BoundingObject::UpdateOutputInformation()
{  
  if ( this->GetSource() )
  {
    this->GetSource()->UpdateOutputInformation();
  }
  ScalarType bounds[6]={0,1,0,1,0,1};  //{xmin,x_max, ymin,y_max,zmin,z_max}
  /* bounding box around the unscaled bounding object */ 
  bounds[0] = - 1;
  bounds[1] = + 1;
  bounds[2] = - 1;
  bounds[3] = + 1;
  bounds[4] = - 1;
  bounds[5] = + 1; 
  GetGeometry()->SetBounds(bounds);
}

mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}

bool mitk::BoundingObject::WriteXMLData( XMLWriter& xmlWriter )
{
  Surface::WriteXMLData( xmlWriter );
  xmlWriter.WriteProperty("Positiv", m_Positive );	
	return true;
}


bool mitk::BoundingObject::ReadXMLData( XMLReader& xmlReader )
{
  Surface::ReadXMLData( xmlReader );
  return true;
}
