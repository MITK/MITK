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
#include "mitkVtkUnstructuredGridReader.h"
#include <mitkUnstructuredGrid.h>
#include <vtkDataReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridReader.h>

#include <itksys/SystemTools.hxx>


mitk::VtkUnstructuredGridReader::VtkUnstructuredGridReader()
: m_FileName("")
{
}

mitk::VtkUnstructuredGridReader::~VtkUnstructuredGridReader()
{
}

void mitk::VtkUnstructuredGridReader::GenerateData()
{
  if( m_FileName != "")
  {
    bool success = false;
    MITK_INFO << "Loading " << m_FileName << " as vtk unstructured grid" << std::endl;

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
    ext = itksys::SystemTools::LowerCase(ext);
    if (ext == ".vtk")
    {
      ///We create a Generic Reader to test de .vtk/
      vtkDataReader *chooser=vtkDataReader::New();
      chooser->SetFileName(m_FileName.c_str() );
      if( chooser->IsFileUnstructuredGrid())
      {
        ///UnstructuredGrid/
        itkDebugMacro( << "UnstructuredGrid" );
        vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          mitk::UnstructuredGrid::Pointer output = this->GetOutput();
          output->SetVtkUnstructuredGrid( reader->GetOutput() );
          success = true;
        }
        reader->Delete();
      }
    }
    else if (ext == ".vtu")
    {
      vtkXMLUnstructuredGridReader *reader=vtkXMLUnstructuredGridReader::New();
      if( reader->CanReadFile(m_FileName.c_str()) )
      {
        ///UnstructuredGrid/
        itkDebugMacro( << "XMLUnstructuredGrid" );
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          mitk::UnstructuredGrid::Pointer output = this->GetOutput();
          output->SetVtkUnstructuredGrid( reader->GetOutput() );
          success = true;
        }
        reader->Delete();
      }
    }
    if(!success)
    {
      itkExceptionMacro( << " ... sorry, this .vtk format is not supported yet." );
    }
  }
}

bool mitk::VtkUnstructuredGridReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
{
  // First check the extension
  if(  filename == "" )
    return false;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext == ".vtk")
  {
    vtkDataReader *chooser=vtkDataReader::New();
    chooser->SetFileName(filename.c_str() );
    if(!chooser->IsFileUnstructuredGrid())
    {
      chooser->Delete();
      return false;
    }
  }
  else if (ext == ".vtu")
    {
      vtkXMLUnstructuredGridReader *chooser=vtkXMLUnstructuredGridReader::New();
      if(!chooser->CanReadFile(filename.c_str()))
      {
        chooser->Delete();
        return false;
      }
    }
    else
      return false;

  return true;
}
