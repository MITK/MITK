/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

void mitk::STLFileReader::GenerateData()
{
    mitk::Surface::Pointer output = this->GetOutput();

    if( m_FileName != "")
    {
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

bool mitk::STLFileReader::CanReadFile(const char* file) 
{
  // First check the extension
  std::string filename = file;
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

  if( !extensionFound )
    {
      //std::cout<<"The filename extension is not recognized."<<std::endl;
    return false;
    }

  return true;
}

mitk::STLFileReader::STLFileReader()
    : m_FileName("")
{
}

mitk::STLFileReader::~STLFileReader()
{
}
