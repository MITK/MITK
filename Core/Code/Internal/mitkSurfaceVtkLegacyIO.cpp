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

#include "mitkSurfaceVtkLegacyIO.h"

#include "mitkSurface.h"
#include "mitkIOMimeTypes.h"

#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>

namespace mitk {

SurfaceVtkLegacyIO::SurfaceVtkLegacyIO()
  : SurfaceVtkIO(Surface::GetStaticNameOfClass(), IOMimeTypes::VTK_POLYDATA_LEGACY_MIMETYPE(), "VTK Legacy PolyData")
{
  this->RegisterService();
}

std::vector<itk::SmartPointer<BaseData> > SurfaceVtkLegacyIO::Read()
{
  mitk::Surface::Pointer output = mitk::Surface::New();

  // The legay vtk reader cannot work with input streams
  const std::string fileName = this->GetLocalFileName();
  vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();

  if ( reader->GetOutput() != NULL )
  {
    output->SetVtkPolyData(reader->GetOutput());
  }
  else
  {
    mitkThrow() << "vtkPolyDataReader error: " << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode());
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(output.GetPointer());
  return result;
}

IFileIO::ConfidenceLevel SurfaceVtkLegacyIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
  reader->SetFileName(this->GetLocalFileName().c_str());
  if (reader->IsFilePolyData())
  {
    return Supported;
  }
  return Unsupported;
}

void SurfaceVtkLegacyIO::Write()
{
  ValidateOutputLocation();

  const Surface* input = dynamic_cast<const Surface*>(this->GetInput());

  const unsigned int timesteps = input->GetTimeGeometry()->CountTimeSteps();
  for(unsigned int t = 0; t < timesteps; ++t)
  {
    std::string fileName;
    vtkSmartPointer<vtkPolyData> polyData = this->GetPolyData(t, fileName);
    vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
    writer->SetInputData(polyData);

    // The legacy vtk poly data writer cannot write to streams
    LocalFile localFile(this);
    writer->SetFileName(localFile.GetFileName().c_str());

    if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
    {
      mitkThrow() << "Error during surface writing" << (writer->GetErrorCode() ?
                                                          std::string(": ") + vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) :
                                                          std::string());
    }

    if (this->GetOutputStream() && input->GetTimeGeometry()->CountTimeSteps() > 1)
    {
      MITK_WARN << "Writing multiple time-steps to output streams is not supported. "
                << "Only the first time-step will be written";
      break;
    }
  }
}


SurfaceVtkLegacyIO* SurfaceVtkLegacyIO::IOClone() const
{
  return new SurfaceVtkLegacyIO(*this);
}

}
