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

#include "mitkPointSetWriterService.h"
#include "mitkGeometry3DToXML.h"
#include "mitkIOMimeTypes.h"
#include "mitkLocaleSwitch.h"

#include "mitkGeometry3D.h"

#include <tinyxml.h>

#include <fstream>
#include <iostream>
#include <locale>

//
// Initialization of the xml tags.
//
const std::string mitk::PointSetWriterService::XML_POINT_SET_FILE = "point_set_file";
const std::string mitk::PointSetWriterService::XML_FILE_VERSION = "file_version";
const std::string mitk::PointSetWriterService::XML_POINT_SET = "point_set";
const std::string mitk::PointSetWriterService::XML_TIME_SERIES = "time_series";
const std::string mitk::PointSetWriterService::XML_TIME_SERIES_ID = "time_series_id";
const std::string mitk::PointSetWriterService::XML_POINT = "point";
const std::string mitk::PointSetWriterService::XML_ID = "id";
const std::string mitk::PointSetWriterService::XML_SPEC = "specification";
const std::string mitk::PointSetWriterService::XML_X = "x";
const std::string mitk::PointSetWriterService::XML_Y = "y";
const std::string mitk::PointSetWriterService::XML_Z = "z";
const std::string mitk::PointSetWriterService::VERSION_STRING = "0.1";

mitk::PointSetWriterService::PointSetWriterService()
  : AbstractFileWriter(
      PointSet::GetStaticNameOfClass(), CustomMimeType(IOMimeTypes::POINTSET_MIMETYPE()), "MITK Point Set Writer")
{
  RegisterService();
}

mitk::PointSetWriterService::PointSetWriterService(const mitk::PointSetWriterService &other) : AbstractFileWriter(other)
{
}

mitk::PointSetWriterService::~PointSetWriterService()
{
}

void mitk::PointSetWriterService::Write()
{
  mitk::LocaleSwitch localeC("C");

  TiXmlDocument doc;

  auto *decl = new TiXmlDeclaration(
    "1.0", "UTF-8", ""); // TODO what to write here? encoding? standalone would mean that we provide a DTD somewhere...
  doc.LinkEndChild(decl);

  auto *rootNode = new TiXmlElement(XML_POINT_SET_FILE);
  doc.LinkEndChild(rootNode);

  auto *versionNode = new TiXmlElement(XML_FILE_VERSION);
  auto *versionText = new TiXmlText(VERSION_STRING);
  versionNode->LinkEndChild(versionText);
  rootNode->LinkEndChild(versionNode);

  TiXmlElement *pointSetNode = ToXML(static_cast<const PointSet *>(this->GetInput()));
  if (!pointSetNode)
  {
    mitkThrow() << "Serialization error during PointSet writing.";
  }
  rootNode->LinkEndChild(pointSetNode);

  // out << doc; // streaming of TinyXML write no new-lines,
  // rendering XML files unreadable (for humans)

  LocalFile f(this);
  if (!doc.SaveFile(f.GetFileName()))
  {
    mitkThrow() << "Some error during point set writing.";
  }
}

mitk::PointSetWriterService *mitk::PointSetWriterService::Clone() const
{
  return new PointSetWriterService(*this);
}

TiXmlElement *mitk::PointSetWriterService::ToXML(const mitk::PointSet *pointSet)
{
  // the following is rather bloated and could be expressed in more compact XML
  // (e.g. using attributes instead of tags for x/y/z). The current format is
  // kept to be compatible with the previous writer.
  auto *pointSetElement = new TiXmlElement(XML_POINT_SET);
  unsigned int timecount = pointSet->GetTimeSteps();

  for (unsigned int i = 0; i < timecount; i++)
  {
    auto *timeSeriesElement = new TiXmlElement(XML_TIME_SERIES);
    pointSetElement->LinkEndChild(timeSeriesElement);

    auto *timeSeriesIDElement = new TiXmlElement(XML_TIME_SERIES_ID);
    timeSeriesElement->LinkEndChild(timeSeriesIDElement);
    TiXmlText *timeSeriesIDText = new TiXmlText(ConvertToString(i));
    timeSeriesIDElement->LinkEndChild(timeSeriesIDText);

    PointSet::PointsContainer *pointsContainer = pointSet->GetPointSet(i)->GetPoints();
    PointSet::PointsContainer::Iterator it;

    auto *geometry = dynamic_cast<Geometry3D *>(pointSet->GetGeometry(i));
    if (geometry == nullptr)
    {
      MITK_WARN << "Writing a PointSet with something other that a Geometry3D. This is not foreseen and not handled.";
      // we'll continue anyway, this imitates previous behavior
    }
    else
    {
      TiXmlElement *geometryElement = Geometry3DToXML::ToXML(geometry);
      timeSeriesElement->LinkEndChild(geometryElement);
    }

    for (it = pointsContainer->Begin(); it != pointsContainer->End(); ++it)
    {
      auto *pointElement = new TiXmlElement(XML_POINT);
      timeSeriesElement->LinkEndChild(pointElement);

      auto *pointIDElement = new TiXmlElement(XML_ID);
      TiXmlText *pointIDText = new TiXmlText(ConvertToString(it->Index()));
      pointIDElement->LinkEndChild(pointIDText);
      pointElement->LinkEndChild(pointIDElement);

      mitk::PointSet::PointType point = it->Value();

      auto *pointSpecElement = new TiXmlElement(XML_SPEC);
      TiXmlText *pointSpecText = new TiXmlText(ConvertToString(pointSet->GetSpecificationTypeInfo(it->Index(), i)));
      pointSpecElement->LinkEndChild(pointSpecText);
      pointElement->LinkEndChild(pointSpecElement);

      auto *pointXElement = new TiXmlElement(XML_X);
      TiXmlText *pointXText = new TiXmlText(ConvertToString(point[0]));
      pointXElement->LinkEndChild(pointXText);
      pointElement->LinkEndChild(pointXElement);

      auto *pointYElement = new TiXmlElement(XML_Y);
      TiXmlText *pointYText = new TiXmlText(ConvertToString(point[1]));
      pointYElement->LinkEndChild(pointYText);
      pointElement->LinkEndChild(pointYElement);

      auto *pointZElement = new TiXmlElement(XML_Z);
      TiXmlText *pointZText = new TiXmlText(ConvertToString(point[2]));
      pointZElement->LinkEndChild(pointZText);
      pointElement->LinkEndChild(pointZElement);
    }
  }

  return pointSetElement;
}

template <typename T>
std::string mitk::PointSetWriterService::ConvertToString(T value)
{
  std::ostringstream o;
  std::locale I("C");
  o.imbue(I);

  if (o << std::setprecision(12) << value)
  {
    return o.str();
  }
  else
  {
    return "conversion error";
  }
}