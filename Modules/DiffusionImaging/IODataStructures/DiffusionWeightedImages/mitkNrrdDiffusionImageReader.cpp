/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Module:    $RCSfile: itkImageSeriesReader.txx,v $
Language:  C++
Date:      $Date: 2007-08-17 20:16:25 $
Version:   $Revision: 1.31 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __mitkNrrdDiffusionImageReader_cpp
#define __mitkNrrdDiffusionImageReader_cpp

#include "mitkNrrdDiffusionImageReader.h"

#include "itkImageFileReader.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"

namespace mitk
{

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>
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
      itkWarningMacro("Tree cache is empty!");    
    }

    static_cast<OutputType*>(this->GetOutput())
      ->SetVectorImage(m_OutputCache->GetVectorImage());
    static_cast<OutputType*>(this->GetOutput())
      ->SetB_Value(m_OutputCache->GetB_Value());
    static_cast<OutputType*>(this->GetOutput())
      ->SetDirections(m_OutputCache->GetDirections());
    static_cast<OutputType*>(this->GetOutput())
      ->InitializeFromVectorImage();
  }

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>::GenerateOutputInformation()
  {
    typename OutputType::Pointer outputForCache = OutputType::New();
    if ( m_FileName == "") 
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename of the vessel tree to be read is empty!");
    }
    else
    {
      try
      {
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        typedef itk::ImageFileReader<ImageType> FileReaderType;
        typename FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetImageIO(io);
        reader->SetFileName(this->m_FileName);
        reader->Update();

        typename ImageType::Pointer img = reader->GetOutput();

        itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();    
        std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
        std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
        std::string metaString;

        GradientDirectionType vect3d;
        m_DiffusionVectors = GradientDirectionContainerType::New();

        int numberOfImages = 0;
        int numberOfGradientImages = 0;
        bool readb0 = false;

        for (; itKey != imgMetaKeys.end(); itKey ++)
        {
          double x,y,z;

          itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
          if (itKey->find("DWMRI_gradient") != std::string::npos)
          { 
            std::cout << *itKey << " ---> " << metaString << std::endl;      
            sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
            vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
            m_DiffusionVectors->InsertElement( numberOfImages, vect3d );
            ++numberOfImages;
            // If the direction is 0.0, this is a reference image
            if (vect3d[0] == 0.0 &&
              vect3d[1] == 0.0 &&
              vect3d[2] == 0.0)
            {
              continue;
            }
            ++numberOfGradientImages;;
          }
          else if (itKey->find("DWMRI_b-value") != std::string::npos)
          {
            std::cout << *itKey << " ---> " << metaString << std::endl;      
            readb0 = true;
            m_B_Value = atof(metaString.c_str());
          }
        }

        std::cout << "Number of gradient images: "
          << numberOfGradientImages
          << " and Number of reference images: "
          << numberOfImages - numberOfGradientImages
          << std::endl;

        if(!readb0)
        {
          std::cerr << "BValue not specified in header file" << std::endl;
        }

        // This call updates the output information of the associated VesselTreeData
        outputForCache->SetVectorImage(img);
        outputForCache->SetB_Value(m_B_Value);
        outputForCache->SetDirections(m_DiffusionVectors);

        // Since we have already read the tree, we can store it in a cache variable
        // so that it can be assigned to the DataObject in GenerateData();
        m_OutputCache = outputForCache;
        m_CacheTime.Modified();
      }
      catch(std::exception& e)
      {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());                    
      }
      catch(...)
      {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
      }
    }
  }


  template <class TPixelType>
  const char* NrrdDiffusionImageReader<TPixelType>
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }

  template <class TPixelType>
  const char* NrrdDiffusionImageReader<TPixelType>
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }

  template <class TPixelType>
  const char* NrrdDiffusionImageReader<TPixelType>
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }

  template <class TPixelType>
  bool NrrdDiffusionImageReader<TPixelType>
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/) 
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".hdwi" || ext == ".dwi")
    {
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();

      typedef itk::ImageFileReader<ImageType> FileReaderType;
      typename FileReaderType::Pointer reader = FileReaderType::New();
      reader->SetImageIO(io);
      reader->SetFileName(filename);
      reader->Update();
      typename ImageType::Pointer img = reader->GetOutput();
      itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();    
      std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
      std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
      std::string metaString;

      for (; itKey != imgMetaKeys.end(); itKey ++)
      {
        itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
        if (itKey->find("modality") != std::string::npos)
        {
          if (metaString.find("DWMRI") != std::string::npos) 
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
