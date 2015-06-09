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

// MITK
#include "mitkPlyFileWriterService.h"
#include <mitkIOMimeTypes.h>
#include <mitkSurface.h>

// VTK
#include <vtkPLYWriter.h>
#include <vtkSmartPointer.h>
#include <vtkDataObject.h>
#include <vtkPolyData.h>


mitk::PlyFileWriterService::PlyFileWriterService()
  : AbstractFileWriter(mitk::Surface::GetStaticNameOfClass(),
                       CustomMimeType(IOMimeTypes::STANFORD_PLY_MIMETYPE()), "Stanford Triangle PLY Writer")
{
  RegisterService();
}

mitk::PlyFileWriterService::PlyFileWriterService(const mitk::PlyFileWriterService& other)
  : AbstractFileWriter(other)
{
}

mitk::PlyFileWriterService::~PlyFileWriterService()
{}

void mitk::PlyFileWriterService::Write()
{
  vtkSmartPointer<vtkPLYWriter> writer = vtkSmartPointer<vtkPLYWriter>::New();
  writer->SetFileTypeToBinary();
  writer->SetFileName(GetOutputLocation().c_str());

  const mitk::Surface* surface = static_cast<const mitk::Surface*> (this->GetInput());
  writer->SetInputDataObject(surface->GetVtkPolyData());

  writer->Write();
}

mitk::PlyFileWriterService* mitk::PlyFileWriterService::Clone() const
{
  return new PlyFileWriterService(*this);
}
