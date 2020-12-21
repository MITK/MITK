/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkContourModelReader.h"
#include <fstream>
#include <iostream>
#include <regex>
#include <mitkCustomMimeType.h>
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

namespace
{
  // Previous versions of the ContourModelSetWriter produced flawed
  // XML files with multiple XML declarations.
  std::string RemoveErroneousXMLDeclarations(const std::string& filename)
  {
    std::ifstream file(filename);
    file.seekg(0, std::ios_base::end);
    auto size = file.tellg();
    std::string string(size, '\0');
    file.seekg(0);
    file.read(&string[0], size);
    file.close();
    std::regex regex("><\\?xml.+\\?>");
    return std::regex_replace(string, regex, ">");
  }
}

mitk::ContourModelReader::ContourModelReader(const mitk::ContourModelReader &other) : mitk::AbstractFileReader(other)
{
}

mitk::ContourModelReader::ContourModelReader() : AbstractFileReader()
{
  std::string category = "Contour File";
  mitk::CustomMimeType customMimeType;
  customMimeType.SetCategory(category);
  customMimeType.AddExtension("cnt");

  this->SetDescription(category);
  this->SetMimeType(customMimeType);

  m_ServiceReg = this->RegisterService();
}

mitk::ContourModelReader::~ContourModelReader()
{
}

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::ContourModelReader::DoRead()
{
  std::vector<itk::SmartPointer<mitk::BaseData>> result;
  std::string location = GetInputLocation();

  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  try
  {
    auto string = RemoveErroneousXMLDeclarations(location);

    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_SUCCESS == doc.Parse(string.c_str()))
    {
      tinyxml2::XMLHandle docHandle(&doc);

      /*++++ handle n contourModels within data tags ++++*/
      for (auto *currentContourElement = docHandle.FirstChildElement("contourModel").ToElement();
           currentContourElement != nullptr;
           currentContourElement = currentContourElement->NextSiblingElement())
      {
        mitk::ContourModel::Pointer newContourModel = mitk::ContourModel::New();
        if (currentContourElement->FirstChildElement("data")->FirstChildElement("timestep") != nullptr)
        {
          /*++++ handle n timesteps within timestep tags ++++*/
          for (auto *currentTimeSeries =
                 currentContourElement->FirstChildElement("data")->FirstChildElement("timestep")->ToElement();
               currentTimeSeries != nullptr;
               currentTimeSeries = currentTimeSeries->NextSiblingElement())
          {
            unsigned int currentTimeStep(0);

            currentTimeStep = atoi(currentTimeSeries->Attribute("n"));

            this->ReadPoints(newContourModel, currentTimeSeries, currentTimeStep);

            int isClosed;
            currentTimeSeries->QueryIntAttribute("isClosed", &isClosed);
            if (isClosed)
            {
              newContourModel->Close(currentTimeStep);
            }
          }
          /*++++ END handle n timesteps within timestep tags ++++*/
        }
        else
        {
          // this should not happen
          MITK_WARN << "wrong file format!";
          // newContourModel = this->ReadPoint(newContourModel, currentContourElement, 0);
        }
        newContourModel->UpdateOutputInformation();
        result.push_back(dynamic_cast<mitk::BaseData *>(newContourModel.GetPointer()));
      }
      /*++++ END handle n contourModels within data tags ++++*/
    }
    else
    {
      MITK_WARN << "XML parser error!";
    }
  }
  catch (...)
  {
    MITK_ERROR << "Cannot read contourModel.";
  }

  return result;
}

mitk::ContourModelReader *mitk::ContourModelReader::Clone() const
{
  return new ContourModelReader(*this);
}

void mitk::ContourModelReader::ReadPoints(mitk::ContourModel::Pointer newContourModel,
                                          const tinyxml2::XMLElement *currentTimeSeries,
                                          unsigned int currentTimeStep)
{
  // check if the timesteps in contourModel have to be expanded
  if (currentTimeStep != newContourModel->GetTimeSteps())
  {
    newContourModel->Expand(currentTimeStep + 1);
  }

  // read all points within controlPoints tag
  if (currentTimeSeries->FirstChildElement("controlPoints")->FirstChildElement("point") != nullptr)
  {
    for (auto *currentPoint =
           currentTimeSeries->FirstChildElement("controlPoints")->FirstChildElement("point")->ToElement();
         currentPoint != nullptr;
         currentPoint = currentPoint->NextSiblingElement())
    {
      double x(0.0);
      double y(0.0);
      double z(0.0);

      x = atof(currentPoint->FirstChildElement("x")->GetText());
      y = atof(currentPoint->FirstChildElement("y")->GetText());
      z = atof(currentPoint->FirstChildElement("z")->GetText());

      int isActivePoint;
      currentPoint->QueryIntAttribute("isActive", &isActivePoint);

      mitk::Point3D point;
      mitk::FillVector3D(point, x, y, z);
      newContourModel->AddVertex(point, isActivePoint, currentTimeStep);
    }
  }
  else
  {
    // nothing to read
  }
}
