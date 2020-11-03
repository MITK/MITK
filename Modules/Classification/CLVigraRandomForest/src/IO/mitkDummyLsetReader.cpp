/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkDummyLsetReader.h>
#include <itksys/SystemTools.hxx>
#include <itkImageFileReader.h>
#include <itkNrrdImageIO.h>
#include <mitkImageCast.h>
#include <mitkCustomMimeType.h>

typedef itk::Image<unsigned char, 3> ImageType;

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::DummyLsetFileReader::DoRead()
{

  std::vector<itk::SmartPointer<mitk::BaseData> > result;
  typedef itk::ImageFileReader<ImageType> FileReaderType;
  FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName(this->GetInputLocation());
  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  reader->SetImageIO(io);
  reader->Update();

  mitk::Image::Pointer img;
  mitk::CastToMitkImage(reader->GetOutput(),img);
  result.push_back(img.GetPointer());

  return result;
}


mitk::DummyLsetFileReader::DummyLsetFileReader(const DummyLsetFileReader & other)
  : AbstractFileReader(other)
{
}


mitk::DummyLsetFileReader* mitk::DummyLsetFileReader::Clone() const
{
  return new DummyLsetFileReader(*this);
}


mitk::DummyLsetFileReader::~DummyLsetFileReader()
{}


mitk::DummyLsetFileReader::DummyLsetFileReader()
{
  CustomMimeType mimeType(this->GetMimeTypePrefix() + "lset");
  mimeType.AddExtension("lset");
  mimeType.SetCategory("Images");
  mimeType.SetComment("Experimental MBI LabelSetImage");
  this->SetMimeType(mimeType);
  this->SetDescription("MBI LabelSetImage");
  this->RegisterService();
}

