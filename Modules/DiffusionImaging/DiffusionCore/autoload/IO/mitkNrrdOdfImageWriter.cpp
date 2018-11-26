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

#include "mitkNrrdOdfImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "mitkImageCast.h"
#include "mitkIOMimeTypes.h"
#include "mitkDiffusionCoreIOMimeTypes.h"
#include <mitkLocaleSwitch.h>


mitk::NrrdOdfImageWriter::NrrdOdfImageWriter()
  : AbstractFileWriter(mitk::OdfImage::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionCoreIOMimeTypes::ODF_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::ODF_MIMETYPE_DESCRIPTION())
{
  RegisterService();
}

mitk::NrrdOdfImageWriter::NrrdOdfImageWriter(const mitk::NrrdOdfImageWriter& other)
  : AbstractFileWriter(other)
{
}

mitk::NrrdOdfImageWriter::~NrrdOdfImageWriter()
{}


void mitk::NrrdOdfImageWriter::Write()
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
    if (input.IsNull())
    {
        MITK_ERROR <<"Sorry, input to NrrdOdfImageWriter is nullptr!";
        return;
    }
    if ( this->GetOutputLocation().empty() )
    {
        MITK_ERROR << "Sorry, filename has not been set!";
        return ;
    }

    mitk::LocaleSwitch localeSwitch("C");

    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();

    typedef itk::VectorImage<float, 3> VecImgType;

    typedef itk::Image<itk::Vector<float,ODF_SAMPLING_SIZE>,3> ImageType;
    typedef itk::ImageFileWriter<VecImgType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();

    ImageType::Pointer outimage = ImageType::New();
    CastToItkImage(input, outimage);

    VecImgType::Pointer vecImg = VecImgType::New();
    vecImg->SetSpacing( outimage->GetSpacing() );   // Set the image spacing
    vecImg->SetOrigin( outimage->GetOrigin() );     // Set the image origin
    vecImg->SetDirection( outimage->GetDirection() );  // Set the image direction
    vecImg->SetLargestPossibleRegion( outimage->GetLargestPossibleRegion());
    vecImg->SetBufferedRegion( outimage->GetLargestPossibleRegion() );
    vecImg->SetVectorLength(ODF_SAMPLING_SIZE);
    vecImg->Allocate();

    itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
    ot.GoToBegin();

    itk::ImageRegionIterator<ImageType> it (outimage, outimage->GetLargestPossibleRegion() );

    typedef ImageType::PixelType VecPixType;
    typedef VecImgType::PixelType VarVecType;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      VarVecType varVec(vec.GetVnlVector().data_block(), ODF_SAMPLING_SIZE);
      ot.Set(varVec);
      ++ot;
    }

    nrrdWriter->SetInput( vecImg );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName(this->GetOutputLocation().c_str());
    nrrdWriter->UseCompressionOn();


    try
    {
      nrrdWriter->Update();
    }
    catch (const itk::ExceptionObject& e)
    {
      MITK_INFO << e.what();
    }
}

mitk::NrrdOdfImageWriter* mitk::NrrdOdfImageWriter::Clone() const
{
  return new NrrdOdfImageWriter(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::NrrdOdfImageWriter::GetConfidenceLevel() const
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
