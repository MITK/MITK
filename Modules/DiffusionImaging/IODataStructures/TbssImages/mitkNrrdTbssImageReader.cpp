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

    int vecsize = m_OutputCache->GetImage()->GetVectorLength();

    static_cast<OutputType*>(this->GetOutput())
        ->SetImage(m_OutputCache->GetImage());
    static_cast<OutputType*>(this->GetOutput())
        ->SetGroupInfo(m_OutputCache->GetGroupInfo());
    static_cast<OutputType*>(this->GetOutput())
        ->SetMetaInfo(m_OutputCache->GetMetaInfo());
    static_cast<OutputType*>(this->GetOutput())
        ->SetIsMeta(m_OutputCache->GetIsMeta());
    static_cast<OutputType*>(this->GetOutput())
        ->SetContainsDistanceMap(m_OutputCache->GetContainsDistanceMap());
    static_cast<OutputType*>(this->GetOutput())
        ->SetContainsMeanSkeleton(m_OutputCache->GetContainsMeanSkeleton());
    static_cast<OutputType*>(this->GetOutput())
        ->SetContainsSkeletonMask(m_OutputCache->GetContainsSkeletonMask());
    static_cast<OutputType*>(this->GetOutput())
        ->SetContainsGradient(m_OutputCache->GetContainsGradient());
    static_cast<OutputType*>(this->GetOutput())
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


          //int numberOfGradientImages = 0;
          std::string measurementInfo;
          bool isMeta = false;
          bool containsSkeleton;
          bool containsSkeletonMask;
          bool containsGradient;
          bool containsDistanceMap;



          std::vector<std::pair<mitk::TbssImage::MetaDataFunction, int > > metaInfo;

          std::vector< std::pair<std::string, int> > groups;

          for (; itKey != imgMetaKeys.end(); itKey ++)
          {
            itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);

            MITK_INFO << *itKey << " ---> " << metaString;

            if (itKey->find("Group_index") != std::string::npos)
            {

              std::vector<std::string> tokens;
              this->Tokenize(metaString, tokens, " ");

              if(tokens.size()==2)
              {

                std::cout << tokens.at(0) << " " << tokens.at(1) << std::endl;

                std::pair< std::string, int > p;
                p.first = tokens.at(0);
                std::string s = tokens.at(1);
                p.second = atoi(tokens.at(1).c_str());
                groups.push_back(p);
              }


            }

            else if(itKey->find("Measurement info") != std::string::npos)
            {
              measurementInfo = metaString;
            }


            else if(itKey->find("meta") != std::string::npos)
            {
              if(metaString == "true")
              {
                isMeta = true;
              }
            }

            else if(itKey->find("mean fa skeleton mask") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::MEAN_FA_SKELETON_MASK;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsSkeletonMask = true;
            }

            else if(itKey->find("mean fa skeleton") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::MEAN_FA_SKELETON;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsSkeleton = true;
            }

            else if(itKey->find("gradient_x") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::GRADIENT_X;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsGradient = true;
            }
            else if(itKey->find("gradient_y") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::GRADIENT_Y;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsGradient = true;
            }

            else if(itKey->find("gradient_z") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::GRADIENT_Z;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsGradient = true;
            }


            else if(itKey->find("tubular structure") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::TUBULAR_STRUCTURE;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
            }

            else if(itKey->find("distance map") != std::string::npos)
            {
              std::pair<mitk::TbssImage::MetaDataFunction, int> p;
              p.first = mitk::TbssImage::DISTANCE_MAP;
              p.second = atoi(metaString.c_str());
              metaInfo.push_back(p);
              containsDistanceMap = true;
            }

          }

          outputForCache->SetIsMeta(isMeta);
          outputForCache->SetContainsGradient(containsGradient);
          outputForCache->SetContainsSkeletonMask(containsSkeletonMask);
          outputForCache->SetContainsMeanSkeleton(containsSkeleton);
          outputForCache->SetContainsDistanceMap(containsDistanceMap);
          outputForCache->SetGroupInfo(groups);
          outputForCache->SetMeasurementInfo(measurementInfo);
          outputForCache->SetMetaInfo(metaInfo);

        }



        // This call updates the output information of the associated VesselTreeData
        outputForCache->SetImage(img);

      //  outputForCache->SetB_Value(m_B_Value);
        //outputForCache->SetDirections(m_DiffusionVectors);
       // outputForCache->SetOriginalDirections(m_OriginalDiffusionVectors);
       // outputForCache->SetMeasurementFrame(m_MeasurementFrame);

        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();

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


/*
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
*/
 //  return false;

      return true;
    }
    return false;
  }

} //namespace MITK

#endif
