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

#include "mitkNrrdQBallImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkImageFileWriter.h"
#include "mitkImageCast.h"
#include "mitkIOMimeTypes.h"
#include "mitkDiffusionIOMimeTypes.h"


mitk::NrrdQBallImageWriter::NrrdQBallImageWriter()
  : AbstractFileWriter(mitk::QBallImage::GetStaticNameOfClass(), CustomMimeType( mitk::DiffusionIOMimeTypes::QBI_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::QBI_MIMETYPE_DESCRIPTION())
{
  RegisterService();
}

mitk::NrrdQBallImageWriter::NrrdQBallImageWriter(const mitk::NrrdQBallImageWriter& other)
  : AbstractFileWriter(other)
{
}

mitk::NrrdQBallImageWriter::~NrrdQBallImageWriter()
{}


void mitk::NrrdQBallImageWriter::Write()
{
  InputType::ConstPointer input = dynamic_cast<const InputType*>(this->GetInput());
    if (input.IsNull())
    {
        MITK_ERROR <<"Sorry, input to NrrdQBallImageWriter is NULL!";
        return;
    }
    if ( this->GetOutputLocation().empty() )
    {
        MITK_ERROR << "Sorry, filename has not been set!";
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

    typedef itk::VectorImage<float, 3> VecImgType;

    typedef itk::Image<itk::Vector<float,QBALL_ODFSIZE>,3> ImageType;
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
    vecImg->SetVectorLength(QBALL_ODFSIZE);
    vecImg->Allocate();

    itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
    ot.GoToBegin();

    itk::ImageRegionIterator<ImageType> it (outimage, outimage->GetLargestPossibleRegion() );

    typedef ImageType::PixelType VecPixType;
    typedef VecImgType::PixelType VarVecType;

    for (it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      VarVecType varVec(vec.GetVnlVector().data_block(), QBALL_ODFSIZE);
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

mitk::NrrdQBallImageWriter* mitk::NrrdQBallImageWriter::Clone() const
{
  return new NrrdQBallImageWriter(*this);
}

mitk::IFileWriter::ConfidenceLevel mitk::NrrdQBallImageWriter::GetConfidenceLevel() const
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
