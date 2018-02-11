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

#include "mitkShImageReader.h"
#include <mitkCustomMimeType.h>
#include "mitkDiffusionCoreIOMimeTypes.h"

#include "itkImageFileReader.h"
#include "itkImageRegionIterator.h"
#include "itkMetaDataObject.h"
#include "itkNrrdImageIO.h"
#include "itkNiftiImageIO.h"
#include "mitkITKImageImport.h"
#include "mitkImageDataItem.h"
#include <mitkLocaleSwitch.h>
#include <itkShCoefficientImageImporter.h>
#include <mitkImageCast.h>

namespace mitk
{
  ShImageReader::ShImageReader(const ShImageReader& other)
    : mitk::AbstractFileReader(other)
  {
  }

  ShImageReader::ShImageReader()
    : mitk::AbstractFileReader( CustomMimeType( mitk::DiffusionCoreIOMimeTypes::SH_MIMETYPE() ), mitk::DiffusionCoreIOMimeTypes::SH_MIMETYPE_DESCRIPTION() )
  {
    m_ServiceReg = this->RegisterService();
  }

  ShImageReader::~ShImageReader()
  {
  }

  template <int shOrder>
  mitk::Image::Pointer ShImageReader::ConvertShImage(ShImage::ShOnDiskType::Pointer img)
  {
    typedef itk::ShCoefficientImageImporter< float, shOrder > ImporterType;
    typename ImporterType::Pointer importer = ImporterType::New();
    importer->SetInputImage(img);
    importer->GenerateData();

    mitk::ShImage::Pointer shImage = mitk::ShImage::New();
    mitk::Image::Pointer resultImage = dynamic_cast<mitk::Image*>(shImage.GetPointer());
    mitk::CastToMitkImage(importer->GetCoefficientImage(), resultImage);
    resultImage->SetVolume(importer->GetCoefficientImage()->GetBufferPointer());
    return resultImage;
  }

  std::vector<itk::SmartPointer<BaseData> > ShImageReader::Read()
  {
    std::vector<itk::SmartPointer<mitk::BaseData> > result;
    std::string location = GetInputLocation();

    if ( location == "")
    {
      throw itk::ImageFileReaderException(__FILE__, __LINE__, "Sorry, the filename is empty!");
    }
    else
    {
      try
      {
        std::string ext = itksys::SystemTools::GetFilenameExtension(location);
        typedef itk::ImageFileReader< ShImage::ShOnDiskType > FileReaderType;
        FileReaderType::Pointer reader = FileReaderType::New();
        reader->SetFileName(location);
        if (ext==".shi")
        {
          itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
          reader->SetImageIO(io);
        }
        reader->Update();
        ShImage::ShOnDiskType::Pointer img = reader->GetOutput();

        switch (img->GetLargestPossibleRegion().GetSize()[3])
        {
        case 6:
          result.push_back( ConvertShImage<2>(img).GetPointer() );
          break;
        case 15:
          result.push_back( ConvertShImage<4>(img).GetPointer() );
          break;
        case 28:
          result.push_back( ConvertShImage<6>(img).GetPointer() );
          break;
        case 45:
          result.push_back( ConvertShImage<8>(img).GetPointer() );
          break;
        case 66:
          result.push_back( ConvertShImage<10>(img).GetPointer() );
          break;
        case 91:
          result.push_back( ConvertShImage<12>(img).GetPointer() );
          break;
        default :
          mitkThrow() << "SH order larger 12 not supported";
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

mitk::ShImageReader* mitk::ShImageReader::Clone() const
{
  return new ShImageReader(*this);
}
