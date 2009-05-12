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

#include "mitkVtkImageReader.h"

#include <vtkDataReader.h>
#include <vtkStructuredPoints.h>
#include <vtkStructuredPointsReader.h>

mitk::VtkImageReader::VtkImageReader()
: m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::VtkImageReader::~VtkImageReader()
{
}

void mitk::VtkImageReader::GenerateData()
{
  if( m_FileName != "")
  {
    std::cout << "Loading " << m_FileName << " as vtk" << std::endl;

    ///We create a Generic Reader to test de .vtk/
    vtkDataReader *chooser=vtkDataReader::New();
    chooser->SetFileName(m_FileName.c_str() );

    if(chooser->IsFileStructuredPoints())
    {
      ///StructuredPoints/
      std::cout << "StructuredPoints"<< std::endl;
      vtkStructuredPointsReader *reader=vtkStructuredPointsReader::New();
      reader->SetFileName(m_FileName.c_str());
      reader->Update();

      if ( reader->GetOutput() != NULL )
      {
        mitk::Image::Pointer output = this->GetOutput();
        output->Initialize( reader->GetOutput() );
        output->SetVolume(  reader->GetOutput()->GetScalarPointer());
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

bool mitk::VtkImageReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
{
  // First check the extension
  if(  filename == "" )
    return false;

  bool extensionFound = false;
  std::string::size_type PVTKPos = filename.rfind(".pvtk");
  if ((PVTKPos != std::string::npos)
      && (PVTKPos == filename.length() - 5))
    {
    extensionFound = true;
    }

  PVTKPos = filename.rfind(".PVTK");
  if ((PVTKPos != std::string::npos)
      && (PVTKPos == filename.length() - 5))
    {
    extensionFound = true;
    }

  if (extensionFound)
  {
    vtkDataReader *chooser=vtkDataReader::New();
    chooser->SetFileName(filename.c_str() );
    if(!chooser->IsFileStructuredPoints())
      return false;
  }
  else
    return false;

  return true;
}
