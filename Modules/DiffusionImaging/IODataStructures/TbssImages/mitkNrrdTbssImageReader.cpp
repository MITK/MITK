/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __mitkNrrdDiffusionImageReader_cpp
#define __mitkNrrdDiffusionImageReader_cpp

#include "mitkNrrdTbssImageReader.h"

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
      void NrrdTbssImageReader<TPixelType>
      ::GenerateData()
  {

    MITK_INFO << "Generate Data";

    // Since everything is completely read in GenerateOutputInformation() it is stored
    // in a cache variable. A timestamp is associated.
    // If the timestamp of the cache variable is newer than the MTime, we only need to
    // assign the cache variable to the DataObject.
    // Otherwise, the tree must be read again from the file and OuputInformation must 
    // be updated!
    if ( ( ! m_OutputCache ) || ( this->GetMTime( ) > m_CacheTime.GetMTime( ) ) )
    {
      this->GenerateOutputInformation();
      itkWarningMacro("Cache regenerated!");    
    }

    if (!m_OutputCache)
    {
      itkWarningMacro("Tree cache is empty!");    
    }

/*

    // Create some meaningful output here

    ImageType* img = static_cast<ImageType*> (m_OutputCache->GetImage());

    static_cast<OutputType*>(this->GetOutput())->SetImage(m_OutputCache->GetImage());




    static_cast<OutputType*>(this->GetOutput())->SetTbssType(m_OutputCache->GetTbssType());


    static_cast<OutputType*>(this->GetOutput())->InitializeFromImage();
*/
  }

  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>::GenerateOutputInformation()
  {
    typename OutputType::Pointer outputForCache = OutputType::New();
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
            MITK_INFO << " ** Changing locale from " << setlocale(LC_ALL, NULL) << " to '" << locale << "'";
            setlocale(LC_ALL, locale.c_str());
          }
          catch(...)
          {
            MITK_INFO << "Could not set locale " << locale;
          }
        }


        MITK_INFO << "NrrdTbssImageReader READING IMAGE INFORMATION";
        typename ImageType::Pointer img;

        std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
        ext = itksys::SystemTools::LowerCase(ext);
        if (ext == ".tbss")
        {
          typedef itk::ImageFileReader<ImageType> FileReaderType;
          typename FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName(this->m_FileName);
          itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
          reader->SetImageIO(io);
          reader->Update();
          img = reader->GetOutput();
        }



        MITK_INFO << "NrrdTbssImageReader READING HEADER INFORMATION";

        if (ext == ".tbss")
        {

          itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
          std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
          std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
          std::string metaString;

          for (; itKey != imgMetaKeys.end(); itKey ++)
          {


            itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
            if (itKey->find("tbss") != std::string::npos)
            {
              MITK_INFO << *itKey << " ---> " << metaString;
              outputForCache->SetTbssType(metaString);
            }            

          }

        }




        // Put image info in the outputForCache image
        outputForCache->SetImage(img);



        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();



        //this->SetOutput(m_OutputCache);

        try
        {
          MITK_INFO << " ** Changing locale back from " << setlocale(LC_ALL, NULL) << " to '" << currLocale << "'";
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
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the file!");
      }
    }
  }


  template <class TPixelType>
      const char* NrrdTbssImageReader<TPixelType>
      ::GetFileName() const
  {
    return m_FileName.c_str();
  }

  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>
      ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }

  template <class TPixelType>
      const char* NrrdTbssImageReader<TPixelType>
      ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }

  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>
      ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }

  template <class TPixelType>
      const char* NrrdTbssImageReader<TPixelType>
      ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }

  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>
      ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }

  template <class TPixelType>
      bool NrrdTbssImageReader<TPixelType>
      ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".tbss")
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
      }

      typename ImageType::Pointer img = reader->GetOutput();
      itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();    
      std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
      std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
      std::string metaString;

      for (; itKey != imgMetaKeys.end(); itKey ++)
      {
        itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
        if (itKey->find("tbss") != std::string::npos)
        {
          if (metaString.find("ROI") != std::string::npos)
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
