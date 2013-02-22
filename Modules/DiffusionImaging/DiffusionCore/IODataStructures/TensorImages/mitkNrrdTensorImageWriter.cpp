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


mitk::NrrdTensorImageWriter::NrrdTensorImageWriter()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
    this->SetNumberOfRequiredInputs( 1 );
}


mitk::NrrdTensorImageWriter::~NrrdTensorImageWriter()
{}


void mitk::NrrdTensorImageWriter::GenerateData()
{
    m_Success = false;
    InputType* input = this->GetInput();
    if (input == NULL)
    {
        itkWarningMacro(<<"Sorry, input to NrrdTensorImageWriter is NULL!");
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

    typedef itk::Image<itk::DiffusionTensor3D<float>,3> ImageType;
    typedef itk::ImageFileWriter<ImageType> WriterType;
    WriterType::Pointer nrrdWriter = WriterType::New();

    ImageType::Pointer outimage = ImageType::New();
    CastToItkImage(input, outimage);

    nrrdWriter->SetInput( outimage );
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


void mitk::NrrdTensorImageWriter::SetInput( InputType* diffVolumes )
{
    this->ProcessObject::SetNthInput( 0, diffVolumes );
}


mitk::TensorImage* mitk::NrrdTensorImageWriter::GetInput()
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


std::vector<std::string> mitk::NrrdTensorImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".dti");
  possibleFileExtensions.push_back(".hdti");
  return possibleFileExtensions;
}
