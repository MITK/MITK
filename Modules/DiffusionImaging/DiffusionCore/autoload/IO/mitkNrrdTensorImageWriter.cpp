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
#include "mitkDiffusionCoreIOMimeTypes.h"
#include <mitkLocaleSwitch.h>


mitk::NrrdTensorImageWriter::NrrdTensorImageWriter()
  : AbstractFileWriter(mitk::TensorImage::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionCoreIOMimeTypes::DTI_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::DTI_MIMETYPE_DESCRIPTION() )
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
    MITK_ERROR <<"Sorry, input to NrrdTensorImageWriter is nullptr!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!" ;
    return ;
  }

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();

  typedef itk::ImageFileWriter<TensorImage::ItkTensorImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();

  TensorImage::ItkTensorImageType::Pointer outimage = TensorImage::ItkTensorImageType::New();
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
