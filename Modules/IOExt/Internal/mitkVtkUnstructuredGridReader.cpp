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

#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>

mitk::VtkUnstructuredGridReader::VtkUnstructuredGridReader()
  : AbstractFileReader()
{
  CustomMimeType mimeType(IOMimeTypes::DEFAULT_BASE_NAME() + ".vtu");
  mimeType.SetComment("Vtk Unstructured Grid Files");
  mimeType.SetCategory("Vtk Unstructured Grid");
  mimeType.AddExtension("vtu");
  mimeType.AddExtension("vtk");

  this->SetDescription("Vtk Unstructured Grid Files");
  this->SetMimeType(mimeType);

  this->RegisterService();
}

mitk::VtkUnstructuredGridReader::~VtkUnstructuredGridReader()
{}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::VtkUnstructuredGridReader::Read()
{
  MITK_INFO << "Loading " << " as vtk unstructured grid";
  std::vector< itk::SmartPointer<mitk::BaseData> > result;
  MITK_INFO << this->GetLocalFileName();
  std::string ext = itksys::SystemTools::GetFilenameLastExtension(GetLocalFileName().c_str());
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext == ".vtk")
  {
    vtkDataReader *chooser = vtkDataReader::New();
    chooser->SetFileName(GetLocalFileName().c_str() );
    if( chooser->IsFileUnstructuredGrid())
    {
      vtkUnstructuredGridReader* reader = vtkUnstructuredGridReader::New();
      reader->SetFileName( GetLocalFileName().c_str() );
      reader->Update();

      if ( reader->GetOutput() != NULL )
      {
        mitk::UnstructuredGrid::Pointer grid = mitk::UnstructuredGrid::New();
        grid->SetVtkUnstructuredGrid( reader->GetOutput() );
        result.push_back( grid.GetPointer() );
      }
      reader->Delete();
    }
  }
  else if (ext == ".vtu")
  {
    vtkXMLUnstructuredGridReader* reader = vtkXMLUnstructuredGridReader::New();
    reader->SetFileName(GetLocalFileName().c_str() );
    reader->Update();

    if ( reader->GetOutput() != NULL )
    {
      mitk::UnstructuredGrid::Pointer grid = mitk::UnstructuredGrid::New();
      grid->SetVtkUnstructuredGrid( reader->GetOutput() );
      result.push_back( grid.GetPointer() );
    }
    reader->Delete();
  }
  return result;
}

mitk::VtkUnstructuredGridReader* mitk::VtkUnstructuredGridReader::Clone() const
{
  return new mitk::VtkUnstructuredGridReader(*this);
}
