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

//MITK
#include "mitkPlyFileReaderService.h"
#include <mitkSurface.h>
#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

// VTK
#include <vtkPLYReader.h>
#include <vtkSmartPointer.h>


mitk::PlyFileReaderService::PlyFileReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::STANFORD_PLY_MIMETYPE()), "Stanford Triangle PLY Reader")
{
  this->RegisterService();
}

mitk::PlyFileReaderService::~PlyFileReaderService()
{
}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::PlyFileReaderService::Read()
{
  std::vector< itk::SmartPointer<BaseData> > result;

  vtkSmartPointer <vtkPLYReader> reader = vtkSmartPointer <vtkPLYReader>::New();
  reader->SetFileName( GetInputLocation().c_str() );
  reader->Update();

  if (reader->GetOutput() != NULL){
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(reader->GetOutput());
    result.push_back(dynamic_cast<mitk::BaseData*> (surface.GetPointer()));
  }
  return result;
}


mitk::PlyFileReaderService* mitk::PlyFileReaderService::Clone() const
{
  return new PlyFileReaderService(*this);
}
