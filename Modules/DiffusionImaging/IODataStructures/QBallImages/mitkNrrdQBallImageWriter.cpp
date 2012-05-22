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


mitk::NrrdQBallImageWriter::NrrdQBallImageWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}


mitk::NrrdQBallImageWriter::~NrrdQBallImageWriter()
{}


void mitk::NrrdQBallImageWriter::GenerateData()
{
    m_Success = false;
    InputType* input = this->GetInput();
    if (input == NULL)
    {
        itkWarningMacro(<<"Sorry, input to NrrdQBallImageWriter is NULL!");
        return;
    }
    if ( m_FileName == "" )
    {
        itkWarningMacro( << "Sorry, filename has not been set!" );
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
    ot = ot.Begin();

    itk::ImageRegionIterator<ImageType> it (outimage, outimage->GetLargestPossibleRegion() );

    typedef ImageType::PixelType VecPixType;
    typedef VecImgType::PixelType VarVecType;

    for (it = it.Begin(); !it.IsAtEnd(); ++it)
    {
      VecPixType vec = it.Get();
      VarVecType varVec(vec.GetVnlVector().data_block(), QBALL_ODFSIZE);
      ot.Set(varVec);
      ++ot;
    }

    nrrdWriter->SetInput( vecImg );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName(m_FileName);
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
    m_Success = true;
}


void mitk::NrrdQBallImageWriter::SetInput( InputType* diffVolumes )
{
    this->ProcessObject::SetNthInput( 0, diffVolumes );
}


mitk::QBallImage* mitk::NrrdQBallImageWriter::GetInput()
{
    if ( this->GetNumberOfInputs() < 1 )
    {
        return NULL;
    }
    else
    {
        return dynamic_cast<InputType*> ( this->ProcessObject::GetInput( 0 ) );
    }
}


std::vector<std::string> mitk::NrrdQBallImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".qbi");
  possibleFileExtensions.push_back(".hqbi");
  return possibleFileExtensions;
}
