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

#ifndef __mitkNrrdTbssImageReader_cpp
#define __mitkNrrdTbssImageReader_cpp

#include "mitkNrrdTbssImageReader.h"

#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"

#include <itkImageFileWriter.h>

#include <iostream>
#include <fstream>

#include "itksys/SystemTools.hxx"


namespace mitk
{
      void NrrdTbssImageReader
      ::GenerateData()
  {

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
      itkWarningMacro("Tree cache is empty!")
    }


    static_cast<OutputType*>(this->GetPrimaryOutput())
        ->SetImage(m_OutputCache->GetImage());
    static_cast<OutputType*>(this->GetPrimaryOutput())
        ->SetGroupInfo(m_OutputCache->GetGroupInfo());

    static_cast<OutputType*>(this->GetPrimaryOutput())
        ->InitializeFromVectorImage();

  }

      void NrrdTbssImageReader
      ::GenerateOutputInformation()
  {
    OutputType::Pointer outputForCache = OutputType::New();

    if ( m_FileName == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename to be read is empty!");
    }
    else
    {
      try
      {
        const std::string& locale = "C";
        const std::string& currLocale = setlocale( LC_ALL, nullptr );

        if ( locale.compare(currLocale)!=0 )
        {
          try
          {
            MITK_INFO << " ** Changing locale from " << setlocale(LC_ALL, nullptr) << " to '" << locale << "'";
            setlocale(LC_ALL, locale.c_str());
          }
          catch(...)
          {
            MITK_INFO << "Could not set locale " << locale;
          }
        }


        MITK_INFO << "NrrdTbssImageReader READING IMAGE INFORMATION";
        ImageType::Pointer img;

        std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
        ext = itksys::SystemTools::LowerCase(ext);
        if (ext == ".tbss")
        {
          typedef itk::ImageFileReader<ImageType> FileReaderType;
          FileReaderType::Pointer reader = FileReaderType::New();
          reader->SetFileName(this->m_FileName);
          itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
          reader->SetImageIO(io);

          reader->Update();
          img = reader->GetOutput();


          MITK_INFO << "NrrdTbssImageReader READING HEADER INFORMATION";


          itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
          std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
          std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
          std::string metaString;



          std::string measurementInfo;


          std::vector< std::pair<std::string, int> > groups;

          for (; itKey != imgMetaKeys.end(); itKey ++)
          {
            itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);

            MITK_INFO << *itKey << " ---> " << metaString;

            if (itKey->find("Group_index") != std::string::npos)
            {

              std::vector<std::string> tokens;
              this->Tokenize(metaString, tokens, " ");


              std::pair< std::string, int > p;

              p.first="";
              for (unsigned int i=0; i<tokens.size()-1;i++)
              {
                p.first.append(" ");
                p.first.append(tokens.at(i));
              }

              std::cout << p.first << std::endl;

              p.second = atoi(tokens.at(tokens.size()-1 ).c_str());
              groups.push_back(p);



            }

            else if(itKey->find("Measurement info") != std::string::npos)
            {
              measurementInfo = metaString;
            }




          }


          outputForCache->SetGroupInfo(groups);
          outputForCache->SetMeasurementInfo(measurementInfo);


        }



        // This call updates the output information of the associated VesselTreeData
        outputForCache->SetImage(img);


        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();

        try
        {
          MITK_INFO << " ** Changing locale back from " << setlocale(LC_ALL, nullptr) << " to '" << currLocale << "'";
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



      const char* NrrdTbssImageReader
      ::GetFileName() const
  {
    return m_FileName.c_str();
  }

       void NrrdTbssImageReader
      ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }

      const char* NrrdTbssImageReader
      ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }

      void NrrdTbssImageReader
      ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }

      const char* NrrdTbssImageReader
      ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }

      void NrrdTbssImageReader
      ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }

      bool NrrdTbssImageReader
      ::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
  {

    // First check the extension
    if(  filename == "" )
      return false;

    // check if image is serie
    if( filePattern != "" && filePrefix != "" )
      return false;


    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".tbss")
    {
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

      typedef itk::ImageFileReader<ImageType> FileReaderType;
      FileReaderType::Pointer reader = FileReaderType::New();
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


      return true;
    }
    return false;
  }

} //namespace MITK

#endif
