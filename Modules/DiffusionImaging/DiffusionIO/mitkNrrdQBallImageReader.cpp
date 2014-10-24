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
#include <mitkCustomMimeType.h>
#include "mitkDiffusionIOMimeTypes.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"

namespace mitk
{
  NrrdQBallImageReader::NrrdQBallImageReader(const NrrdQBallImageReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  NrrdQBallImageReader::NrrdQBallImageReader()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionIOMimeTypes::QBI_MIMETYPE_NAME() ), mitk::DiffusionIOMimeTypes::QBI_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  NrrdQBallImageReader::~NrrdQBallImageReader()
  {
  }

  std::vector<itk::SmartPointer<BaseData> > NrrdQBallImageReader::Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    std::string location = GetInputLocation();

    if ( location == "")
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
        reader->SetFileName(location);
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
        ot.GoToBegin();

        itk::ImageRegionIterator<ImageType> it (img, img->GetLargestPossibleRegion() );

        typedef ImageType::PixelType  VarPixType;
        typedef VecImgType::PixelType FixPixType;

        for (it.GoToBegin(); !it.IsAtEnd(); ++it)
        {
          VarPixType vec = it.Get();
          FixPixType fixVec(vec.GetDataPointer());
          ot.Set(fixVec);
          ++ot;
        }

        OutputType::Pointer resultImage = OutputType::New();
        resultImage->InitializeByItk( vecImg.GetPointer() );
        resultImage->SetVolume( vecImg->GetBufferPointer() );
        result.push_back( resultImage.GetPointer() );

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
    return result;
  }

} //namespace MITK

mitk::NrrdQBallImageReader* mitk::NrrdQBallImageReader::Clone() const
{
  return new NrrdQBallImageReader(*this);
}
