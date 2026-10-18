/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfacePlyIO.h"

#include <mitkIOMimeTypes.h>

#include <vtkErrorCode.h>
#include <vtkPLYReader.h>
#include <vtkPLYWriter.h>
#include <vtkPolyData.h>

mitk::SurfacePlyIO::SurfacePlyIO()
  : SurfaceVtkIO(IOMimeTypes::STANFORD_PLY_MIMETYPE(), "Stanford Triangle PLY")
{
  this->RegisterService();
}

vtkSmartPointer<vtkPolyData> mitk::SurfacePlyIO::ReadPolyData(const std::string &fileName)
{
  auto reader = vtkSmartPointer<vtkPLYReader>::New();
  reader->SetFileName(fileName.c_str());

  if (!reader->Update())
    mitkThrow() << "Could not read \"" << fileName << "\".";

  return reader->GetOutput();
}

void mitk::SurfacePlyIO::WritePolyData(vtkPolyData *polyData, const std::string &fileName)
{
  auto writer = vtkSmartPointer<vtkPLYWriter>::New();
  writer->SetFileTypeToBinary();
  writer->SetInputData(polyData);
  writer->SetFileName(fileName.c_str());

  // vtkPLYWriter refuses some input without setting an error code.
  if (writer->Write() == 0 || writer->GetErrorCode() != 0)
  {
    mitkThrow() << "Error during surface writing"
                << (writer->GetErrorCode() ?
                      std::string(": ") + vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) :
                      std::string());
  }
}

mitk::SurfacePlyIO *mitk::SurfacePlyIO::IOClone() const
{
  return new SurfacePlyIO(*this);
}
