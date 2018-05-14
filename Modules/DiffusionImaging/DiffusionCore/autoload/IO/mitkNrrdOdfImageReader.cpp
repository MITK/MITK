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

#include "mitkNrrdOdfImageReader.h"
#include <mitkCustomMimeType.h>
#include "mitkDiffusionCoreIOMimeTypes.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"
#include <mitkLocaleSwitch.h>

namespace mitk
{
  NrrdOdfImageReader::NrrdOdfImageReader(const NrrdOdfImageReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  NrrdOdfImageReader::NrrdOdfImageReader()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::ODF_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::ODF_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  NrrdOdfImageReader::~NrrdOdfImageReader()
  {
  }

  std::vector<itk::SmartPointer<BaseData> > NrrdOdfImageReader::Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    std::string location = GetInputLocation();

    if ( location == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename of the ODF image to be read is empty!");
    }
    else
    {
      try
      {
        mitk::LocaleSwitch localeSwitch("C");

        typedef itk::VectorImage<float,3> ImageType;
        itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
        typedef itk::ImageFileReader<ImageType> FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetImageIO(io);
        reader->SetFileName(location);
        reader->Update();
        ImageType::Pointer img = reader->GetOutput();

        typedef itk::Image<itk::Vector<float,ODF_SAMPLING_SIZE>,3> VecImgType;
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

mitk::NrrdOdfImageReader* mitk::NrrdOdfImageReader::Clone() const
{
  return new NrrdOdfImageReader(*this);
}
