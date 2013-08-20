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

#ifndef __mitkNrrdLabelSetImageReader_cpp
#define __mitkNrrdLabelSetImageReader_cpp

#include "mitkNrrdLabelSetImageReader.h"

#include "mitkLabelSetImage.h"
#include "mitkLookupTableProperty.h"
#include "mitkColormapProperty.h"
#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"

#include <iostream>
#include <fstream>

#include "itksys/SystemTools.hxx"

namespace mitk
{
   template <class TPixelType>
   NrrdLabelSetImageReader<TPixelType>::NrrdLabelSetImageReader()
        : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
   {
   }

   template <class TPixelType>
   NrrdLabelSetImageReader<TPixelType>::~NrrdLabelSetImageReader()
   {
   }

  template <class TPixelType>
  void NrrdLabelSetImageReader<TPixelType>::GenerateData()
  {
    if ( m_FileName == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
    }
    else
    {
      try
      {
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

        typename ImageType::Pointer image;

        std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
        ext = itksys::SystemTools::LowerCase(ext);
        if (ext == ".lset")
        {
          typedef itk::ImageFileReader<ImageType> FileReaderType;
          typename FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName(this->m_FileName);
          itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
          reader->SetImageIO(io);
          reader->Update();
          image = reader->GetOutput();

          if (image.IsNotNull())
          {
              mitk::LabelSetImage::Pointer output = static_cast<OutputType*>(this->GetOutput());
              output->InitializeByItk<ImageType>( image );
              output->SetVolume( reader->GetOutput()->GetBufferPointer() );

              itk::MetaDataDictionary imgMetaDictionary = image->GetMetaDataDictionary();
              std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
              std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
              std::string metaString;

              char keybuffer[256];

              int numberOfLabels = 0;
              std::string name, lastmodified;
              for (; itKey != imgMetaKeys.end(); itKey ++)
              {
                itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
                if (itKey->find("name") != std::string::npos)
                {
                    name = metaString;
                }

                itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
                if (itKey->find("last modified") != std::string::npos)
                {
                    lastmodified = metaString;
                }

                if (itKey->find("number of labels") != std::string::npos)
                {
                    numberOfLabels = atoi(metaString.c_str());
                }
              }

              output->SetName(name);
              output->SetLabelSetLastModified(lastmodified);

              // skip first label (exterior) since it is created by mitkLabelSetImage constructor
              for (int i=1; i<numberOfLabels; i++)
              {
                  itKey = imgMetaKeys.begin();

                  mitk::Color color;
                  std::string name;
                  float opacity, volume;
                  int locked, visible;
                  for (; itKey != imgMetaKeys.end(); itKey ++)
                  {
                    itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
                    sprintf( keybuffer, "label_%03d_name", i );
                    if (itKey->find(keybuffer) != std::string::npos)
                    {
                        char str [512];
                        sscanf(metaString.c_str(), "%[^\n]s", &str);
                        name = str;
                    }
                    sprintf( keybuffer, "label_%03d_props", i );
                    if (itKey->find(keybuffer) != std::string::npos)
                    {
                        float rgba[4];
                        sscanf(metaString.c_str(), "%f %f %f %f %d %d %f", &rgba[0], &rgba[1], &rgba[2], &rgba[3], &locked, &visible, &volume);
                        color.SetRed(rgba[0]);
                        color.SetGreen(rgba[1]);
                        color.SetBlue(rgba[2]);
                        opacity = rgba[3];
                    }
                  }

                  mitk::Label::Pointer label = mitk::Label::New();
                  label->SetName(name);
                  label->SetOpacity(opacity);
                  label->SetColor(color);
                  label->SetLocked(locked);
                  label->SetVisible(visible);
                  label->SetVolume(volume);

                  output->AddLabel(*label);
              }
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
      }
      catch(std::exception& e)
      {
        MITK_INFO << "Std::Exception while reading file!!";
        MITK_INFO << e.what();
        throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
      }
      catch(...)
      {
        MITK_INFO << "Exception while reading file!!";
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
      }
    }
  }

  template <class TPixelType>
  void NrrdLabelSetImageReader<TPixelType>::GenerateOutputInformation()
  {

  }


  template <class TPixelType>
      const char* NrrdLabelSetImageReader<TPixelType>
      ::GetFileName() const
  {
    return m_FileName.c_str();
  }

  template <class TPixelType>
      void NrrdLabelSetImageReader<TPixelType>
      ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }

  template <class TPixelType>
      const char* NrrdLabelSetImageReader<TPixelType>
      ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }

  template <class TPixelType>
      void NrrdLabelSetImageReader<TPixelType>
      ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }

  template <class TPixelType>
      const char* NrrdLabelSetImageReader<TPixelType>
      ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }

  template <class TPixelType>
      void NrrdLabelSetImageReader<TPixelType>
      ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }

  template <class TPixelType>
      bool NrrdLabelSetImageReader<TPixelType>
      ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".lset")
    {
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

      typedef itk::ImageFileReader<ImageType> FileReaderType;
      typename FileReaderType::Pointer reader = FileReaderType::New();
      reader->SetImageIO(io);
      reader->SetFileName(filename);

      try
      {
        reader->Update();
      }
      catch(itk::ExceptionObject e)
      {
        MITK_INFO << e.GetDescription();
        return false;
      }

      typename ImageType::Pointer image = reader->GetOutput();
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
    }

    return false;
  }

} //namespace MITK

#endif
