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
#include "mitkPointSetReaderService.h"
#include <mitkPointSet.h>

// STL
#include <iostream>
#include <fstream>
#include <locale>

mitk::PointSetReaderService::PointSetReaderService()
  : AbstractFileReader("application/vnd.mitk.pointset", "mps", "Great Reader of Point Sets")
{
  RegisterService();
}

mitk::PointSetReaderService::~PointSetReaderService()
{}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::PointSetReaderService::Read(std::istream& stream)
{
  std::locale::global(std::locale("C"));

  std::vector< itk::SmartPointer<mitk::BaseData> > result;


  stream.seekg(0, std::ios_base::end);
  long int length = stream.tellg();
  stream.seekg(0, std::ios_base::beg);

  if (length <= 0)
  {
    MITK_WARN << "Could not read point set, no data.";
    return result;
  }

  char* data = new char[length+1];
  stream.read(data, length);
  data[length] = 0;

  try{
    TiXmlDocument doc("pointset.xml");
    if (doc.Parse(data))
    {
      TiXmlHandle docHandle( &doc );
      //unsigned int pointSetCounter(0);
      for( TiXmlElement* currentPointSetElement = docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
        currentPointSetElement != NULL; currentPointSetElement = currentPointSetElement->NextSiblingElement())
      {
        mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
        if(currentPointSetElement->FirstChildElement("time_series") != NULL)
        {
          for( TiXmlElement* currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
            currentTimeSeries != NULL; currentTimeSeries = currentTimeSeries->NextSiblingElement())
          {
            unsigned int currentTimeStep(0);
            TiXmlElement* currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

            currentTimeStep = atoi(currentTimeSeriesID->GetText());

            newPointSet = this->ReadPoint(newPointSet, currentTimeSeries, currentTimeStep);
          }
        }
        else
        {
          newPointSet = this->ReadPoint(newPointSet, currentPointSetElement, 0);
        }
        result.push_back( newPointSet.GetPointer() );
      }
    }
    else
    {
      MITK_WARN << "XML parser error in mitkPointSetReaderService! Aborting...";
    }
  }catch(...)
  {
    MITK_ERROR  << "Error while reading point set. Aborting...";
  }

  delete data;

  return result;
}

mitk::PointSet::Pointer mitk::PointSetReaderService::ReadPoint(mitk::PointSet::Pointer newPointSet,
                                                               TiXmlElement* currentTimeSeries, unsigned int currentTimeStep)
{
  if(currentTimeSeries->FirstChildElement("point") != NULL)
  {
    for( TiXmlElement* currentPoint = currentTimeSeries->FirstChildElement("point")->ToElement();
      currentPoint != NULL; currentPoint = currentPoint->NextSiblingElement())
    {
      unsigned int id(0);
      mitk::PointSpecificationType spec((mitk::PointSpecificationType) 0);
      double x(0.0);
      double y(0.0);
      double z(0.0);

      id = atoi(currentPoint->FirstChildElement("id")->GetText());
      if(currentPoint->FirstChildElement("specification") != NULL)
      {
        spec = (mitk::PointSpecificationType) atoi(currentPoint->FirstChildElement("specification")->GetText());
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
    if(currentTimeStep != newPointSet->GetTimeSteps()+1)
    {
      newPointSet->Expand(currentTimeStep+1);     // expand time step series with empty time step
    }
  }
  return newPointSet;
}

mitk::PointSetReaderService::PointSetReaderService(const mitk::PointSetReaderService& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::IFileReader* mitk::PointSetReaderService::Clone() const
{
  return new mitk::PointSetReaderService(*this);
}
