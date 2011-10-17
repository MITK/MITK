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

#ifndef __mitkNrrdTbssGradientImageWriter__cpp
#define __mitkNrrdTbssGradientImageWriter__cpp

#include "mitkNrrdTbssGradientImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"
#include "boost/lexical_cast.hpp"

#include <iostream>
#include <fstream>


mitk::NrrdTbssGradientImageWriter::NrrdTbssGradientImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::NrrdTbssGradientImageWriter::~NrrdTbssGradientImageWriter()
{}


void mitk::NrrdTbssGradientImageWriter::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdTbssGradientImageWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }

  typedef itk::VectorImage<int,3> ImageType;

  ImageType::Pointer img = input->GetImage();

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();


  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();
 // nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput( img );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName(m_FileName);
  nrrdWriter->UseCompressionOn();
  nrrdWriter->SetImageIO(io);
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






void mitk::NrrdTbssGradientImageWriter::SetInput( InputType* tbssVol )
{
  this->ProcessObject::SetNthInput( 0, tbssVol );
}


mitk::TbssGradientImage* mitk::NrrdTbssGradientImageWriter::GetInput()
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


std::vector<std::string> mitk::NrrdTbssGradientImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".tgi"); // tgi = tbss gradient image
  return possibleFileExtensions;
}

#endif //__mitkNrrdTbssImageWriter__cpp
