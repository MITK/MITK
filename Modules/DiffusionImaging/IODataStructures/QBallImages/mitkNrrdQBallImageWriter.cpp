/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-12-10 18:05:13 +0100 (Mi, 10 Dez 2008) $
Version:   $Revision: 15922 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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
    
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType( itk::ImageIOBase::Binary );

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

    try
    {
      nrrdWriter->Update();
    }
    catch (itk::ExceptionObject e)
    {
      std::cout << e << std::endl;
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
