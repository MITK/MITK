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

#ifndef __mitkNrrdTbssImageWriter__cpp
#define __mitkNrrdTbssImageWriter__cpp

#include "mitkNrrdTbssImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>


mitk::NrrdTbssImageWriter::NrrdTbssImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

mitk::NrrdTbssImageWriter::~NrrdTbssImageWriter()
{}


void mitk::NrrdTbssImageWriter::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdTbssImageWriter is NULL!");
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" );
    return ;
  }



  itk::VectorImage<float, 3>::Pointer img = input->GetImage();

  typedef itk::VectorImage<float,3> ImageType;


  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  io->SetFileType( itk::ImageIOBase::Binary );
  io->UseCompressionOn();


  typedef itk::ImageFileWriter<ImageType> WriterType;
  WriterType::Pointer nrrdWriter = WriterType::New();
  nrrdWriter->UseInputMetaDataDictionaryOn();
  nrrdWriter->SetInput( img );
  nrrdWriter->SetImageIO(io);
  nrrdWriter->SetFileName(m_FileName);
 // nrrdWriter->UseCompressionOn();
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


void mitk::NrrdTbssImageWriter::SetInput( InputType* tbssVol )
{
  this->ProcessObject::SetNthInput( 0, tbssVol );
}


mitk::TbssImage* mitk::NrrdTbssImageWriter::GetInput()
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


std::vector<std::string> mitk::NrrdTbssImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".tbss");
  return possibleFileExtensions;
}

#endif //__mitkNrrdTbssImageWriter__cpp
