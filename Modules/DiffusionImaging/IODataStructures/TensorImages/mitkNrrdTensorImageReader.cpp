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

#include "mitkNrrdTensorImageReader.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkDiffusionTensor3D.h"

#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"

namespace mitk
{

  void NrrdTensorImageReader
    ::GenerateData()
  {
    if ( m_FileName == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename is empty!");
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

        typedef itk::Image<itk::DiffusionTensor3D<float>,3> VecImgType;
        VecImgType::Pointer vecImg = VecImgType::New();
        vecImg->SetSpacing( img->GetSpacing() );   // Set the image spacing
        vecImg->SetOrigin( img->GetOrigin() );     // Set the image origin
        vecImg->SetDirection( img->GetDirection() );  // Set the image direction
        vecImg->SetRegions( img->GetLargestPossibleRegion());
        vecImg->Allocate();

        itk::ImageRegionIterator<VecImgType> ot (vecImg, vecImg->GetLargestPossibleRegion() );
        ot = ot.Begin();

        itk::ImageRegionIterator<ImageType> it (img, img->GetLargestPossibleRegion() );
        it = it.Begin();

        typedef ImageType::PixelType  VarPixType;
        typedef VecImgType::PixelType FixPixType;
        int numComponents = img->GetNumberOfComponentsPerPixel();

        itk::MetaDataDictionary imgMetaDictionary = img->GetMetaDataDictionary();
        std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
        std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
        std::string metaString;

        bool readFrame = false;
        double xx, xy, xz, yx, yy, yz, zx, zy, zz;
        MeasurementFrameType measFrame;
        measFrame.SetIdentity();
        MeasurementFrameType measFrameTransp;
        measFrameTransp.SetIdentity();

        for (; itKey != imgMetaKeys.end(); itKey ++)
        {
          itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
          if (itKey->find("measurement frame") != std::string::npos)
          {
            sscanf(metaString.c_str(), " ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) ( %lf , %lf , %lf ) \n", &xx, &xy, &xz, &yx, &yy, &yz, &zx, &zy, &zz);

            if (xx>10e-10 || xy>10e-10 || xz>10e-10 ||
                yx>10e-10 || yy>10e-10 || yz>10e-10 ||
                zx>10e-10 || zy>10e-10 || zz>10e-10 )
            {
              readFrame = true;

              measFrame(0,0) = xx;
              measFrame(0,1) = xy;
              measFrame(0,2) = xz;
              measFrame(1,0) = yx;
              measFrame(1,1) = yy;
              measFrame(1,2) = yz;
              measFrame(2,0) = zx;
              measFrame(2,1) = zy;
              measFrame(2,2) = zz;

              measFrameTransp = measFrame.GetTranspose();
            }
          }
        }

        if (numComponents==6)
        {
          while (!it.IsAtEnd())
          {
            // T'=RTR'
            VarPixType vec = it.Get();
            FixPixType fixVec(vec.GetDataPointer());

            if(readFrame)
            {
              itk::DiffusionTensor3D<float> tensor;
              tensor.SetElement(0, vec.GetElement(0));
              tensor.SetElement(1, vec.GetElement(1));
              tensor.SetElement(2, vec.GetElement(2));
              tensor.SetElement(3, vec.GetElement(3));
              tensor.SetElement(4, vec.GetElement(4));
              tensor.SetElement(5, vec.GetElement(5));

              tensor = tensor.PreMultiply(measFrame);
              tensor = tensor.PostMultiply(measFrameTransp);
              fixVec = tensor;
            }

            ot.Set(fixVec);
            ++ot;
            ++it;
          }
        }
        else if(numComponents==9)
        {
          while (!it.IsAtEnd())
          {
            VarPixType vec = it.Get();
            itk::DiffusionTensor3D<float> tensor;
            tensor.SetElement(0, vec.GetElement(0));
            tensor.SetElement(1, vec.GetElement(1));
            tensor.SetElement(2, vec.GetElement(2));
            tensor.SetElement(3, vec.GetElement(4));
            tensor.SetElement(4, vec.GetElement(5));
            tensor.SetElement(5, vec.GetElement(8));

            if(readFrame)
            {
              tensor = tensor.PreMultiply(measFrame);
              tensor = tensor.PostMultiply(measFrameTransp);
            }

            FixPixType fixVec(tensor);
            ot.Set(fixVec);
            ++ot;
            ++it;
          }
        }
        else
        {
          throw itk::ImageFileReaderException(__FILE__, __LINE__, "Image has wrong number of pixel components!");
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
        throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, an error occurred while reading the requested DTI file!");
      }
    }
  }

  void NrrdTensorImageReader::GenerateOutputInformation()
  {

  }



  const char* NrrdTensorImageReader
    ::GetFileName() const
  {
    return m_FileName.c_str();
  }


  void NrrdTensorImageReader
    ::SetFileName(const char* aFileName)
  {
    m_FileName = aFileName;
  }


  const char* NrrdTensorImageReader
    ::GetFilePrefix() const
  {
    return m_FilePrefix.c_str();
  }


  void NrrdTensorImageReader
    ::SetFilePrefix(const char* aFilePrefix)
  {
    m_FilePrefix = aFilePrefix;
  }


  const char* NrrdTensorImageReader
    ::GetFilePattern() const
  {
    return m_FilePattern.c_str();
  }


  void NrrdTensorImageReader
    ::SetFilePattern(const char* aFilePattern)
  {
    m_FilePattern = aFilePattern;
  }


  bool NrrdTensorImageReader
    ::CanReadFile(const std::string filename, const std::string /*filePrefix*/, const std::string /*filePattern*/)
  {
    // First check the extension
    if(  filename == "" )
    {
      return false;
    }
    std::string ext = itksys::SystemTools::GetFilenameLastExtension(filename);
    ext = itksys::SystemTools::LowerCase(ext);

    if (ext == ".hdti" || ext == ".dti")
    {
      return true;
    }

    return false;
  }

} //namespace MITK
