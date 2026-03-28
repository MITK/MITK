/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkPlyFileReaderService.h"
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <mitkSurface.h>

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

std::vector<itk::SmartPointer<mitk::BaseData>> mitk::PlyFileReaderService::DoRead()
{
  std::vector<itk::SmartPointer<BaseData>> result;

  vtkSmartPointer<vtkPLYReader> reader = vtkSmartPointer<vtkPLYReader>::New();
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

mitk::PlyFileReaderService *mitk::PlyFileReaderService::Clone() const
{
  return new PlyFileReaderService(*this);
}
