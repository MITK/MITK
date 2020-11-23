/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPointSetReader.h"
#include <fstream>
#include <iostream>
#include <mitkLocaleSwitch.h>
#include <tinyxml2.h>

mitk::PointSetReader::PointSetReader()
{
  m_Success = false;
}

mitk::PointSetReader::~PointSetReader()
{
}

void mitk::PointSetReader::GenerateData()
{
  // Switch the current locale to "C"
  LocaleSwitch localeSwitch("C");

  m_Success = false;
  if (m_FileName == "")
  {
    itkWarningMacro(<< "Sorry, filename has not been set!");
    return;
  }
  if (!this->CanReadFile(m_FileName.c_str()))
  {
    itkWarningMacro(<< "Sorry, can't read file " << m_FileName << "!");
    return;
  }

  try
  {
    tinyxml2::XMLDocument doc;
    if (tinyxml2::XML_SUCCESS == doc.LoadFile(m_FileName.c_str()))
    {
      tinyxml2::XMLHandle docHandle(&doc);
      unsigned int pointSetCounter(0);
      for (auto *currentPointSetElement =
             docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
           currentPointSetElement != nullptr;
           currentPointSetElement = currentPointSetElement->NextSiblingElement())
      {
        mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
        if (currentPointSetElement->FirstChildElement("time_series") != nullptr)
        {
          for (auto *currentTimeSeries = currentPointSetElement->FirstChildElement("time_series");
               currentTimeSeries != nullptr;
               currentTimeSeries = currentTimeSeries->NextSiblingElement())
          {
            unsigned int currentTimeStep(0);
            auto *currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

            currentTimeStep = atoi(currentTimeSeriesID->GetText());

            newPointSet = this->ReadPoint(newPointSet, currentTimeSeries, currentTimeStep);
          }
        }
        else
        {
          newPointSet = this->ReadPoint(newPointSet, currentPointSetElement, 0);
        }
        this->SetNthOutput(pointSetCounter, newPointSet);
        pointSetCounter++;
      }
    }
    else
    {
      MITK_WARN << "XML parser error!";
    }
  }
  catch (...)
  {
    MITK_ERROR << "Cannot read point set.";
    m_Success = false;
  }
  m_Success = true;
}

mitk::PointSet::Pointer mitk::PointSetReader::ReadPoint(mitk::PointSet::Pointer newPointSet,
                                                        const tinyxml2::XMLElement *currentTimeSeries,
                                                        unsigned int currentTimeStep)
{
  if (currentTimeSeries->FirstChildElement("point") != nullptr)
  {
    for (auto *currentPoint = currentTimeSeries->FirstChildElement("point");
         currentPoint != nullptr;
         currentPoint = currentPoint->NextSiblingElement())
    {
      unsigned int id(0);
      mitk::PointSpecificationType spec((mitk::PointSpecificationType)0);
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

void mitk::PointSetReader::GenerateOutputInformation()
{
}

int mitk::PointSetReader::CanReadFile(const char *name)
{
  std::ifstream in(name);
  bool isGood = in.good();
  in.close();
  return isGood;
}

bool mitk::PointSetReader::CanReadFile(const std::string filename,
                                       const std::string filePrefix,
                                       const std::string filePattern)
{
  // First check the extension
  if (filename == "")
  {
    // MITK_INFO<<"No filename specified."<<std::endl;
    return false;
  }

  // check if image is serie
  if (filePattern != "" && filePrefix != "")
    return false;

  bool extensionFound = false;
  std::string::size_type MPSPos = filename.rfind(".mps");
  if ((MPSPos != std::string::npos) && (MPSPos == filename.length() - 4))
  {
    extensionFound = true;
  }

  MPSPos = filename.rfind(".MPS");
  if ((MPSPos != std::string::npos) && (MPSPos == filename.length() - 4))
  {
    extensionFound = true;
  }

  if (!extensionFound)
  {
    // MITK_INFO<<"The filename extension is not recognized."<<std::endl;
    return false;
  }

  return true;
}

void mitk::PointSetReader::ResizeOutputs(const unsigned int &num)
{
  unsigned int prevNum = this->GetNumberOfOutputs();
  this->SetNumberOfIndexedOutputs(num);
  for (unsigned int i = prevNum; i < num; ++i)
  {
    this->SetNthOutput(i, this->MakeOutput(i).GetPointer());
  }
}

bool mitk::PointSetReader::GetSuccess() const
{
  return m_Success;
}
