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

// MITK
#include "mitkGeometryDataReaderService.h"
#include "mitkIOMimeTypes.h"
#include "mitkGeometry3DToXML.h"

// STL
#include <mitkLocaleSwitch.h>

#include <tinyxml.h>

mitk::GeometryDataReaderService::GeometryDataReaderService()
  : AbstractFileReader( IOMimeTypes::GEOMETRY_DATA_MIMETYPE(), "MITK Geometry Data Reader")
{
  RegisterService();
}

mitk::GeometryDataReaderService::~GeometryDataReaderService()
{
}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::GeometryDataReaderService::Read()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  std::vector< itk::SmartPointer<BaseData> > result;

  InputStream stream(this);

  TiXmlDocument doc;
  stream >> doc;
  if (!doc.Error())
  {
    TiXmlHandle docHandle( &doc );

    for( TiXmlElement* currentGeometryElement = docHandle.FirstChild("GeometryData").FirstChild("Geometry3D").ToElement();
         currentGeometryElement != NULL;
         currentGeometryElement = currentGeometryElement->NextSiblingElement())
    {
      Geometry3D::Pointer geometry = Geometry3DToXML::FromXML(currentGeometryElement);
      if (geometry.IsNotNull())
      {
        GeometryData::Pointer newGeometryData = GeometryData::New();
        newGeometryData->SetGeometry( geometry );

        result.push_back( newGeometryData.GetPointer() );
      }
      else
      {
          MITK_ERROR << "Invalid <Geometry3D> tag encountered. Skipping.";
      }
    }
  }
  else
  {
    mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
  }

  return result;
}

mitk::GeometryDataReaderService::GeometryDataReaderService(const mitk::GeometryDataReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::GeometryDataReaderService* mitk::GeometryDataReaderService::Clone() const
{
  return new GeometryDataReaderService(*this);
}
