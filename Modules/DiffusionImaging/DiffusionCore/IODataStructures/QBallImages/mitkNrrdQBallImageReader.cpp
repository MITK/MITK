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

#include "mitkNrrdQBallImageReader.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"

namespace mitk
{

  void NrrdQBallImageReader
    ::GenerateData()
  {
    if ( m_FileName == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename of the vessel tree to be read is empty!");
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

        typedef itk::VectorImage<float,3> ImageType;
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        typedef itk::ImageFileReader<ImageType> FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetImageIO(io);
        reader->SetFileName(this->m_FileName);
        reader->Update();
        ImageType::Pointer img = reader->GetOutput();

        typedef itk::Image<itk::Vector<float,QBALL_ODFSIZE>,3> VecImgType;
        VecImgType::Pointer vecImg = VecImgType::New();
        vecImg->SetSpacing( img->GetSpacing() );   // Set the image spacing
        vecImg->SetOrigin( img->GetOrigin() );     // Set the image origin
        vecImg->SetDirection( img->GetDirection() );  // Set the image direction
        vecImg->SetLargestPossibleRegion( img->GetLargestPossibleRegion());
        vecImg->SetBufferedRegion( img->GetLargestPossibleRegion() );
        vecImg->Allocate();

        itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
        ot = ot.Begin();

        itk::ImageRegionIterator<ImageType> it (img, img->GetLargestPossibleRegion() );

        typedef ImageType::PixelType  VarPixType;
        typedef VecImgType::PixelType FixPixType;

        for (it = it.Begin(); !it.IsAtEnd(); ++it)
        {
          VarPixType vec = it.Get();
          FixPixType fixVec(vec.GetDataPointer());
          ot.Set(fixVec);
          ++ot;
        }

        this->GetOutput()->InitializeByItk(vecImg.GetPointer());
        this->GetOutput()->SetVolume(vecImg->GetBufferPointer());

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
        throw itk::ImageFileReaderException(__FILE__, __LINE__, e.what());
      }
      catch(...)
      {
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested vessel tree file!");
      }
    }
  }

  void NrrdQBallImageReader::GenerateOutputInformation()
  {

  }



  const char* NrrdQBallImageReader
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }


  void NrrdQBallImageReader
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }


  const char* NrrdQBallImageReader
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }


  void NrrdQBallImageReader
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }


  const char* NrrdQBallImageReader
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }


  void NrrdQBallImageReader
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }


  bool NrrdQBallImageReader
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".hqbi" || ext == ".qbi")
    {
      return true;
    }

    return false;
  }

} //namespace MITK
