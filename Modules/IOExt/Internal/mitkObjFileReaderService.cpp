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
#include "mitkObjFileReaderService.h"
#include <mitkSurface.h>
#include <mitkIOMimeTypes.h>
#include <mitkCustomMimeType.h>

// VTK
#include <vtkOBJReader.h>
#include <vtkSmartPointer.h>


mitk::ObjFileReaderService::ObjFileReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::WAVEFRONT_OBJ_MIMETYPE()), "Wavefront OBJ Reader")
{
  this->RegisterService();
}

mitk::ObjFileReaderService::~ObjFileReaderService()
{
}

std::vector< itk::SmartPointer<mitk::BaseData> > mitk::ObjFileReaderService::Read()
{
  std::vector< itk::SmartPointer<BaseData> > result;

  vtkSmartPointer <vtkOBJReader> reader = vtkSmartPointer <vtkOBJReader>::New();
  reader->SetFileName( GetInputLocation().c_str() );
  reader->Update();

  if (reader->GetOutput() != NULL){
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(reader->GetOutput());
    result.push_back(dynamic_cast<mitk::BaseData*> (surface.GetPointer()));
  }
  return result;
}


mitk::ObjFileReaderService* mitk::ObjFileReaderService::Clone() const
{
  return new ObjFileReaderService(*this);
}
