/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSurfaceStlIO.h"
#include "mitkVtkFileIOProgressObserver.h"

#include <mitkIOMimeTypes.h>
#include <mitkLocaleSwitch.h>

#include <vtkErrorCode.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSTLReader.h>
#include <vtkSTLWriter.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

namespace mitk
{
  SurfaceStlIO::SurfaceStlIO()
    : SurfaceVtkIO(IOMimeTypes::STEREOLITHOGRAPHY_MIMETYPE(), "Stereolithography")
  {
    this->RegisterService();
  }

  vtkSmartPointer<vtkPolyData> SurfaceStlIO::ReadPolyData(const std::string &fileName)
  {
    LocaleSwitch localeSwitch("C");

    // STL repeats the points of each triangle. The reader merges them and
    // drops the triangles that collapse in doing so.
    auto stlReader = vtkSmartPointer<vtkSTLReader>::New();
    stlReader->SetFileName(fileName.c_str());

    // STL holds no point normals. Splitting would duplicate the points along
    // sharp edges and leave the surface unwelded there.
    auto normalsGenerator = vtkSmartPointer<vtkPolyDataNormals>::New();
    normalsGenerator->SetInputConnection(stlReader->GetOutputPort());
    normalsGenerator->SplittingOff();

    // The reader rather than the tail of the pipeline: reading the file is
    // the part that grows with the file, and each filter reports only its
    // own share of the work.
    VtkFileIOProgressObserver progress(stlReader, [this](float p)
      {
        this->ReportReadProgress(p);
      });

    if (!normalsGenerator->Update())
      mitkThrow() << "Could not read \"" << fileName << "\".";

    return normalsGenerator->GetOutput();
  }

  void SurfaceStlIO::WritePolyData(vtkPolyData *polyData, const std::string &fileName)
  {
    LocaleSwitch localeSwitch("C");

    vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(polyData);
    vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
    writer->SetInputConnection(triangleFilter->GetOutputPort());
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

  SurfaceStlIO *SurfaceStlIO::IOClone() const { return new SurfaceStlIO(*this); }
}
