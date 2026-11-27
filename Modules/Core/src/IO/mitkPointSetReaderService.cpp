/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkPointSetReaderService.h"
#include "mitkGeometry3DToXML.h"
#include <mitkIOMimeTypes.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkLocaleSwitch.h>
#include <mitkLexicalCast.h>

// STL
#include <charconv>
#include <fstream>
#include <iostream>
#include <string_view>
#include <type_traits>

#include <tinyxml2.h>

namespace
{
  template <typename T>
  T ParseNumber(const tinyxml2::XMLElement *parent, const char *name)
  {
    const auto *child = parent->FirstChildElement(name);
    const std::string_view text = child != nullptr && child->GetText() != nullptr ? child->GetText() : "";

    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string_view::npos)
      mitkThrow() << "Missing or empty <" << name << "> element.";

    const auto number = text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);

    // Floating-point std::from_chars is missing from older libc++ (macOS), while
    // LexicalCast wraps negative input around for unsigned types.
    if constexpr (std::is_floating_point_v<T>)
    {
      try
      {
        return mitk::LexicalCast<T>(std::string(number));
      }
      catch (const mitk::BadLexicalCast &)
      {
        mitkThrow() << "Invalid <" << name << "> \"" << number << "\".";
      }
    }
    else
    {
      const char *end = number.data() + number.size();

      T value{};
      const auto [last, error] = std::from_chars(number.data(), end, value);

      if (error != std::errc() || last != end)
        mitkThrow() << "Invalid <" << name << "> \"" << number << "\".";

      return value;
    }
  }

  mitk::TimeStepType ReadTimeStep(const tinyxml2::XMLElement *timeSeries)
  {
    const auto timeStep = ParseNumber<mitk::TimeStepType>(timeSeries, "time_series_id");

    if (timeStep == mitk::TIMESTEP_INVALID)
      mitkThrow() << "Invalid <time_series_id> \"" << timeStep << "\".";

    return timeStep;
  }
}

mitk::PointSetReaderService::PointSetReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::POINTSET_MIMETYPE()), "MITK Point Set Reader")
{
  RegisterService();
}

mitk::PointSetReaderService::~PointSetReaderService()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::PointSetReaderService::DoRead()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  std::vector<itk::SmartPointer<mitk::BaseData>> result;

  InputStream stream(this);

  std::string s(std::istreambuf_iterator<char>{stream.rdbuf()}, std::istreambuf_iterator<char>());
  tinyxml2::XMLDocument doc;
  doc.Parse(s.c_str(), s.size());
  if (!doc.Error())
  {
    tinyxml2::XMLHandle docHandle(&doc);
    // unsigned int pointSetCounter(0);
    for (auto *currentPointSetElement =
           docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
         currentPointSetElement != nullptr;
         currentPointSetElement = currentPointSetElement->NextSiblingElement("point_set"))
    {
      mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();

      // time geometry assembled for addition after all points
      // else the SetPoint method would already transform the points that we provide it
      mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();

      if (currentPointSetElement->FirstChildElement("time_series") != nullptr)
      {
        for (auto *currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
             currentTimeSeries != nullptr;
             currentTimeSeries = currentTimeSeries->NextSiblingElement("time_series"))
        {
          const mitk::TimeStepType currentTimeStep = ReadTimeStep(currentTimeSeries);

          timeGeometry->Expand(currentTimeStep + 1); // expand (default to identity) in any case
          auto *geometryElem = currentTimeSeries->FirstChildElement("Geometry3D");
          if (geometryElem)
          {
            Geometry3D::Pointer geometry = Geometry3DToXML::FromXML(geometryElem);
            if (geometry.IsNotNull())
            {
              timeGeometry->SetTimeStepGeometry(geometry, currentTimeStep);
            }
            else
            {
              MITK_ERROR << "Could not deserialize Geometry3D element.";
            }
          }
          else
          {
            MITK_WARN << "Fallback to legacy behavior: defining PointSet geometry as identity";
          }

          newPointSet = this->ReadPoints(newPointSet, currentTimeSeries, currentTimeStep);
        }
      }
      else
      {
        timeGeometry->Expand(1);
        newPointSet = this->ReadPoints(newPointSet, currentPointSetElement, 0);
      }

      newPointSet->SetTimeGeometry(timeGeometry);

      result.push_back(newPointSet.GetPointer());
    }
  }
  else
  {
    mitkThrow() << doc.ErrorStr();
  }

  return result;
}


mitk::PointSet::Pointer mitk::PointSetReaderService::ReadPoints(mitk::PointSet::Pointer newPointSet,
                                                                tinyxml2::XMLElement *currentTimeSeries,
                                                                mitk::TimeStepType currentTimeStep)
{
  if (currentTimeSeries->FirstChildElement("point") != nullptr)
  {
    for (auto *currentPoint = currentTimeSeries->FirstChildElement("point"); currentPoint != nullptr;
         currentPoint = currentPoint->NextSiblingElement("point"))
    {
      const auto id = ParseNumber<mitk::PointSet::PointIdentifier>(currentPoint, "id");

      auto spec = mitk::PTUNDEFINED;
      if (currentPoint->FirstChildElement("specification") != nullptr)
        spec = static_cast<mitk::PointSpecificationType>(ParseNumber<int>(currentPoint, "specification"));
      const auto x = ParseNumber<double>(currentPoint, "x");
      const auto y = ParseNumber<double>(currentPoint, "y");
      const auto z = ParseNumber<double>(currentPoint, "z");

      mitk::Point3D point;
      mitk::FillVector3D(point, x, y, z);
      newPointSet->SetPoint(id, point, spec, currentTimeStep);
    }
  }
  else
  {
    newPointSet->Expand(currentTimeStep + 1); // expand time step series with empty time step
  }
  return newPointSet;
}

mitk::PointSetReaderService::PointSetReaderService(const mitk::PointSetReaderService &other)
  : mitk::AbstractFileReader(other)
{
}

mitk::PointSetReaderService *mitk::PointSetReaderService::Clone() const
{
  return new mitk::PointSetReaderService(*this);
}
