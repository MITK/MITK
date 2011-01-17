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
      ->SetOriginalDirections(m_OutputCache->GetOriginalDirections());
    static_cast<OutputType*>(this->GetOutput())
      ->SetMeasurementFrame(m_OutputCache->GetMeasurementFrame());
    static_cast<OutputType*>(this->GetOutput())
      ->InitializeFromVectorImage();
  }

  template <class TPixelType>
  void NrrdDiffusionImageReader<TPixelType>::GenerateOutputInformation()
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
        m_OriginalDiffusionVectors = GradientDirectionContainerType::New();

        int numberOfImages = 0;
        int numberOfGradientImages = 0;
        bool readb0 = false;
        bool readFrame = false;
        double xx, xy, xz, yx, yy, yz, zx, zy, zz;

        for (; itKey != imgMetaKeys.end(); itKey ++)
        {
          double x,y,z;

          itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
          if (itKey->find("DWMRI_gradient") != std::string::npos)
          { 
            MITK_INFO << *itKey << " ---> " << metaString;
            sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
            vect3d[0] = x; vect3d[1] = y; vect3d[2] = z;
            m_DiffusionVectors->InsertElement( numberOfImages, vect3d );
            m_OriginalDiffusionVectors->InsertElement( numberOfImages, vect3d );
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
            MITK_INFO << *itKey << " ---> " << metaString;
            readb0 = true;
            m_B_Value = atof(metaString.c_str());
          }
          else if (itKey->find("measurement frame") != std::string::npos)
          {
            MITK_INFO << *itKey << " ---> " << metaString;
            sscanf(metaString.c_str(), " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);

            readFrame = true;
            if (xx>10e-10 || xy>10e-10 || xz>10e-10 ||
                yx>10e-10 || yy>10e-10 || yz>10e-10 ||
                zx>10e-10 || zy>10e-10 || zz>10e-10 )
            {
              m_MeasurementFrame(0,0) = xx;
              m_MeasurementFrame(0,1) = xy;
              m_MeasurementFrame(0,2) = xz;
              m_MeasurementFrame(1,0) = yx;
              m_MeasurementFrame(1,1) = yy;
              m_MeasurementFrame(1,2) = yz;
              m_MeasurementFrame(2,0) = zx;
              m_MeasurementFrame(2,1) = zy;
              m_MeasurementFrame(2,2) = zz;
            }
            else
            {
              m_MeasurementFrame(0,0) = 1;
              m_MeasurementFrame(0,1) = 0;
              m_MeasurementFrame(0,2) = 0;
              m_MeasurementFrame(1,0) = 0;
              m_MeasurementFrame(1,1) = 1;
              m_MeasurementFrame(1,2) = 0;
              m_MeasurementFrame(2,0) = 0;
              m_MeasurementFrame(2,1) = 0;
              m_MeasurementFrame(2,2) = 1;
            }
          }
        }

        if(readFrame)
        {
          MITK_INFO << "Applying Measurement Frame: (" << xx << "," << xy << "," << xz
              << ") (" << yx << "," << yy << "," << yz << ") (" << zx << "," << zy << "," << zz  << ")";

          for(int i=0; i<numberOfImages; i++)
          {
            vnl_vector<double> vec(3);
            vec.copy_in(m_DiffusionVectors->ElementAt(i).data_block());
            vec = vec.pre_multiply(m_MeasurementFrame);
            m_DiffusionVectors->ElementAt(i).copy_in(vec.data_block());
          }
        }

        MITK_INFO << "Number of gradient images: "
          << numberOfGradientImages
          << " and Number of reference images: "
          << numberOfImages - numberOfGradientImages;

        if(!readb0)
        {
          MITK_INFO << "BValue not specified in header file";
        }

        // This call updates the output information of the associated VesselTreeData
        outputForCache->SetVectorImage(img);
        outputForCache->SetB_Value(m_B_Value);
        outputForCache->SetDirections(m_DiffusionVectors);
        outputForCache->SetOriginalDirections(m_OriginalDiffusionVectors);
        outputForCache->SetMeasurementFrame(m_MeasurementFrame);

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
