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

#include "mitkContourModelSetReader.h"
#include "mitkContourModelReader.h"
#include <iostream>
#include <fstream>
#include <locale>

mitk::ContourModelSetReader::ContourModelSetReader()
{
  m_Success = false;
}


mitk::ContourModelSetReader::~ContourModelSetReader()
{}


void mitk::ContourModelSetReader::GenerateData()
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
      mitk::ContourModelSet::Pointer contourSet = mitk::ContourModelSet::New();
      this->SetNthOutput(0,contourSet);

      mitk::ContourModelReader::Pointer reader = mitk::ContourModelReader::New();
      reader->SetFileName( this->GetFileName() );
      reader->Update();

      for(unsigned int i = 0; i < reader->GetNumberOfOutputs(); ++i)
      {
        contourSet->AddContourModel( reader->GetOutput(i) );
      }

    }catch(...)
    {
      MITK_ERROR  << "Cannot read contourModel.";
    }
    m_Success = true;
}

void mitk::ContourModelSetReader::ReadPoints(mitk::ContourModel::Pointer newContourModel,
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

void mitk::ContourModelSetReader::GenerateOutputInformation()
{
}

int mitk::ContourModelSetReader::CanReadFile ( const char *name )
{
    std::ifstream in( name );
    bool isGood = in.good();
    in.close();
    return isGood;
}

bool mitk::ContourModelSetReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
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
  std::string::size_type MPSPos = filename.rfind(".cnt_set");
  if ((MPSPos != std::string::npos)
      && (MPSPos == filename.length() - 8))
    {
    extensionFound = true;
    }

  MPSPos = filename.rfind(".CNT_SET");
  if ((MPSPos != std::string::npos)
      && (MPSPos == filename.length() - 8))
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

void mitk::ContourModelSetReader::ResizeOutputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfOutputs();
    this->SetNumberOfIndexedOutputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
    }
}


bool mitk::ContourModelSetReader::GetSuccess() const
{
    return m_Success;
}
