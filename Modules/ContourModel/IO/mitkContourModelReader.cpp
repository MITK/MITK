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

#include "mitkContourModelReader.h"
#include <mitkCustomMimeType.h>
#include <iostream>
#include <fstream>
#include <locale>

mitk::ContourModelReader::ContourModelReader(const mitk::ContourModelReader& other)
  : mitk::AbstractFileReader(other)
{
}

mitk::ContourModelReader::ContourModelReader()
  : AbstractFileReader()
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

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::ContourModelReader::Read()
{
  std::vector<itk::SmartPointer<mitk::BaseData> > result;
  std::string location = GetInputLocation();

  std::locale::global(std::locale("C"));


  try{
    TiXmlDocument doc(location.c_str());
    bool loadOkay = doc.LoadFile();
    if (loadOkay)
    {
      TiXmlHandle docHandle( &doc );

      /*++++ handle n contourModels within data tags ++++*/
      for( TiXmlElement* currentContourElement = docHandle.FirstChildElement("contourModel").ToElement();
        currentContourElement != NULL; currentContourElement = currentContourElement->NextSiblingElement())
      {
        mitk::ContourModel::Pointer newContourModel = mitk::ContourModel::New();
        if(currentContourElement->FirstChildElement("data")->FirstChildElement("timestep") != NULL)
        {

          //handle geometry information
          //TiXmlElement* currentGeometryInfo = currentContourElement->FirstChildElement("head")->FirstChildElement("geometryInformation")->ToElement();
          ///////////// NOT SUPPORTED YET ////////////////


          /*++++ handle n timesteps within timestep tags ++++*/
          for( TiXmlElement* currentTimeSeries = currentContourElement->FirstChildElement("data")->FirstChildElement("timestep")->ToElement();
            currentTimeSeries != NULL; currentTimeSeries = currentTimeSeries->NextSiblingElement())
          {
            unsigned int currentTimeStep(0);


            currentTimeStep = atoi(currentTimeSeries->Attribute("n"));

            this->ReadPoints(newContourModel, currentTimeSeries, currentTimeStep);

            int isClosed;
            currentTimeSeries->QueryIntAttribute("isClosed", &isClosed);
            if( isClosed )
            {
              newContourModel->Close(currentTimeStep);
            }
          }
          /*++++ END handle n timesteps within timestep tags ++++*/

        }
        else
        {
          //this should not happen
          MITK_WARN << "wrong file format!";
          //newContourModel = this->ReadPoint(newContourModel, currentContourElement, 0);
        }
        newContourModel->UpdateOutputInformation();
        result.push_back(dynamic_cast<mitk::BaseData*>(newContourModel.GetPointer()));
      }
      /*++++ END handle n contourModels within data tags ++++*/
    }
    else
    {
      MITK_WARN << "XML parser error!";
    }
  }catch(...)
  {
    MITK_ERROR  << "Cannot read contourModel.";
  }

  return result;
}

mitk::ContourModelReader* mitk::ContourModelReader::Clone() const
{
  return new ContourModelReader(*this);
}

void mitk::ContourModelReader::ReadPoints(mitk::ContourModel::Pointer newContourModel,
        TiXmlElement* currentTimeSeries, unsigned int currentTimeStep)
{
  //check if the timesteps in contourModel have to be expanded
  if(currentTimeStep != newContourModel->GetTimeSteps())
  {
    newContourModel->Expand(currentTimeStep+1);
  }

  //read all points within controlPoints tag
  if(currentTimeSeries->FirstChildElement("controlPoints")->FirstChildElement("point") != NULL)
  {
    for( TiXmlElement* currentPoint = currentTimeSeries->FirstChildElement("controlPoints")->FirstChildElement("point")->ToElement();
              currentPoint != NULL; currentPoint = currentPoint->NextSiblingElement())
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
        newContourModel->AddVertex(point, isActivePoint,currentTimeStep);
      }
    }
  else
  {
    //nothing to read
  }

}
