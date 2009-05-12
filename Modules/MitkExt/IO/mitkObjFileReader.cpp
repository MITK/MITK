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
#include "mitkObjFileReader.h"
#include <mitkSurface.h>
#include <vtkOBJReader.h>


mitk::ObjFileReader::ObjFileReader()
    : m_FileName("")
{
}

mitk::ObjFileReader::~ObjFileReader()
{
}

void mitk::ObjFileReader::GenerateData()
{
  mitk::Surface::Pointer output = this->GetOutput();

  if( m_FileName != "")
  {
    std::cout << "Loading " << m_FileName << " as obj..." << std::endl;

    vtkOBJReader *reader = vtkOBJReader::New();
    reader->SetFileName( m_FileName.c_str() );
    reader->Update();

    if ( reader->GetOutput() != NULL )
      output->SetVtkPolyData( reader->GetOutput() );

    reader->Delete();

    std::cout << "...finished!" << std::endl;
  }
}

bool mitk::ObjFileReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
{
  // First check the extension
  //std::string filename = file;
  if(  filename == "" )
    {
      //std::cout<<"No filename specified."<<std::endl;
    return false;
    }

  bool extensionFound = false;
  std::string::size_type ObjPos = filename.rfind(".obj");
  if ((ObjPos != std::string::npos)
      && (ObjPos == filename.length() - 4))
    {
    extensionFound = true;
    }

  ObjPos = filename.rfind(".OBJ");
  if ((ObjPos != std::string::npos)
      && (ObjPos == filename.length() - 4))
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
