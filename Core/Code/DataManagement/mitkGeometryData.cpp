/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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


#include "mitkGeometryData.h"
#include "mitkBaseProcess.h"
#include "mitkXMLReader.h"

mitk::GeometryData::GeometryData()
{

}

mitk::GeometryData::~GeometryData()
{
}

void mitk::GeometryData::UpdateOutputInformation()
{
  Superclass::UpdateOutputInformation();
}

void mitk::GeometryData::SetRequestedRegionToLargestPossibleRegion()
{

}

bool mitk::GeometryData::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  if(GetGeometry()!=NULL) return true;

  return false;
}

bool mitk::GeometryData::VerifyRequestedRegion()
{
  if(GetGeometry()==NULL) return false;

  return true;
}

void mitk::GeometryData::SetRequestedRegion(itk::DataObject *)
{

}

void mitk::GeometryData::CopyInformation(const itk::DataObject *)
{
}

bool mitk::GeometryData::WriteXMLData( XMLWriter& xmlWriter )
{
  mitk::Geometry3D* geometry = GetGeometry();

  if ( geometry )
    geometry->WriteXML( xmlWriter );

  return true;
}
  
bool mitk::GeometryData::ReadXMLData( XMLReader& xmlReader )
{
  if ( xmlReader.Goto( Geometry3D::XML_NODE_NAME ) ) 
  {
    Geometry3D::Pointer geometry = dynamic_cast<mitk::Geometry3D*>( xmlReader.CreateObject().GetPointer() );
    if ( geometry.IsNotNull() ) geometry->ReadXMLData( xmlReader );
    SetGeometry(geometry);
    xmlReader.GotoParent();
  }

  return true;
}



