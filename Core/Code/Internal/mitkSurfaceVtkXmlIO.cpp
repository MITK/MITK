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

#include "mitkSurfaceVtkXmlIO.h"

#include "mitkSurface.h"
#include "mitkIOMimeTypes.h"

#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>

namespace mitk {

class VtkXMLPolyDataReader : public ::vtkXMLPolyDataReader
{
public:
  static VtkXMLPolyDataReader *New() { return new VtkXMLPolyDataReader(); }
  vtkTypeMacro(VtkXMLPolyDataReader,vtkXMLPolyDataReader)

  void SetStream(std::istream* is) { this->Stream = is; }
  std::istream* GetStream() const { return this->Stream; }
};

class VtkXMLPolyDataWriter : public ::vtkXMLPolyDataWriter
{
public:
  static VtkXMLPolyDataWriter *New() { return new VtkXMLPolyDataWriter(); }
  vtkTypeMacro(VtkXMLPolyDataWriter,vtkXMLPolyDataWriter)

  void SetStream(std::ostream* os) { this->Stream = os; }
  std::ostream* GetStream() const { return this->Stream; }
};

SurfaceVtkXmlIO::SurfaceVtkXmlIO()
  : SurfaceVtkIO(Surface::GetStaticNameOfClass(), IOMimeTypes::VTK_POLYDATA_MIMETYPE(), "VTK XML PolyData")
{
  this->RegisterService();
}

std::vector<itk::SmartPointer<BaseData> > SurfaceVtkXmlIO::Read()
{
  mitk::Surface::Pointer output = mitk::Surface::New();

  vtkSmartPointer<VtkXMLPolyDataReader> reader= vtkSmartPointer<VtkXMLPolyDataReader>::New();
  if (this->GetInputStream())
  {
    reader->SetStream(this->GetInputStream());
  }
  else
  {
    reader->SetFileName(this->GetInputLocation().c_str());
  }
  reader->Update();

  if (reader->GetOutput() != NULL)
  {
    output->SetVtkPolyData(reader->GetOutput());
  }
  else
  {
    mitkThrow() << "vtkXMLPolyDataReader error: " << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode());
  }

  std::vector<BaseData::Pointer> result;
  result.push_back(output.GetPointer());
  return result;
}

IFileIO::ConfidenceLevel SurfaceVtkXmlIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  if (this->GetInputStream() == NULL)
  {
    // check if the xml vtk reader can handle the file
    vtkSmartPointer<VtkXMLPolyDataReader> xmlReader = vtkSmartPointer<VtkXMLPolyDataReader>::New();
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

void SurfaceVtkXmlIO::Write()
{
  ValidateOutputLocation();

  const Surface* input = dynamic_cast<const Surface*>(this->GetInput());

  const unsigned int timesteps = input->GetTimeGeometry()->CountTimeSteps();
  for(unsigned int t = 0; t < timesteps; ++t)
  {
    std::string fileName;
    vtkSmartPointer<vtkPolyData> polyData = this->GetPolyData(t, fileName);
    if (polyData.Get() == NULL)
    {
      mitkThrow() << "Cannot write empty surface";
    }

    vtkSmartPointer<VtkXMLPolyDataWriter> writer = vtkSmartPointer<VtkXMLPolyDataWriter>::New();
    writer->SetInputData(polyData);

    if (this->GetOutputStream())
    {
      if (input->GetTimeGeometry()->CountTimeSteps() > 1)
      {
        MITK_WARN << "Writing multiple time-steps to output streams is not supported. "
                  << "Only the first time-step will be written";
      }
      writer->SetStream(this->GetOutputStream());
    }
    else
    {
      writer->SetFileName(fileName.c_str());
    }

    if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
    {
      mitkThrow() << "Error during surface writing" << (writer->GetErrorCode() ?
                                                          std::string(": ") + vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode()) :
                                                          std::string());
    }

    if (this->GetOutputStream()) break;
  }
}

SurfaceVtkXmlIO* SurfaceVtkXmlIO::IOClone() const
{
  return new SurfaceVtkXmlIO(*this);
}

}
