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

#include <itkImageFileWriter.h>

#include <iostream>
#include <fstream>

#include "itksys/SystemTools.hxx"

namespace mitk
{

  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>
      ::GenerateData()
  {

    try
    {

      // Change locale if needed
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



      // READ IMAGE INFORMATION
      const unsigned int MINDIM = 3;
      const unsigned int MAXDIM = 4;

      MITK_INFO << "loading " << m_FileName << " via mitk::NrrdTbssImageReader... " << std::endl;

      // Check to see if we can read the file given the name or prefix
      if ( m_FileName == "" )
      {
        itkWarningMacro( << "Filename is empty!" )
        return ;
      }

      itk::NrrdImageIO::Pointer imageIO = itk::NrrdImageIO::New();
      imageIO->SetFileName( m_FileName.c_str() );
      imageIO->ReadImageInformation();

      unsigned int ndim = imageIO->GetNumberOfDimensions();

      if ( ndim < MINDIM || ndim > MAXDIM )
      {
        itkWarningMacro( << "Sorry, only dimensions 3 is supported. The given file has " << ndim << " dimensions!" )
        return;
      }


      itk::ImageIORegion ioRegion( ndim );
      itk::ImageIORegion::SizeType ioSize = ioRegion.GetSize();
      itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();

      unsigned int dimensions[ MAXDIM ];
      dimensions[ 0 ] = 0;
      dimensions[ 1 ] = 0;
      dimensions[ 2 ] = 0;
      dimensions[ 3 ] = 0;

      float spacing[ MAXDIM ];
      spacing[ 0 ] = 1.0f;
      spacing[ 1 ] = 1.0f;
      spacing[ 2 ] = 1.0f;
      spacing[ 3 ] = 1.0f;


      Point3D origin;
      origin.Fill(0);

      unsigned int i;
      for ( i = 0; i < ndim ; ++i )
      {
        ioStart[ i ] = 0;
        ioSize[ i ] = imageIO->GetDimensions( i );
        if(i<MAXDIM)
        {
          dimensions[ i ] = imageIO->GetDimensions( i );
          spacing[ i ] = imageIO->GetSpacing( i );
          if(spacing[ i ] <= 0)
            spacing[ i ] = 1.0f;
        }
        if(i<3)
        {
          origin[ i ] = imageIO->GetOrigin( i );
        }
      }

      ioRegion.SetSize( ioSize );
      ioRegion.SetIndex( ioStart );

      MITK_INFO << "ioRegion: " << ioRegion << std::endl;
      imageIO->SetIORegion( ioRegion );
      void* buffer = new unsigned char[imageIO->GetImageSizeInBytes()];
      imageIO->Read( buffer );
      //mitk::Image::Pointer static_cast<OutputType*>(this->GetOutput())image = mitk::Image::New();
      if((ndim==4) && (dimensions[3]<=1))
        ndim = 3;
      if((ndim==3) && (dimensions[2]<=1))
        ndim = 2;
      mitk::PixelType pixelType( imageIO->GetComponentTypeInfo(), imageIO->GetNumberOfComponents(), imageIO->GetPixelType() );
      static_cast<OutputType*>(this->GetOutput())->Initialize( pixelType, ndim, dimensions );
      static_cast<OutputType*>(this->GetOutput())->SetImportChannel( buffer, 0, Image::ManageMemory );

      // access direction of itk::Image and include spacing
      mitk::Matrix3D matrix;
      matrix.SetIdentity();
      unsigned int j, itkDimMax3 = (ndim >= 3? 3 : ndim);
      for ( i=0; i < itkDimMax3; ++i)
        for( j=0; j < itkDimMax3; ++j )
          matrix[i][j] = imageIO->GetDirection(j)[i];

      // re-initialize PlaneGeometry with origin and direction
      PlaneGeometry* planeGeometry = static_cast<PlaneGeometry*>
                                     (static_cast<OutputType*>
                                      (this->GetOutput())->GetSlicedGeometry(0)->GetGeometry2D(0));
      planeGeometry->SetOrigin(origin);
      planeGeometry->GetIndexToWorldTransform()->SetMatrix(matrix);

      // re-initialize SlicedGeometry3D
      SlicedGeometry3D* slicedGeometry = static_cast<OutputType*>(this->GetOutput())->GetSlicedGeometry(0);
      slicedGeometry->InitializeEvenlySpaced(planeGeometry, static_cast<OutputType*>(this->GetOutput())->GetDimension(2));
      slicedGeometry->SetSpacing(spacing);

      // re-initialize TimeSlicedGeometry
      static_cast<OutputType*>(this->GetOutput())->GetTimeSlicedGeometry()->InitializeEvenlyTimed(slicedGeometry, static_cast<OutputType*>(this->GetOutput())->GetDimension(3));

      buffer = NULL;
      MITK_INFO << "number of image components: "<< static_cast<OutputType*>(this->GetOutput())->GetPixelType().GetNumberOfComponents() << std::endl;



      // READ TBSS HEADER INFORMATION
      typename ImageType::Pointer img;

      std::string ext = itksys::SystemTools::GetFilenameLastExtension(m_FileName);
      ext = itksys::SystemTools::LowerCase(ext);
      if (ext == ".tbss")
      {
        typedef itk::ImageFileReader<ImageType> FileReaderType;
        typename FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetFileName(this->m_FileName);

        reader->SetImageIO(imageIO);
        reader->Update();

        img = reader->GetOutput();

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

            if(metaString == "ROI")
            {
              MITK_INFO << "Read the ROI info";
              ReadRoiInfo(imgMetaDictionary); // move back into if statement
              static_cast<OutputType*>(this->GetOutput())->SetTbssType(OutputType::ROI);
            }


          }

        }
      }


      // RESET LOCALE
      try
      {
        MITK_INFO << " ** Changing locale back from " << setlocale(LC_ALL, NULL) << " to '" << currLocale << "'";
        setlocale(LC_ALL, currLocale.c_str());
      }
      catch(...)
      {
        MITK_INFO << "Could not reset locale " << currLocale;
      }

      MITK_INFO << "...finished!" << std::endl;

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


  template <class TPixelType>
      void NrrdTbssImageReader<TPixelType>
      ::ReadRoiInfo(itk::MetaDataDictionary dict)
  {
    std::vector<std::string> imgMetaKeys = dict.GetKeys();
    std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
    std::string metaString;
    std::vector< itk::Index<3> > roi;

    for (; itKey != imgMetaKeys.end(); itKey ++)
    {
      double x,y,z;
      itk::Index<3> ix;
      itk::ExposeMetaData<std::string> (dict, *itKey, metaString);

      if (itKey->find("ROI_index") != std::string::npos)
      {
        MITK_INFO << *itKey << " ---> " << metaString;
        sscanf(metaString.c_str(), "%lf %lf %lf\n", &x, &y, &z);
        ix[0] = x; ix[1] = y; ix[2] = z;
        roi.push_back(ix);
      }
      else if(itKey->find("preprocessed FA") != std::string::npos)
      {
        MITK_INFO << *itKey << " ---> " << metaString;
        static_cast<OutputType*>(this->GetOutput())->SetPreprocessedFA(true);
        static_cast<OutputType*>(this->GetOutput())->SetPreprocessedFAFile(metaString);
      }

      // Name of structure
      if (itKey->find("structure") != std::string::npos)
      {
        MITK_INFO << *itKey << " ---> " << metaString;
        static_cast<OutputType*>(this->GetOutput())->SetStructure(metaString);
      }
    }
    static_cast<OutputType*>(this->GetOutput())->SetRoi(roi);

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
