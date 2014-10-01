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

#ifndef __mitkNrrdTbssRoiImageWriter__cpp
#define __mitkNrrdTbssRoiImageWriter__cpp

#include "mitkNrrdTbssRoiImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>


mitk::NrrdTbssRoiImageWriter::NrrdTbssRoiImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}


mitk::NrrdTbssRoiImageWriter::~NrrdTbssRoiImageWriter()
{}

void mitk::NrrdTbssRoiImageWriter::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();

  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdTbssImageWriter is NULL!")
    return;
  }
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "Sorry, filename has not been set!" )
    return ;
  }


  char keybuffer[512];
  char valbuffer[512];

  std::vector< itk::Index<3> > roi = input->GetRoi();

  std::vector< itk::Index<3> >::iterator it = roi.begin();

  int i=0;
  while(it != roi.end())
  {
    itk::Index<3> ix = *it;

    sprintf( keybuffer, "ROI_index_%04d", i );
    sprintf( valbuffer, "%ld %ld %ld", ix[0],ix[1],ix[2]);

    std::cout << valbuffer << std::endl;

    //input->GetImage()->GetMetaDataDictionary();

    itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(), std::string(keybuffer), std::string(valbuffer));

    it++;
    ++i;
  }

  std::string structure = input->GetStructure();
  itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(), "structure", structure);

  typedef itk::Image<unsigned char,3> ImageType;
  ImageType::Pointer img = input->GetImage();

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


void mitk::NrrdTbssRoiImageWriter::SetInput( InputType* tbssVol )
{
  this->ProcessObject::SetNthInput( 0, tbssVol );
}


mitk::TbssRoiImage* mitk::NrrdTbssRoiImageWriter::GetInput()
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


std::vector<std::string> mitk::NrrdTbssRoiImageWriter::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".roi");
  return possibleFileExtensions;
}

std::string mitk::NrrdTbssRoiImageWriter::GetSupportedBaseData() const
{
  return TbssRoiImage::GetStaticNameOfClass();
}

#endif //__mitkNrrdTbssRoiImageWriter__cpp
