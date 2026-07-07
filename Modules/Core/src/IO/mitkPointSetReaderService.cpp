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

// STL
#include <fstream>
#include <iostream>

#include <tinyxml2.h>

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
         currentPointSetElement = currentPointSetElement->NextSiblingElement())
    {
      mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();

      // time geometry assembled for addition after all points
      // else the SetPoint method would already transform the points that we provide it
      mitk::ProportionalTimeGeometry::Pointer timeGeometry = mitk::ProportionalTimeGeometry::New();

      if (currentPointSetElement->FirstChildElement("time_series") != nullptr)
      {
        for (auto *currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
             currentTimeSeries != nullptr;
             currentTimeSeries = currentTimeSeries->NextSiblingElement())
        {
          unsigned int currentTimeStep(0);
          auto *currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

          currentTimeStep = atoi(currentTimeSeriesID->GetText());

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
                                                                unsigned int currentTimeStep)
{
  if (currentTimeSeries->FirstChildElement("point") != nullptr)
  {
    for (auto *currentPoint = currentTimeSeries->FirstChildElement("point"); currentPoint != nullptr;
         currentPoint = currentPoint->NextSiblingElement())
    {
      unsigned int id(0);
      auto spec((mitk::PointSpecificationType)0);
      double x(0.0);
      double y(0.0);
      double z(0.0);

      id = atoi(currentPoint->FirstChildElement("id")->GetText());
      if (currentPoint->FirstChildElement("specification") != nullptr)
      {
        spec = (mitk::PointSpecificationType)atoi(currentPoint->FirstChildElement("specification")->GetText());
      }
      x = atof(currentPoint->FirstChildElement("x")->GetText());
      y = atof(currentPoint->FirstChildElement("y")->GetText());
      z = atof(currentPoint->FirstChildElement("z")->GetText());

      mitk::Point3D point;
      mitk::FillVector3D(point, x, y, z);
      newPointSet->SetPoint(id, point, spec, currentTimeStep);
    }
  }
  else
  {
    if (currentTimeStep != newPointSet->GetTimeSteps() + 1)
    {
      newPointSet->Expand(currentTimeStep + 1); // expand time step series with empty time step
    }
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
