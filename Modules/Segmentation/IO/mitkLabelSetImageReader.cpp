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

#ifndef __mitkLabelSetImageReader_cpp
#define __mitkLabelSetImageReader_cpp

#include "mitkLabelSetImageReader.h"

// itk
#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

#include <iostream>
#include <fstream>

#include "itksys/SystemTools.hxx"

#include "tinyxml.h"

namespace mitk
{

LabelSetImageReader::LabelSetImageReader() :
m_FileName(""),
m_FilePrefix(""),
m_FilePattern("")
{
}

LabelSetImageReader::~LabelSetImageReader()
{
}

void LabelSetImageReader::GenerateData()
{
  if ( m_FileName == "")
    mitkThrow() << "The filename to be read is empty!";

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
      mitkThrow() << "Could not set locale.";
    }
  }

  LabelSetImage::VectorImageType::Pointer vectorImage;

  std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
  ext = itksys::SystemTools::LowerCase(ext);
  if (ext != ".lset")
  {
    MITK_WARN << "Selected file is not a segmentation session";
    return;
  }

  typedef itk::ImageFileReader<VectorImageType> FileReaderType;
  FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetFileName(this->m_FileName);
  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  reader->SetImageIO(io);
  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }
  catch(...)
  {
    mitkThrow() << "Unknown exception caught!";
  }

  vectorImage = reader->GetOutput();

  if (vectorImage.IsNull())
    mitkThrow() << "Could not retrieve the vector image.";

  LabelSetImage * output = static_cast<OutputType*>(this->GetOutput());

  ImageType::Pointer auxImg = ImageType::New();
  auxImg->SetSpacing( vectorImage->GetSpacing() );
  auxImg->SetOrigin( vectorImage->GetOrigin() );
  auxImg->SetDirection( vectorImage->GetDirection() );
  auxImg->SetRegions( vectorImage->GetLargestPossibleRegion() );
  auxImg->Allocate();

  // initialize output image based on vector image meta information
  output->InitializeByItk<ImageType>( auxImg.GetPointer() );

  itk::MetaDataDictionary imgMetaDictionary = vectorImage->GetMetaDataDictionary();

  char keybuffer[256];

  int numberOfLayers = GetIntByKey(imgMetaDictionary,"layers");
  std::string _xmlStr;
  mitk::Label::Pointer label;
  TiXmlDocument doc;


  for( int layerIdx=0; layerIdx < numberOfLayers; layerIdx++)
  {
    sprintf( keybuffer, "layer_%03d", layerIdx );
    int numberOfLabels = GetIntByKey(imgMetaDictionary,keybuffer);

    output->AddLayer();

    for(int labelIdx=0; labelIdx < numberOfLabels; labelIdx++)
    {
      sprintf( keybuffer, "label_%03d_%03d", layerIdx, labelIdx );
      _xmlStr = GetStringByKey(imgMetaDictionary,keybuffer);
      doc.Parse(_xmlStr.c_str());

      label = mitk::Label::New();
      label->LoadFromTiXmlDocument(&doc);

      if(label->GetValue() == 0) // set exterior label is needed to hold exterior information
        output->SetExteriorLabel(label);
      output->GetLabelSet()->AddLabel(label);
    }
  }


  // set vector image
  output->SetVectorImage(vectorImage);

  try
  {
    setlocale(LC_ALL, currLocale.c_str());
  }
  catch(...)
  {
    mitkThrow() << "Could not reset locale!";
  }
}

int LabelSetImageReader::GetIntByKey(const itk::MetaDataDictionary & dic,const std::string & str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    itk::ExposeMetaData<std::string> (dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return atoi(metaString.c_str());
    }
  }
  return 0;
}

std::string LabelSetImageReader::GetStringByKey(const itk::MetaDataDictionary & dic,const std::string & str)
{
  std::vector<std::string> imgMetaKeys = dic.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString("");
  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    itk::ExposeMetaData<std::string> (dic, *itKey, metaString);
    if (itKey->find(str.c_str()) != std::string::npos)
    {
      return metaString;
    }
  }
  return metaString;
}

void LabelSetImageReader::GenerateOutputInformation()
{
}

const char* LabelSetImageReader::GetFileName() const
{
  return m_FileName.c_str();
}

void LabelSetImageReader::SetFileName(const char* aFileName)
{
  m_FileName = aFileName;
}

const char* LabelSetImageReader::GetFilePrefix() const
{
  return m_FilePrefix.c_str();
}

void LabelSetImageReader::SetFilePrefix(const char* aFilePrefix)
{
  m_FilePrefix = aFilePrefix;
}

const char* LabelSetImageReader::GetFilePattern() const
{
  return m_FilePattern.c_str();
}

void LabelSetImageReader::SetFilePattern(const char* aFilePattern)
{
  m_FilePattern = aFilePattern;
}

bool LabelSetImageReader::CanReadFile(const std::string filename,
                                          const std::string /*filePrefix*/, const std::string /*filePattern*/)
{
  // First check the extension
  if(  filename == "" )
  {
    return false;
  }
  std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
  ext = itksys::SystemTools::LowerCase(ext);

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

  typedef itk::ImageFileReader<LabelSetImage::VectorImageType> FileReaderType;
  FileReaderType::Pointer reader = FileReaderType::New();
  reader->SetImageIO(io);
  reader->SetFileName(filename);

  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject& e)
  {
    mitkThrow() << e.GetDescription();
  }

  LabelSetImage::VectorImageType::Pointer image = reader->GetOutput();
  if (image.IsNotNull())
  {
    itk::MetaDataDictionary imgMetaDictionary = image->GetMetaDataDictionary();
    std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString;

    for (; itKey != imgMetaKeys.end(); itKey ++)
    {
      itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
      if (itKey->find("modality") != std::string::npos)
      {
        if (metaString.find("LSET") != std::string::npos)
        {
          return true;
        }
      }
    }
  }

  return false;
}

} //namespace MITK

#endif
