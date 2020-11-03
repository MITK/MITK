/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// MITK
#include "mitkPlyFileWriterService.h"
#include <mitkIOMimeTypes.h>
#include <mitkSurface.h>

// VTK
#include <vtkDataObject.h>
#include <vtkPLYWriter.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

mitk::PlyFileWriterService::PlyFileWriterService()
  : AbstractFileWriter(mitk::Surface::GetStaticNameOfClass(),
                       CustomMimeType(IOMimeTypes::STANFORD_PLY_MIMETYPE()),
                       "Stanford Triangle PLY Writer")
{
  RegisterService();
}

mitk::PlyFileWriterService::PlyFileWriterService(const mitk::PlyFileWriterService &other) : AbstractFileWriter(other)
{
}

mitk::PlyFileWriterService::~PlyFileWriterService()
{
}

void mitk::PlyFileWriterService::Write()
{
  vtkSmartPointer<vtkPLYWriter> writer = vtkSmartPointer<vtkPLYWriter>::New();
  writer->SetFileTypeToBinary();
  writer->SetFileName(GetOutputLocation().c_str());

  const mitk::Surface *surface = static_cast<const mitk::Surface *>(this->GetInput());
  writer->SetInputDataObject(surface->GetVtkPolyData());

  writer->Write();
}

mitk::PlyFileWriterService *mitk::PlyFileWriterService::Clone() const
{
  return new PlyFileWriterService(*this);
}
