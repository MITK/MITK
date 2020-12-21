/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkGeometryDataReaderService.h"
#include "mitkGeometry3DToXML.h"
#include "mitkIOMimeTypes.h"
#include "mitkProportionalTimeGeometryToXML.h"

// STL
#include <mitkLocaleSwitch.h>

#include <tinyxml2.h>

mitk::GeometryDataReaderService::GeometryDataReaderService()
  : AbstractFileReader(IOMimeTypes::GEOMETRY_DATA_MIMETYPE(), "MITK Geometry Data Reader")
{
  RegisterService();
}

mitk::GeometryDataReaderService::~GeometryDataReaderService()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::GeometryDataReaderService::DoRead()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  std::vector<itk::SmartPointer<BaseData>> result;

  InputStream stream(this);

  std::string s(std::istreambuf_iterator<char>{stream.rdbuf()}, std::istreambuf_iterator<char>());
  tinyxml2::XMLDocument doc;
  doc.Parse(s.c_str(), s.size());
  if (!doc.Error())
  {
    tinyxml2::XMLHandle docHandle(&doc);

    for (auto *geomDataElement = docHandle.FirstChildElement("GeometryData").ToElement();
         geomDataElement != nullptr;
         geomDataElement = geomDataElement->NextSiblingElement())
    {
      for (auto *currentElement = geomDataElement->FirstChildElement(); currentElement != nullptr;
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
            newGeometryData->SetGeometry(restoredGeometry);
            result.push_back(newGeometryData.GetPointer());
          }
          else
          {
            MITK_ERROR << "Invalid <Geometry3D> tag encountered. Skipping.";
          }
        }
        else if (tagName == "ProportionalTimeGeometry")
        {
          ProportionalTimeGeometry::Pointer restoredTimeGeometry =
            ProportionalTimeGeometryToXML::FromXML(currentElement);
          if (restoredTimeGeometry.IsNotNull())
          {
            GeometryData::Pointer newGeometryData = GeometryData::New();
            newGeometryData->SetTimeGeometry(restoredTimeGeometry);
            result.push_back(newGeometryData.GetPointer());
          }
          else
          {
            MITK_ERROR << "Invalid <ProportionalTimeGeometry> tag encountered. Skipping.";
          }
        }
      } // for child of <GeometryData>
    }   // for <GeometryData>
  }
  else
  {
    mitkThrow() << doc.ErrorStr();
  }

  if (result.empty())
  {
    mitkThrow() << "Did not read a single GeometryData object from input.";
  }

  return result;
}

mitk::GeometryDataReaderService::GeometryDataReaderService(const mitk::GeometryDataReaderService &other)
  : mitk::AbstractFileReader(other)
{
}

mitk::GeometryDataReaderService *mitk::GeometryDataReaderService::Clone() const
{
  return new GeometryDataReaderService(*this);
}
