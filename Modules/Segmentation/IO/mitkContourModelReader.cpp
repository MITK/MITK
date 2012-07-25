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
{/*
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
      unsigned int contourCounter(0);
      for( TiXmlElement* currentContourModelElement = docHandle.FirstChildElement("point_set_file").FirstChildElement("point_set").ToElement();
        currentContourModelElement != NULL; currentContourModelElement = currentContourModelElement->NextSiblingElement())
      {
        mitk::ContourModel::Pointer newContourModel = mitk::ContourModel::New();
        if(currentContourModelElement->FirstChildElement("time_series") != NULL)
        {
          for( TiXmlElement* currentTimeSeries = currentContourModelElement->FirstChildElement("time_series")->ToElement();
            currentTimeSeries != NULL; currentTimeSeries = currentTimeSeries->NextSiblingElement())
          {
            unsigned int currentTimeStep(0);
            TiXmlElement* currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");

            currentTimeStep = atoi(currentTimeSeriesID->GetText());

            newContourModel = this->ReadPoint(newContourModel, currentTimeSeries, currentTimeStep);
          }
        } 
        else 
        {
          newContourModel = this->ReadPoint(newContourModel, currentContourModelElement, 0);
        }
        this->SetNthOutput( ContourModelCounter, newContourModel );
        contourCounter++;
      }
    }
    else
    {
      MITK_WARN << "XML parser error!";
    }
  }catch(...)
   {
      MITK_ERROR  << "Cannot read point set.";
      m_Success = false;
   }
    m_Success = true;*/
}

mitk::ContourModel::Pointer mitk::ContourModelReader::ReadPoint(mitk::ContourModel::Pointer newContourModel, 
        TiXmlElement* currentTimeSeries, unsigned int currentTimeStep) 
{/*
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
        newContourModel->SetPoint(id, point, spec, currentTimeStep);
      }
    }
  else
  {
    if(currentTimeStep != newContourModel->GetTimeSteps()+1)
    {
      newContourModel->Expand(currentTimeStep+1);     // expand time step series with empty time step
    }
  }
    return newContourModel;*/return NULL;
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
  std::string::size_type MPSPos = filename.rfind(".mps");
  if ((MPSPos != std::string::npos)
      && (MPSPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  MPSPos = filename.rfind(".MPS");
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
