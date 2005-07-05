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
  m_Geometry3D->Initialize();
}

mitk::BoundingObject::~BoundingObject() 
{
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
  m_Geometry3D->SetBounds(bounds);
}

mitk::ScalarType mitk::BoundingObject::GetVolume()
{
  return 0.0;
}

bool mitk::BoundingObject::WriteXML( XMLWriter& xmlWriter )
{
	xmlWriter.BeginNode("data");
	xmlWriter.WriteProperty( "className", typeid( *this ).name() );

  std::string fileName = xmlWriter.getNewFileName();
  fileName += ".stl";
  xmlWriter.WriteProperty( "FILENAME", fileName.c_str() );

  // Positiv
  xmlWriter.WriteProperty("Positiv", m_Positive );

  // Geometrie
	mitk::Geometry3D* geomety = GetGeometry();

	if ( geomety )
		geomety->WriteXML( xmlWriter );

	xmlWriter.EndNode(); // data
	return true;
}


bool mitk::BoundingObject::ReadXML( XMLReader& xmlReader )
{

  return false;
}
