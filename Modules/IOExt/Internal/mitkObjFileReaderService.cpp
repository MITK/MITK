/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkObjFileReaderService.h"
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSurface.h>

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

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::ObjFileReaderService::DoRead()
{
  std::vector<itk::SmartPointer<BaseData>> result;

  vtkSmartPointer<vtkOBJReader> reader = vtkSmartPointer<vtkOBJReader>::New();
  reader->SetFileName(GetInputLocation().c_str());
  reader->Update();

  if (reader->GetOutput() != nullptr)
  {
    mitk::Surface::Pointer surface = mitk::Surface::New();
    surface->SetVtkPolyData(reader->GetOutput());
    result.push_back(dynamic_cast<mitk::BaseData *>(surface.GetPointer()));
  }
  return result;
}

mitk::ObjFileReaderService *mitk::ObjFileReaderService::Clone() const
{
  return new ObjFileReaderService(*this);
}
