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

#include "mitkImageVtkLegacyIO.h"

#include "mitkImage.h"
#include "mitkIOMimeTypes.h"
#include "mitkImageVtkReadAccessor.h"

#include <vtkStructuredPointsReader.h>
#include <vtkStructuredPointsWriter.h>
#include <vtkStructuredPoints.h>
#include <vtkErrorCode.h>
#include <vtkSmartPointer.h>

namespace mitk {

ImageVtkLegacyIO::ImageVtkLegacyIO()
  : AbstractFileIO(Image::GetStaticNameOfClass(), IOMimeTypes::VTK_IMAGE_LEGACY_MIMETYPE(), "VTK Legacy Image")
{
  this->RegisterService();
}

std::vector<BaseData::Pointer> ImageVtkLegacyIO::Read()
{
  // The legay vtk reader cannot work with input streams
  const std::string fileName = this->GetLocalFileName();
  vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
  reader->SetFileName(fileName.c_str());
  reader->Update();

  if ( reader->GetOutput() != NULL )
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
    mitkThrow() << "vtkStructuredPointsReader error: " << vtkErrorCode::GetStringFromErrorCode(reader->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkLegacyIO::GetReaderConfidenceLevel() const
{
  if (AbstractFileIO::GetReaderConfidenceLevel() == Unsupported) return Unsupported;
  vtkSmartPointer<vtkStructuredPointsReader> reader = vtkSmartPointer<vtkStructuredPointsReader>::New();
  reader->SetFileName(this->GetLocalFileName().c_str());
  if (reader->IsFileStructuredPoints())
  {
    return Supported;
  }
  return Unsupported;
}

void ImageVtkLegacyIO::Write()
{
  ValidateOutputLocation();

  const Image* input = dynamic_cast<const Image*>(this->GetInput());

  vtkSmartPointer<vtkStructuredPointsWriter> writer = vtkSmartPointer<vtkStructuredPointsWriter>::New();

  // The legacy vtk image writer cannot write to streams
  LocalFile localFile(this);
  writer->SetFileName(localFile.GetFileName().c_str());

  ImageVtkReadAccessor vtkReadAccessor(Image::ConstPointer(input), NULL, input->GetVtkImageData());
  writer->SetInputData(const_cast<vtkImageData*>(vtkReadAccessor.GetVtkImageData()));

  if (writer->Write() == 0 || writer->GetErrorCode() != 0 )
  {
    mitkThrow() << "vtkStructuredPointesWriter error: " << vtkErrorCode::GetStringFromErrorCode(writer->GetErrorCode());
  }
}

IFileIO::ConfidenceLevel ImageVtkLegacyIO::GetWriterConfidenceLevel() const
{
  if (AbstractFileIO::GetWriterConfidenceLevel() == Unsupported) return Unsupported;
  const Image* input = static_cast<const Image*>(this->GetInput());
  if (input->GetDimension() == 3) return Supported;
  else if (input->GetDimension() < 3) return PartiallySupported;
  return Unsupported;
}

ImageVtkLegacyIO* ImageVtkLegacyIO::IOClone() const
{
  return new ImageVtkLegacyIO(*this);
}

}
