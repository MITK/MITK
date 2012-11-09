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


#include "mitkVtkVolumeTimeSeriesReader.h"
#include "mitkVtkSurfaceReader.h"
#include "mitkSurface.h"
#include "vtkPolyData.h"

void mitk::VtkVolumeTimeSeriesReader::GenerateData()
{
  if ( !this->GenerateFileList() )
  {
    itkWarningMacro( << "Sorry, file list could not be determined..." );
    return ;
  }

  mitk::Surface::Pointer output = this->GetOutput();
  MITK_INFO << "prefix: "<< m_FilePrefix << ", pattern: " <<m_FilePattern << std::endl;
  output->Expand(m_MatchedFileNames.size());
  for ( unsigned int i = 0 ; i < m_MatchedFileNames.size(); ++i )
  {
    std::string fileName = m_MatchedFileNames[i];
    MITK_INFO << "Loading " << fileName << " as vtk..." << std::endl;

    VtkSurfaceReader::Pointer vtkReader = VtkSurfaceReader::New();
    vtkReader->SetFileName( fileName.c_str() );
    vtkReader->Update();

    if ( vtkReader->GetOutput() != NULL )
    {
      output->SetVtkPolyData( vtkReader->GetOutput()->GetVtkPolyData(), i );
    }
    else
    {
      itkWarningMacro(<< "vtkPolyDataReader returned NULL while reading " << fileName << ". Trying to continue with empty vtkPolyData...");
      output->SetVtkPolyData( vtkPolyData::New(), i );
    }
  }
}

bool mitk::VtkVolumeTimeSeriesReader::CanReadFile(const std::string /*filename*/, const std::string filePrefix, const std::string filePattern)
{
  if( filePattern != "" && filePrefix != "" )
    return false;

  bool extensionFound = false;
  std::string::size_type VTKPos = filePattern.rfind(".vtk");
  if ((VTKPos != std::string::npos) && (VTKPos == filePattern.length() - 4))
    extensionFound = true;

  VTKPos = filePattern.rfind(".VTK");
  if ((VTKPos != std::string::npos) && (VTKPos == filePattern.length() - 4))
    extensionFound = true;

  if( !extensionFound )
    return false;

  return true;
}

mitk::VtkVolumeTimeSeriesReader::VtkVolumeTimeSeriesReader()
{}

mitk::VtkVolumeTimeSeriesReader::~VtkVolumeTimeSeriesReader()
{}
