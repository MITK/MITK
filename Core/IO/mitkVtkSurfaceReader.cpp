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
#include "mitkVtkSurfaceReader.h"
#include <mitkSurface.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>


mitk::VtkSurfaceReader::VtkSurfaceReader()
    : m_FileName("")
{
}

mitk::VtkSurfaceReader::~VtkSurfaceReader()
{
}

void mitk::VtkSurfaceReader::GenerateData()
{
  if( m_FileName != "")
  {
  std::cout << "Loading " << m_FileName << " as vtk" << std::flush;

  ///We create a Generic Reader to test de .vtk/
  vtkDataReader *chooser=vtkDataReader::New();
  chooser->SetFileName(m_FileName.c_str() );

  if( chooser->IsFilePolyData())
  {
    ///PolyData/
    std::cout << "PolyData" << std::endl;
    vtkPolyDataReader *reader = vtkPolyDataReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();

    if ( reader->GetOutput() != NULL )
    {
      mitk::Surface::Pointer output = this->GetOutput();
      output->SetVtkPolyData( reader->GetOutput() );
    }
    reader->Delete();
  }
  else
  {
    std::cerr << " ... sorry, this .vtk format is not supported yet."<<std::endl;
  }
  chooser->Delete();
  }
}

bool mitk::VtkSurfaceReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern) 
{
  // First check the extension
  if(  filename == "" )
    return false;

  bool extensionFound = false;
  std::string::size_type VtkPos = filename.rfind(".vtk");
  if ((VtkPos != std::string::npos)
      && (VtkPos == filename.length() - 4))
    {
    extensionFound = true;
    }
  VtkPos = filename.rfind(".pvtk");
  if ((VtkPos != std::string::npos)
      && (VtkPos == filename.length() - 5))
    {
    extensionFound = true;
    }

  if (extensionFound)
  {
    vtkDataReader *chooser=vtkDataReader::New();
    chooser->SetFileName(filename.c_str() );
    if(!chooser->IsFilePolyData())
      return false;

  }
  else
    return false;

  return true;
}
