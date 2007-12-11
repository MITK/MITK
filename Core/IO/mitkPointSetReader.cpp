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
#include "vtkPointSetXMLParser.h"
#include <iostream>
#include <fstream>

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
    std::ifstream in( m_FileName.c_str() );
    if ( ! in.good() )
    {
        itkWarningMacro( << "Sorry, can't read file " << m_FileName << "!" );
        in.close();
        return ;
    }
    in.close();

    mitk::vtkPointSetXMLParser* parser = new mitk::vtkPointSetXMLParser();
   parser->SetFileName( m_FileName.c_str() );
    if ( parser->Parse() == 0 ) //Parse returns zero as error indicator
    {
        itkWarningMacro( << "Sorry, an error occurred during parsing!" );
        return ;
    }
    mitk::vtkPointSetXMLParser::PointSetList pointSetList = parser->GetParsedPointSets();
    this->ResizeOutputs( pointSetList.size() );

    unsigned int i = 0;
    for ( mitk::vtkPointSetXMLParser::PointSetList::iterator it = pointSetList.begin(); it != pointSetList.end(); ++it, ++i )
    {
        this->SetNthOutput( i, *it );
    }
    parser->Delete();
    m_Success = true;
}


void mitk::PointSetReader::GenerateOutputInformation()
{
}

int mitk::PointSetReader::CanReadFile ( const char *name )
{
    std::ifstream in( name );
    if ( !in.good() )
    {
        in.close();
        return false;
    }
    else
    {
        in.close();
        return true;
    }
}

bool mitk::PointSetReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) 
{
  // First check the extension
  if(  filename == "" )
  {
      //std::cout<<"No filename specified."<<std::endl;
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
      //std::cout<<"The filename extension is not recognized."<<std::endl;
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
