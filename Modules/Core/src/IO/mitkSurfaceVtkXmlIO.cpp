/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceVtkXmlIO.h"
#include "mitkVtkFileIOProgressObserver.h"

#include <mitkIOMimeTypes.h>

#include <vtkErrorCode.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>

namespace mitk
{
  SurfaceVtkXmlIO::SurfaceVtkXmlIO()
    : SurfaceVtkIO(IOMimeTypes::VTK_POLYDATA_MIMETYPE(), "VTK XML PolyData")
  {
    // The default among the formats that fully support surfaces without time,
    // since older versions of MITK cannot read VTKHDF.
    this->SetWriterRanking(1);
    this->RegisterService();
  }

  vtkSmartPointer<vtkPolyData> SurfaceVtkXmlIO::ReadPolyData(const std::string &fileName)
  {
    auto reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
    reader->SetFileName(fileName.c_str());

    VtkFileIOProgressObserver progress(reader, [this](float p)
      {
        this->ReportReadProgress(p);
      });

    if (!reader->Update())
      mitkThrow() << "Could not read \"" << fileName << "\".";

    return reader->GetOutput();
  }

  IFileIO::ConfidenceLevel SurfaceVtkXmlIO::GetReaderConfidenceLevel() const
  {
    if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported)
      return Unsupported;
    if (this->GetInputStream() == nullptr)
    {
      // check if the xml vtk reader can handle the file
      auto xmlReader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
      if (xmlReader->CanReadFile(this->GetInputLocation().c_str()) != 0)
      {
        return Supported;
      }
      return Unsupported;
    }
    // in case of an input stream, VTK does not seem to have methods for
    // validating it
    return Supported;
  }

  void SurfaceVtkXmlIO::WritePolyData(vtkPolyData *polyData, const std::string &fileName)
  {
    auto writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
    writer->SetInputData(polyData);
    writer->SetFileName(fileName.c_str());

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

  SurfaceVtkXmlIO *SurfaceVtkXmlIO::IOClone() const { return new SurfaceVtkXmlIO(*this); }
}
