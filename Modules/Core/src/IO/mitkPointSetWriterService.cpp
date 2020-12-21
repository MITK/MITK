/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetWriterService.h"
#include "mitkGeometry3DToXML.h"
#include "mitkIOMimeTypes.h"
#include "mitkLocaleSwitch.h"

#include "mitkGeometry3D.h"

#include <fstream>
#include <iostream>
#include <locale>

#include <tinyxml2.h>

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

  tinyxml2::XMLDocument doc;
  doc.InsertEndChild(doc.NewDeclaration());

  auto *rootNode = doc.NewElement(XML_POINT_SET_FILE.c_str());
  doc.InsertEndChild(rootNode);

  auto *versionNode = doc.NewElement(XML_FILE_VERSION.c_str());
  auto *versionText = doc.NewText(VERSION_STRING.c_str());
  versionNode->InsertEndChild(versionText);
  rootNode->InsertEndChild(versionNode);

  auto *pointSetNode = ToXML(doc, static_cast<const PointSet *>(this->GetInput()));
  if (!pointSetNode)
  {
    mitkThrow() << "Serialization error during PointSet writing.";
  }
  rootNode->InsertEndChild(pointSetNode);

  // out << doc; // streaming of TinyXML write no new-lines,
  // rendering XML files unreadable (for humans)

  LocalFile f(this);
  if (tinyxml2::XML_SUCCESS != doc.SaveFile(f.GetFileName().c_str()))
  {
    mitkThrow() << "Some error during point set writing.";
  }
}

mitk::PointSetWriterService *mitk::PointSetWriterService::Clone() const
{
  return new PointSetWriterService(*this);
}

tinyxml2::XMLElement *mitk::PointSetWriterService::ToXML(tinyxml2::XMLDocument& doc, const mitk::PointSet *pointSet)
{
  // the following is rather bloated and could be expressed in more compact XML
  // (e.g. using attributes instead of tags for x/y/z). The current format is
  // kept to be compatible with the previous writer.
  auto *pointSetElement = doc.NewElement(XML_POINT_SET.c_str());
  unsigned int timecount = pointSet->GetTimeSteps();

  for (unsigned int i = 0; i < timecount; i++)
  {
    auto *timeSeriesElement = doc.NewElement(XML_TIME_SERIES.c_str());
    pointSetElement->InsertEndChild(timeSeriesElement);

    auto *timeSeriesIDElement = doc.NewElement(XML_TIME_SERIES_ID.c_str());
    timeSeriesElement->InsertEndChild(timeSeriesIDElement);
    auto *timeSeriesIDText = doc.NewText(ConvertToString(i).c_str());
    timeSeriesIDElement->InsertEndChild(timeSeriesIDText);

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
      auto *geometryElement = Geometry3DToXML::ToXML(doc, geometry);
      timeSeriesElement->InsertEndChild(geometryElement);
    }

    for (it = pointsContainer->Begin(); it != pointsContainer->End(); ++it)
    {
      auto *pointElement = doc.NewElement(XML_POINT.c_str());
      timeSeriesElement->InsertEndChild(pointElement);

      auto *pointIDElement = doc.NewElement(XML_ID.c_str());
      auto *pointIDText = doc.NewText(ConvertToString(it->Index()).c_str());
      pointIDElement->InsertEndChild(pointIDText);
      pointElement->InsertEndChild(pointIDElement);

      mitk::PointSet::PointType point = it->Value();

      auto *pointSpecElement = doc.NewElement(XML_SPEC.c_str());
      auto *pointSpecText = doc.NewText(ConvertToString(pointSet->GetSpecificationTypeInfo(it->Index(), i)).c_str());
      pointSpecElement->InsertEndChild(pointSpecText);
      pointElement->InsertEndChild(pointSpecElement);

      auto *pointXElement = doc.NewElement(XML_X.c_str());
      auto *pointXText = doc.NewText(ConvertToString(point[0]).c_str());
      pointXElement->InsertEndChild(pointXText);
      pointElement->InsertEndChild(pointXElement);

      auto *pointYElement = doc.NewElement(XML_Y.c_str());
      auto *pointYText = doc.NewText(ConvertToString(point[1]).c_str());
      pointYElement->InsertEndChild(pointYText);
      pointElement->InsertEndChild(pointYElement);

      auto *pointZElement = doc.NewElement(XML_Z.c_str());
      auto *pointZText = doc.NewText(ConvertToString(point[2]).c_str());
      pointZElement->InsertEndChild(pointZText);
      pointElement->InsertEndChild(pointZElement);
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
