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

#include "mitkNrrdTensorImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "itkDiffusionTensor3D.h"
#include "mitkImageCast.h"
#include "mitkDiffusionIOMimeTypes.h"


mitk::NrrdTensorImageWriter::NrrdTensorImageWriter()
  : AbstractFileWriter(mitk::TensorImage::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::DTI_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::DTI_MIMETYPE_DESCRIPTION() )
{
  RegisterService();
}

mitk::NrrdTensorImageWriter::NrrdTensorImageWriter(const mitk::NrrdTensorImageWriter& other)
  : AbstractFileWriter(other)
{
}


mitk::NrrdTensorImageWriter::~NrrdTensorImageWriter()
{}


void mitk::NrrdTensorImageWriter::Write()
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull() )
  {
    MITK_ERROR <<"Sorry, input to NrrdTensorImageWriter is NULL!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!" ;
    return ;
  }
  const std::string& locale = "C";
  const std::string& currLocale = setlocale( LC_ALL, NULL );
  if ( locale.compare(currLocale)!=0 )
  {
    try
    {
      setlocale(LC_ALL, locale.c_str());
    }
    catch(...)
    {
      MITK_INFO << "Could not set locale " << locale;
    }
  }

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();

  typedef itk::Image<itk::DiffusionTensor3D<float>,3> ImageType;
  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();

  ImageType::Pointer outimage = ImageType::New();
  CastToItkImage(input, outimage);

  nrrdWriter->SetInput( outimage );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName(this->GetOutputLocation().c_str());
  nrrdWriter->UseCompressionOn();

  try
  {
    nrrdWriter->Update();
  }
  catch (itk::ExceptionObject e)
  {
    std::cout << e << std::endl;
  }

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    MITK_INFO << "Could not reset locale " << currLocale;
  }

}

mitk::NrrdTensorImageWriter* mitk::NrrdTensorImageWriter::Clone() const
{
  return new NrrdTensorImageWriter(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::NrrdTensorImageWriter::GetConfidenceLevel() const
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull() )
  {
    return Unsupported;
  }
  else
  {
    return Supported;
  }
}
