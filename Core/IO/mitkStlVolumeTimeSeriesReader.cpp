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


#include "mitkStlVolumeTimeSeriesReader.h"
#include "mitkSTLFileReader.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"

void mitk::StlVolumeTimeSeriesReader::GenerateData()
{
  if ( !this->GenerateFileList() )
  {
    itkWarningMacro( << "Sorry, file list could not be determined..." );
    return ;
  }

  mitk::Surface::Pointer surface = this->GetOutput();
  std::cout << "prefix: "<< m_FilePrefix << ", pattern: " <<m_FilePattern << std::endl;
  surface->Resize(m_MatchedFileNames.size());
  for ( unsigned int i = 0 ; i < m_MatchedFileNames.size(); ++i )
  {
    std::string fileName = m_MatchedFileNames[i];
    std::cout << "Loading " << fileName << " as stl..." << std::endl;

    STLFileReader::Pointer stlReader = STLFileReader::New();
    stlReader->SetFileName( fileName.c_str() );
    stlReader->Update();

    if ( stlReader->GetOutput() != NULL )
    {
      surface->SetVtkPolyData( stlReader->GetOutput()->GetVtkPolyData(), i );
    }
    else
    {
      itkWarningMacro(<< "stlReader returned NULL while reading " << fileName << ". Trying to continue with empty vtkPolyData...");
      surface->SetVtkPolyData( vtkPolyData::New(), i ); 
    }
  }
}

bool mitk::StlVolumeTimeSeriesReader::CanReadFile(const std::string /*filename*/, const std::string filePrefix, const std::string filePattern) 
{
  if( filePattern != "" && filePrefix != "" )
    return false;

  bool extensionFound = false;
  std::string::size_type STLPos = filePattern.rfind(".stl");
  if ((STLPos != std::string::npos) && (STLPos == filePattern.length() - 4))
    extensionFound = true;

  STLPos = filePattern.rfind(".STL");
  if ((STLPos != std::string::npos) && (STLPos == filePattern.length() - 4))
    extensionFound = true;

  if( !extensionFound )
    return false;

  return true;
}

mitk::StlVolumeTimeSeriesReader::StlVolumeTimeSeriesReader()
{}

mitk::StlVolumeTimeSeriesReader::~StlVolumeTimeSeriesReader()
{}
