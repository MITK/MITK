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

#include "mitkImageVtkXmlIO.h"

#include "mitkImage.h"
#include "mitkIOMimeTypes.h"
#include "mitkImageVtkReadAccessor.h"

#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkImageData.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>

namespace mitk {

class VtkXMLImageDataReader : public ::vtkXMLImageDataReader
{
public:
  static VtkXMLImageDataReader *New() { return new VtkXMLImageDataReader(); }
  vtkTypeMacro(VtkXMLImageDataReader,vtkXMLImageDataReader)

  void SetStream(std::istream* is) { this->Stream = is; }
  std::istream* GetStream() const { return this->Stream; }
};

class VtkXMLImageDataWriter : public ::vtkXMLImageDataWriter
{
public:
  static VtkXMLImageDataWriter *New() { return new VtkXMLImageDataWriter(); }
  vtkTypeMacro(VtkXMLImageDataWriter,vtkXMLImageDataWriter)

  void SetStream(std::ostream* os) { this->Stream = os; }
  std::ostream* GetStream() const { return this->Stream; }
};

ImageVtkXmlIO::ImageVtkXmlIO()
  : AbstractFileIO(Image::GetStaticNameOfClass(), IOMimeTypes::VTK_IMAGE_MIMETYPE(), "VTK XML Image")
{
  this->RegisterService();
}

std::vector<BaseData::Pointer> ImageVtkXmlIO::Read()
{
  vtkSmartPointer<VtkXMLImageDataReader> reader = vtkSmartPointer<VtkXMLImageDataReader>::New();
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
    mitk::Image::Pointer output = mitk::Image::New();
    output->Initialize(reader->GetOutput());
    output->SetVolume(reader->GetOutput()->GetScalarPointer());
    std::vector<BaseData::Pointer> result;
    result.push_back(output.GetPointer());
    return result;
  }
  else
  {
    mitkThrow() << "vtkXMLImageDataReader error: " << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkXmlIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  if (this->GetInputStream() == NULL)
  {
    // check if the xml vtk reader can handle the file
    vtkSmartPointer<VtkXMLImageDataReader> xmlReader = vtkSmartPointer<VtkXMLImageDataReader>::New();
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

void ImageVtkXmlIO::Write()
{
  ValidateOutputLocation();

  const Image* input = dynamic_cast<const Image*>(this->GetInput());

  vtkSmartPointer<VtkXMLImageDataWriter> writer = vtkSmartPointer<VtkXMLImageDataWriter>::New();
  if (this->GetOutputStream())
  {
    writer->SetStream(this->GetOutputStream());
  }
  else
  {
    writer->SetFileName(this->GetOutputLocation().c_str());
  }

  ImageVtkReadAccessor vtkReadAccessor(Image::ConstPointer(input), NULL, input->GetVtkImageData());
  writer->SetInputData(const_cast<vtkImageData*>(vtkReadAccessor.GetVtkImageData()));

  if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
  {
    mitkThrow() << "vtkXMLImageDataWriter error: " << vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkXmlIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported) return Unsupported;
  const Image* input = static_cast<const Image*>(this->GetInput());
  if (input->GetDimension() == 3) return Supported;
  else if (input->GetDimension() < 3) return PartiallySupported;
  return Unsupported;
}

ImageVtkXmlIO* ImageVtkXmlIO::IOClone() const
{
  return new ImageVtkXmlIO(*this);
}

}
