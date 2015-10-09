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
#include "mitkProportionalTimeGeometryToXML.h"

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

    for( TiXmlElement* geomDataElement = docHandle.FirstChildElement("GeometryData").ToElement();
         geomDataElement != nullptr;
         geomDataElement = geomDataElement->NextSiblingElement())
    {
      for( TiXmlElement* currentElement = geomDataElement->FirstChildElement();
           currentElement != nullptr;
           currentElement = currentElement->NextSiblingElement())
      {
        // different geometries could have been serialized from a GeometryData
        // object:
        std::string tagName = currentElement->Value();
        if (tagName == "Geometry3D")
        {
          Geometry3D::Pointer restoredGeometry = Geometry3DToXML::FromXML(currentElement);
          if (restoredGeometry.IsNotNull())
          {
            GeometryData::Pointer newGeometryData = GeometryData::New();
            newGeometryData->SetGeometry( restoredGeometry );
            result.push_back( newGeometryData.GetPointer() );
          }
          else
          {
            MITK_ERROR << "Invalid <Geometry3D> tag encountered. Skipping.";
          }
        }
        else if (tagName == "ProportionalTimeGeometry")
        {
          ProportionalTimeGeometry::Pointer restoredTimeGeometry = ProportionalTimeGeometryToXML::FromXML(currentElement);
          if (restoredTimeGeometry.IsNotNull())
          {
            GeometryData::Pointer newGeometryData = GeometryData::New();
            newGeometryData->SetTimeGeometry( restoredTimeGeometry );
            result.push_back( newGeometryData.GetPointer() );
          }
          else
          {
            MITK_ERROR << "Invalid <ProportionalTimeGeometry> tag encountered. Skipping.";
          }
        }
      } // for child of <GeometryData>
    } // for <GeometryData>
  }
  else
  {
    mitkThrow() << "Parsing error at line " << doc.ErrorRow() << ", col " << doc.ErrorCol() << ": " << doc.ErrorDesc();
  }

  if (result.empty())
  {
    mitkThrow() << "Did not read a single GeometryData object from input.";
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
