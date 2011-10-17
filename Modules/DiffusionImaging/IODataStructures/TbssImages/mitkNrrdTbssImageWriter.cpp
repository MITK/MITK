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

  if(!input->GetIsMeta())
  {
    key = "meta";
    val = "false";
    itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),key,val);


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
  }
  /* If we are dealing with metadata we need to know what index of the vector corresponds to what image.
     Metadata are the images involved in the algorithms of tbss, like the mean FA Skeleton. */
  else
  {
    key = "meta";
    val = "true";
    itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),key,val);

    std::vector< std::pair<mitk::TbssImage::MetaDataFunction, int> > metaInfo = input->GetMetaInfo();
    std::vector< std::pair<mitk::TbssImage::MetaDataFunction, int> >::iterator it = metaInfo.begin();
    while(it != metaInfo.end())
    {
      std::pair<mitk::TbssImage::MetaDataFunction, int> p = *it;
      key = RetrieveString(p.first);
      val = " " + boost::lexical_cast<std::string>(p.second);
      itk::EncapsulateMetaData< std::string >(input->GetImage()->GetMetaDataDictionary(),key,val);
      it++;
    }
  }

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



std::string mitk::NrrdTbssImageWriter::RetrieveString(mitk::TbssImage::MetaDataFunction meta)
{
  if(meta == mitk::TbssImage::MEAN_FA_SKELETON)
  {
    return "mean fa skeleton";
  }
  else if(meta == mitk::TbssImage::MEAN_FA_SKELETON_MASK)
  {
    return "mean fa skeleton mask";
  }
  else if(meta == mitk::TbssImage::GRADIENT_X)
  {
    return "gradient_x";
  }
  else if(meta == mitk::TbssImage::GRADIENT_Y)
  {
    return "gradient_y";
  }
  else if(meta == mitk::TbssImage::GRADIENT_Z)
  {
    return "gradient_z";
  }
  else if(meta == mitk::TbssImage::TUBULAR_STRUCTURE)
  {
    return "tubular structure";
  }
  else if(meta == mitk::TbssImage::DISTANCE_MAP)
  {
    return "distance map";
  }
  return "";
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
