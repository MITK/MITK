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
#include <iostream>
#include <fstream>
#include <locale>

mitk::ContourModelReader::ContourModelReader()
{
  m_Success = false;
}


mitk::ContourModelReader::~ContourModelReader()
{}


void mitk::ContourModelReader::GenerateData()
{
    std::locale::global(std::locale("C"));

    m_Success = false;
    if ( m_FileName == "" )
    {
      itkWarningMacro( << "Sorry, filename has not been set!" );
        return ;
    }
    if ( ! this->CanReadFile( m_FileName.c_str() ) )
    {
      itkWarningMacro( << "Sorry, can't read file " << m_FileName << "!" );
        return ;
    }

  try{
    TiXmlDocument doc(m_FileName.c_str());
    bool loadOkay = doc.LoadFile();
    if (loadOkay)
    {
      TiXmlHandle docHandle( &doc );

      //handle geometry information
      TiXmlElement* currentContourElement = docHandle.FirstChildElement("contourModel").FirstChildElement("head").FirstChildElement("geometryInformation").ToElement();


      /*++++ handle n contourModels within data tags ++++*/
      unsigned int contourCounter(0);

      for( TiXmlElement* currentContourElement = docHandle.FirstChildElement("contourModel").FirstChildElement("data").ToElement();
        currentContourElement != NULL; currentContourElement = currentContourElement->NextSiblingElement())
      {
        mitk::ContourModel::Pointer newContourModel = mitk::ContourModel::New();
        if(currentContourElement->FirstChildElement("timestep") != NULL)
        {
          /*++++ handle n timesteps within timestep tags ++++*/
          for( TiXmlElement* currentTimeSeries = currentContourElement->FirstChildElement("timestep")->ToElement();
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
        this->SetNthOutput( contourCounter, newContourModel );
        contourCounter++;
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
      m_Success = false;
   }
    m_Success = true;
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

        mitk::PointSpecificationType spec((mitk::PointSpecificationType) 0);
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

void mitk::ContourModelReader::GenerateOutputInformation()
{
}

int mitk::ContourModelReader::CanReadFile ( const char *name )
{
    std::ifstream in( name );
    bool isGood = in.good();
    in.close();
    return isGood;
}

bool mitk::ContourModelReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
{
  // First check the extension
  if(  filename == "" )
  {
      //MITK_INFO<<"No filename specified."<<std::endl;
    return false;
  }

  // check if image is serie
  if( filePattern != "" && filePrefix != "" )
    return false;

  bool extensionFound = false;
  std::string::size_type MPSPos = filename.rfind(".cnt");
  if ((MPSPos != std::string::npos)
      && (MPSPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  MPSPos = filename.rfind(".CNT");
  if ((MPSPos != std::string::npos)
      && (MPSPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  if( !extensionFound )
    {
      //MITK_INFO<<"The filename extension is not recognized."<<std::endl;
    return false;
    }

  return true;
}

void mitk::ContourModelReader::ResizeOutputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfOutputs();
    this->SetNumberOfOutputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
    }
}


bool mitk::ContourModelReader::GetSuccess() const
{
    return m_Success;
}
