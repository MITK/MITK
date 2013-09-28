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

#ifndef __mitkNrrdLabelSetImageWriter__cpp
#define __mitkNrrdLabelSetImageWriter__cpp

#include "mitkNrrdLabelSetImageWriter.h"
#include "mitkImageCast.h"
#include "itkMetaDataDictionary.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
//#include "itkNiftiImageIO.h"
#include "itkImageFileWriter.h"
#include "itksys/SystemTools.hxx"

#include <iostream>
#include <fstream>

template<typename TPixelType>
mitk::NrrdLabelSetImageWriter<TPixelType>::NrrdLabelSetImageWriter()
  : m_FileName(""), m_FilePrefix(""), m_FilePattern(""), m_Success(false)
{
  this->SetNumberOfRequiredInputs( 1 );
}

template<typename TPixelType>
mitk::NrrdLabelSetImageWriter<TPixelType>::~NrrdLabelSetImageWriter()
{}

template<typename TPixelType>
void mitk::NrrdLabelSetImageWriter<TPixelType>::GenerateData()
{
  m_Success = false;
  InputType* input = this->GetInput();
  if (input == NULL)
  {
    itkWarningMacro(<<"Sorry, input to NrrdLabelSetImageWriter is NULL!");
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

  typedef itk::Image<TPixelType,3> ImageType;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext == ".lset")
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileType( itk::ImageIOBase::Binary );
    io->UseCompressionOn();

    typedef itk::ImageFileWriter<ImageType> WriterType;
    typename WriterType::Pointer nrrdWriter = WriterType::New();
    nrrdWriter->UseInputMetaDataDictionaryOn();

    ImageType::Pointer itkImage = ImageType::New();
    mitk::CastToItkImage(input, itkImage);

    char keybuffer[512];
    char valbuffer[512];

    sprintf( valbuffer, "LSET");
    itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string("modality"),std::string(valbuffer));

    sprintf( valbuffer, input->GetLabelSetName().c_str() );
    itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string("name"),std::string(valbuffer));

    sprintf( valbuffer, input->GetLabelSetLastModified().c_str() );
    itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string("last modified"),std::string(valbuffer));

    sprintf( valbuffer, "%1d", input->GetNumberOfLabels());
    itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string("number of labels"),std::string(valbuffer));

    for(int i=0; i<input->GetNumberOfLabels(); i++)
    {
        sprintf( keybuffer, "label_%03d_name", i );
        sprintf( valbuffer, "%s", input->GetLabelName(i).c_str());

        itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));

        sprintf( keybuffer, "label_%03d_props", i );
        float rgba[4];
        rgba[0] = input->GetLabelColor(i).GetRed();
        rgba[1] = input->GetLabelColor(i).GetGreen();
        rgba[2] = input->GetLabelColor(i).GetBlue();
        rgba[3] = input->GetLabelOpacity(i);
        int locked = input->GetLabelLocked(i);
        int visible = input->GetLabelVisible(i);
        float volume = input->GetLabelVolume(i);
        unsigned int component = input->GetLabelLayer(i);
        sprintf( valbuffer, "%f %f %f %f %d %d %f %d", rgba[0], rgba[1], rgba[2], rgba[3], locked, visible, volume, component);

        itk::EncapsulateMetaData<std::string>(itkImage->GetMetaDataDictionary(),std::string(keybuffer), std::string(valbuffer));
    }

    nrrdWriter->SetInput( itkImage );
    nrrdWriter->SetImageIO(io);
    nrrdWriter->SetFileName(m_FileName);
    nrrdWriter->SetImageIO(io);
    try
    {
      nrrdWriter->Update();
    }
    catch (itk::ExceptionObject e)
    {
      MITK_ERROR << "Could not write segmentation. See error log for details.\n " << e.GetDescription();
      throw;
    }
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

template<typename TPixelType>
void mitk::NrrdLabelSetImageWriter<TPixelType>::SetInput( InputType* diffVolumes )
{
  this->ProcessObject::SetNthInput( 0, diffVolumes );
}

template<typename TPixelType>
//mitk::LabelSetImage<TPixelType>* mitk::NrrdLabelSetImageWriter<TPixelType>::GetInput()
mitk::LabelSetImage* mitk::NrrdLabelSetImageWriter<TPixelType>::GetInput()
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

template<typename TPixelType>
std::vector<std::string> mitk::NrrdLabelSetImageWriter<TPixelType>::GetPossibleFileExtensions()
{
  std::vector<std::string> possibleFileExtensions;
  possibleFileExtensions.push_back(".lset");
  return possibleFileExtensions;
}

#endif //__mitkNrrdLabelSetImageWriter__cpp
