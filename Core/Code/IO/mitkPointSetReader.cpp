/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkPointSetReader.h"
//#include "vtkPointSetXMLParser.h"
//#include "vtkSmartPointer.h"
#include <iostream>
#include <fstream>
#include <tinyxml.h>

mitk::PointSetReader::PointSetReader()
{
  m_Success = false;
}


mitk::PointSetReader::~PointSetReader()
{}


void mitk::PointSetReader::GenerateData()
{
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

	TiXmlDocument doc(m_FileName.c_str());
	bool loadOkay = doc.LoadFile();
	if (loadOkay)
	{
		mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
		
		TiXmlHandle docHandle( &doc );

		unsigned int pointSetCounter(0);
        for( TiXmlElement* currentPointSetElement = docHandle.FirstChildElement("point_set_file").FirstChildElement( "point_set" ).ToElement(); 
			currentPointSetElement != NULL; currentPointSetElement = currentPointSetElement->NextSiblingElement())
		{	
			mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();

            for( TiXmlElement* currentTimeSeries = currentPointSetElement->FirstChildElement("time_series")->ToElement();
				currentTimeSeries != NULL; currentTimeSeries = currentTimeSeries->NextSiblingElement())
			{
				unsigned int currentTimeStep(0);
				TiXmlElement* currentTimeSeriesID = currentTimeSeries->FirstChildElement("time_series_id");
				currentTimeStep = atoi(currentTimeSeriesID->GetText());
						
				for( TiXmlElement* currentPoint = currentTimeSeries->FirstChildElement("point")->ToElement();
					currentPoint != NULL; currentPoint = currentPoint->NextSiblingElement())
				{
					unsigned int id(0);
                    mitk::PointSpecificationType spec;
					int x(0);
					int y(0);
					int z(0);

					id = atoi(currentPoint->FirstChildElement("id")->GetText());
                    spec = (mitk::PointSpecificationType) atoi(currentPoint->FirstChildElement("specification")->GetText());
					x = atoi(currentPoint->FirstChildElement("x")->GetText());
					y = atoi(currentPoint->FirstChildElement("y")->GetText());
					z = atoi(currentPoint->FirstChildElement("z")->GetText());

					mitk::Point3D point;
					mitk::FillVector3D(point, x, y, z);

					newPointSet->SetPoint(id, point, spec, currentTimeStep);
					MITK_INFO << "PointSet " << id << " " << x << " " << y << " " << currentTimeStep;
				} 
			} 

			this->SetNthOutput( pointSetCounter, newPointSet );
			pointSetCounter++;
		}

	}
	else
	{
		MITK_WARN << "Cannot load XML file!";
	}

	//this->ResizeOutputs( pointSetList.size() );

    //unsigned int i = 0;
    //for ( mitk::vtkPointSetXMLParser::PointSetList::iterator it = pointSetList.begin(); it != pointSetList.end(); ++it, ++i )
    //{
    //    this->SetNthOutput( i, *it );
    //}

    m_Success = true;
}

void mitk::PointSetReader::GenerateOutputInformation()
{
}

int mitk::PointSetReader::CanReadFile ( const char *name )
{
    std::ifstream in( name );
    bool isGood = in.good();
    in.close();
    return isGood;
}

bool mitk::PointSetReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) 
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

void mitk::PointSetReader::ResizeOutputs( const unsigned int& num )
{
    unsigned int prevNum = this->GetNumberOfOutputs();
    this->SetNumberOfOutputs( num );
    for ( unsigned int i = prevNum; i < num; ++i )
    {
        this->SetNthOutput( i, this->MakeOutput( i ).GetPointer() );
    }
}


bool mitk::PointSetReader::GetSuccess() const
{
    return m_Success;  
}
