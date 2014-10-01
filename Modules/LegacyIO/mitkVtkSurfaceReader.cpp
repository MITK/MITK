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
#include "mitkVtkSurfaceReader.h"
#include <mitkSurface.h>
#include <vtkDataReader.h>
#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>
#include <itksys/SystemTools.hxx>


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
    bool success = false;
    MITK_INFO << "Loading " << m_FileName << " as vtk" << std::endl;

    std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
    ext = itksys::SystemTools::LowerCase(ext);
    if (ext == ".vtk")
    {
      ///We create a Generic Reader to test de .vtk/
      vtkDataReader *chooser=vtkDataReader::New();
      chooser->SetFileName(m_FileName.c_str() );
      if( chooser->IsFilePolyData())
      {
        ///PolyData/
        itkDebugMacro( << "PolyData" );
        vtkPolyDataReader *reader = vtkPolyDataReader::New();
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          mitk::Surface::Pointer output = this->GetOutput();
          output->SetVtkPolyData( reader->GetOutput() );
          success = true;
        }
        reader->Delete();
      }
      chooser->Delete();
    }
    else
    if (ext == ".vtp")
    {
      vtkXMLPolyDataReader *reader=vtkXMLPolyDataReader::New();
      if( reader->CanReadFile(m_FileName.c_str()) )
      {
        ///PolyData/
        itkDebugMacro( << "XMLPolyData" );
        reader->SetFileName( m_FileName.c_str() );
        reader->Update();

        if ( reader->GetOutput() != NULL )
        {
          mitk::Surface::Pointer output = this->GetOutput();
          output->SetVtkPolyData( reader->GetOutput() );
          success = true;
        }
        reader->Delete();
      }
    }
    if(!success)
    {
      itkWarningMacro( << " ... sorry, this .vtk format is not supported yet." );
    }
  }
}

bool mitk::VtkSurfaceReader::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
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
    if(!chooser->IsFilePolyData())
    {
      chooser->Delete();
      return false;
    }
    chooser->Delete();
  }
  else
    if (ext == ".vtp")
    {
      vtkXMLPolyDataReader *chooser=vtkXMLPolyDataReader::New();
      if(!chooser->CanReadFile(filename.c_str()))
      {
        chooser->Delete();
        return false;
      }
      chooser->Delete();
    }
    else
      return false;

  return true;
}
