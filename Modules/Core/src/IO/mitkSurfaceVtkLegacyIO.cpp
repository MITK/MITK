/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceVtkLegacyIO.h"
#include "mitkVtkFileIOProgressObserver.h"

#include <mitkIOMimeTypes.h>

#include <vtkErrorCode.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkSmartPointer.h>

#include <cstring>

namespace mitk
{
  SurfaceVtkLegacyIO::SurfaceVtkLegacyIO()
    : SurfaceVtkIO(IOMimeTypes::VTK_POLYDATA_LEGACY_MIMETYPE(), "VTK Legacy PolyData")
  {
    Options defaultOptions;
    defaultOptions["Save as binary file"] = false;
    this->SetDefaultWriterOptions(defaultOptions);
    this->RegisterService();
  }

  vtkSmartPointer<vtkPolyData> SurfaceVtkLegacyIO::ReadPolyData(const std::string &fileName)
  {
    auto reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(fileName.c_str());

    VtkFileIOProgressObserver progress(reader, [this](float p)
      {
        this->ReportReadProgress(p);
      });

    // vtkPolyDataReader reports failures to open the file or read its header
    // only through the error code.
    if (!reader->Update() || reader->GetErrorCode() != 0)
    {
      mitkThrow() << "Could not read \"" << fileName << "\""
                  << (reader->GetErrorCode() ?
                        std::string(": ") + vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode()) :
                        std::string())
                  << ".";
    }

    return reader->GetOutput();
  }

  IFileIO::ConfidenceLevel SurfaceVtkLegacyIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;
    vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
    reader->SetFileName(this->GetLocalFileName().c_str());
    if (reader->IsFilePolyData())
    {
      if (std::strcmp(reader->GetHeader(), "vtk output") == 0)
      {
        return Supported;
      }
      else
        return PartiallySupported;
    }
    return Unsupported;
  }

  void SurfaceVtkLegacyIO::WritePolyData(vtkPolyData *polyData, const std::string &fileName)
  {
    auto writer = vtkSmartPointer<vtkPolyDataWriter>::New();
    writer->SetInputData(polyData);
    writer->SetFileName(fileName.c_str());

    if (us::any_cast<bool>(this->GetWriterOption("Save as binary file")))
      writer->SetFileTypeToBinary();

    VtkFileIOProgressObserver progress(writer, [this](float p)
      {
        this->ReportWriteProgress(p);
      });

    if (writer->Write() == 0 || writer->GetErrorCode() != 0)
    {
      mitkThrow() << "Error during surface writing"
                  << (writer->GetErrorCode() ?
                        std::string(": ") + vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) :
                        std::string());
    }
  }

  SurfaceVtkLegacyIO *SurfaceVtkLegacyIO::IOClone() const { return new SurfaceVtkLegacyIO(*this); }
}
