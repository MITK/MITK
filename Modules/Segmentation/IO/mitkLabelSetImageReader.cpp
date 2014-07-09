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
#include "mitkPropertyListsXmlFileReaderAndWriter.h"

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
  std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
  std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
  std::string metaString;

  char keybuffer[256];

  int numberOfLabels(0);
  std::string name, lastmodified;
  for (; itKey != imgMetaKeys.end(); itKey ++)
  {
    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
    if (itKey->find("name") != std::string::npos)
    {
      name = metaString;
    }

    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
    if (itKey->find("last modification time") != std::string::npos)
    {
      lastmodified = metaString;
    }

    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
    if (itKey->find("number of labels") != std::string::npos)
    {
      numberOfLabels = atoi(metaString.c_str());
    }
  }

  output->SetName(name);
  output->SetLastModificationTime(lastmodified);
  output->AddLayer();

  char str[32786];
  std::string _xmlStr;
  mitk::Label::Pointer label;
  TiXmlDocument doc;

  for (int idx=0; idx<numberOfLabels; idx++)
  {
    itKey = imgMetaKeys.begin();
    for (; itKey != imgMetaKeys.end(); itKey ++)
    {
      itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
      sprintf( keybuffer, "label_%03d", idx );
      if (itKey->find(keybuffer) != std::string::npos)
      {
        sscanf(metaString.c_str(), "%[^\n]s", &str);
        _xmlStr = str;
        doc.Parse(_xmlStr.c_str());

        MITK_INFO << _xmlStr;
      }
    }

    label = mitk::Label::New();
    label->DeserializeLabel(&doc);

    if (label->GetValue() != 0)
      output->GetLabelSet()->AddLabel(*label);
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
