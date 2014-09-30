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

#ifndef __mitkNrrdTbssImageWriter__cpp
#define __mitkNrrdTbssImageWriter__cpp

#include "mitkNrrdTbssImageWriter.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"
#include "boost/lexical_cast.hpp"

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




  std::string key;
  std::string val;


  /* For the case of a tbss image containing data of the patients:
     Save info about the groups and the type of measurement */




  std::vector< std::pair <std::string, int> > groups = input->GetGroupInfo();
  std::vector< std::pair <std::string, int> >::iterator it = groups.begin();

  int i=0;
  while(it != groups.end())
  {
    std::pair<std::string, int> p = *it;

    key = "Group_index_" + boost::lexical_cast<std::string>(i);
    val = " " + p.first + " " + boost::lexical_cast<std::string>(p.second);
    //sprintf( keybuffer, "Group_index_%04d", std::string(i) );
    // sprintf( valbuffer, "%1d %1d", p.first, p.second);

    //std::cout << valbuffer << std::endl;

    //itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),std::string(keybuffer),std::string(valbuffer));
    itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),key,val);
    it++;
    ++i;
  }

  key = "Measurement info";
  val = input->GetMeasurementInfo();
  itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),key,val);





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

std::string mitk::NrrdTbssImageWriter::GetSupportedBaseData() const
{
  return TbssImage::GetStaticNameOfClass();
}

#endif //__mitkNrrdTbssImageWriter__cpp
