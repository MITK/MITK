/*=========================================================================

Program:   Insight Segmentation & Registration Toolkit
Language:  C++
Date:      $Date: 2007-08-17 20:16:25 $
Version:   $Revision: 1.31 $

Copyright (c) Insight Software Consortium. All rights reserved.
See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

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
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename of the vessel tree to be read is empty!");
    }
    else
    {
      try
      {
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

        mitk::GrabItkImageMemory(vecImg, this->GetOutput());

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
