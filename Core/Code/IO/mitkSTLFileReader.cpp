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
#include "mitkSTLFileReader.h"
#include <mitkSurface.h>
#include <vtkSTLReader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>


mitk::STLFileReader::STLFileReader() : mitk::SurfaceSource(), m_FileName("")
{
}

mitk::STLFileReader::~STLFileReader()
{
}

void mitk::STLFileReader::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();

  if( m_FileName != "")
  {
    std::cout << "Loading " << m_FileName << " as stl..." << std::endl;
    vtkSTLReader* stlReader = vtkSTLReader::New();
    stlReader->SetFileName( m_FileName.c_str() );

    vtkPolyDataNormals* normalsGenerator = vtkPolyDataNormals::New();
    normalsGenerator->SetInput( stlReader->GetOutput() );
    
    normalsGenerator->Update();
    if ( ( stlReader->GetOutput() != NULL ) && ( normalsGenerator->GetOutput() != NULL ) )
    {
      vtkPolyData* surfaceWithNormals = normalsGenerator->GetOutput();
      output->SetVtkPolyData( surfaceWithNormals );
    }
  normalsGenerator->Delete();
  stlReader->Delete();
  }
}

bool mitk::STLFileReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
{
  // First check the extension
  if(  filename == "" )
    {
      //std::cout<<"No filename specified."<<std::endl;
    return false;
    }

  bool extensionFound = false;
  std::string::size_type STLPos = filename.rfind(".stl");
  if ((STLPos != std::string::npos)
      && (STLPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  STLPos = filename.rfind(".STL");
  if ((STLPos != std::string::npos)
      && (STLPos == filename.length() - 4))
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
