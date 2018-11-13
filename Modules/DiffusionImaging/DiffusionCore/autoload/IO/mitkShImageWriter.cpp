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

#include "mitkShImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkImageFileWriter.h"
#include "mitkImageCast.h"
#include "mitkIOMimeTypes.h"
#include "mitkDiffusionCoreIOMimeTypes.h"
#include <mitkLocaleSwitch.h>
#include <itkShCoefficientImageExporter.h>
#include <itksys/SystemTools.hxx>
#include <itkNrrdImageIO.h>

mitk::ShImageWriter::ShImageWriter()
  : AbstractFileWriter(mitk::ShImage::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionCoreIOMimeTypes::SH_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::SH_MIMETYPE_DESCRIPTION())
{
  RegisterService();
}

mitk::ShImageWriter::ShImageWriter(const mitk::ShImageWriter& other)
  : AbstractFileWriter(other)
{
}

mitk::ShImageWriter::~ShImageWriter()
{}

template <int shOrder>
void mitk::ShImageWriter::WriteShImage(InputType::ConstPointer input)
{
  mitk::LocaleSwitch localeSwitch("C");
  typename itk::ShCoefficientImageExporter< float, shOrder >::InputImageType::Pointer itk_image = itk::ShCoefficientImageExporter< float, shOrder >::InputImageType::New();
  CastToItkImage(input, itk_image);

  typedef itk::ShCoefficientImageExporter< float, shOrder > ExporterType;
  typename ExporterType::Pointer exporter = ExporterType::New();
  exporter->SetInputImage(itk_image);
  exporter->GenerateData();

  std::string ext = itksys::SystemTools::GetFilenameExtension(this->GetOutputLocation());

  typedef itk::ImageFileWriter<ShImage::ShOnDiskType> WriterType;
  WriterType::Pointer writer = WriterType::New();
  ShImage::ShOnDiskType::Pointer image = exporter->GetOutputImage();
  writer->SetInput( image );

  if (ext==".shi")
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();
    writer->SetImageIO(io);
  }

  writer->SetFileName(this->GetOutputLocation().c_str());
  writer->SetUseCompression(true);

  try
  {
    writer->Update();
  }
  catch (const itk::ExceptionObject& e)
  {
    MITK_INFO << e.what();
  }
}

void mitk::ShImageWriter::Write()
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
  if (input.IsNull())
  {
    MITK_ERROR <<"Sorry, input to ShImageWriter is nullptr!";
    return;
  }
  if ( this->GetOutputLocation().empty() )
  {
    MITK_ERROR << "Sorry, filename has not been set!";
    return ;
  }

  switch (input->GetImageDescriptor()->GetChannelTypeById(0).GetNumberOfComponents())
  {
  case 6:
    WriteShImage<2>(input);
    break;
  case 15:
    WriteShImage<4>(input);
    break;
  case 28:
    WriteShImage<6>(input);
    break;
  case 45:
    WriteShImage<8>(input);
    break;
  case 66:
    WriteShImage<10>(input);
    break;
  case 91:
    WriteShImage<12>(input);
    break;
  default :
    mitkThrow() << "SH order larger 12 not supported";
  }
}

mitk::ShImageWriter* mitk::ShImageWriter::Clone() const
{
  return new ShImageWriter(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::ShImageWriter::GetConfidenceLevel() const
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
