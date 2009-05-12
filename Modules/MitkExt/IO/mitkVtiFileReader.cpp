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

#include "mitkVtiFileReader.h"

#include <vtkImageData.h>
#include <vtkXMLImageDataReader.h>

mitk::VtiFileReader::VtiFileReader()
: m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::VtiFileReader::~VtiFileReader()
{
}

void mitk::VtiFileReader::GenerateData()
{
  if( m_FileName != "")
  {
  vtkXMLImageDataReader * vtkReader = vtkXMLImageDataReader::New();
  vtkReader->SetFileName( m_FileName.c_str() );
  vtkReader->Update();

  if ( vtkReader->GetOutput() != NULL )
  {
    mitk::Image::Pointer output = this->GetOutput();
    output->Initialize( vtkReader->GetOutput() );
    output->SetVolume( vtkReader->GetOutput()->GetScalarPointer() );
  }
  vtkReader->Delete();
  }
}

bool mitk::VtiFileReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
{
  // First check the extension
  if(  filename == "" )
    return false;

  bool extensionFound = false;
  std::string::size_type VTIPos = filename.rfind(".vti");
  if ((VTIPos != std::string::npos) && (VTIPos == filename.length() - 4))
    extensionFound = true;

  VTIPos = filename.rfind(".VTI");
  if ((VTIPos != std::string::npos) && (VTIPos == filename.length() - 4))
    extensionFound = true;

  if (!extensionFound)
    return false;

  return true;
}
