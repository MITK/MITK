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


#include "mitkRawImageFileReader.h"
#include "mitkITKImageImport.h"
#include "mitkImageCast.h"
#include "mitkIOConstants.h"
#include "mitkIOMimeTypes.h"

#include <itkImage.h>
#include <itkRawImageIO.h>
#include <itkImageFileReader.h>

mitk::RawImageFileReaderService::RawImageFileReaderService()
  : AbstractFileReader(CustomMimeType(IOMimeTypes::RAW_MIMETYPE_NAME()), "ITK raw image reader")
{
  Options defaultOptions;

  defaultOptions[IOConstants::PIXEL_TYPE()] = IOConstants::PIXEL_TYPE_USHORT();

  std::vector<std::string> pixelEnum;
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_UCHAR());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_CHAR());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_USHORT());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_SHORT());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_UINT());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_INT());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_FLOAT());
  pixelEnum.push_back(IOConstants::PIXEL_TYPE_DOUBLE());
  defaultOptions[IOConstants::PIXEL_TYPE_ENUM()] = pixelEnum;

  defaultOptions[IOConstants::DIMENSION()] = std::string("3");
  std::vector<std::string> dimEnum;
  dimEnum.push_back("2");
  dimEnum.push_back("3");
  defaultOptions[IOConstants::DIMENSION_ENUM()] = dimEnum;

  defaultOptions[IOConstants::ENDIANNESS()] = IOConstants::ENDIANNESS_LITTLE();
  std::vector<std::string> endianEnum;
  endianEnum.push_back(IOConstants::ENDIANNESS_LITTLE());
  endianEnum.push_back(IOConstants::ENDIANNESS_BIG());
  defaultOptions[IOConstants::ENDIANNESS_ENUM()] = endianEnum;

  defaultOptions[IOConstants::SIZE_X()] = 0;
  defaultOptions[IOConstants::SIZE_Y()] = 0;
  defaultOptions[IOConstants::SIZE_Z()] = 0;
  //defaultOptions[IOConstants::SIZE_T()] = 0;

  this->SetDefaultOptions(defaultOptions);

  this->RegisterService();
}

mitk::RawImageFileReaderService::RawImageFileReaderService(const mitk::RawImageFileReaderService& other)
  : AbstractFileReader(other)
{
}

std::vector<itk::SmartPointer<mitk::BaseData> > mitk::RawImageFileReaderService::Read()
{
  std::vector<mitk::BaseData::Pointer> result;

  const std::string path = this->GetLocalFileName();

  const Options options = this->GetOptions();

  const std::string dimensionality = options.find(IOConstants::DIMENSION())->second.ToString();
  const std::string pixelType = options.find(IOConstants::PIXEL_TYPE())->second.ToString();

  EndianityType endianity = options.find(IOConstants::ENDIANNESS())->second.ToString() == IOConstants::ENDIANNESS_LITTLE() ? LITTLE : BIG;
  int dimensions[4];
  dimensions[0] = us::any_cast<int>(options.find(IOConstants::SIZE_X())->second);
  dimensions[1] = us::any_cast<int>(options.find(IOConstants::SIZE_Y())->second);
  dimensions[2] = us::any_cast<int>(options.find(IOConstants::SIZE_Z())->second);
  dimensions[3] = 0 ;//us::any_cast<int>(options.find(IOConstants::SIZE_T())->second);

  // check file dimensionality and pixel type and perform reading according to it
  if (dimensionality == "2")
  {
    if (pixelType == IOConstants::PIXEL_TYPE_CHAR()) result.push_back(TypedRead<signed char, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_UCHAR()) result.push_back(TypedRead<unsigned char, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_SHORT()) result.push_back(TypedRead<signed short int, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_USHORT()) result.push_back(TypedRead<unsigned short int, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_UINT()) result.push_back(TypedRead<unsigned int, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_INT()) result.push_back(TypedRead<signed int, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_FLOAT()) result.push_back(TypedRead<float, 2 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_DOUBLE())result.push_back(TypedRead<double, 2 >(path, endianity, dimensions));
    else
    {
      MITK_INFO << "Error while reading raw file: Dimensionality or pixel type not supported or not properly set" << std::endl;
    }
   }
  else if (dimensionality == "3")
  {
    if (pixelType == IOConstants::PIXEL_TYPE_CHAR()) result.push_back(TypedRead<signed char, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_UCHAR()) result.push_back(TypedRead<unsigned char, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_SHORT()) result.push_back(TypedRead<signed short int, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_USHORT()) result.push_back(TypedRead<unsigned short int, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_UINT()) result.push_back(TypedRead<unsigned int, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_INT()) result.push_back(TypedRead<signed int, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_FLOAT()) result.push_back(TypedRead<float, 3 >(path, endianity, dimensions));
    else if (pixelType == IOConstants::PIXEL_TYPE_DOUBLE()) result.push_back(TypedRead<double, 3 >(path, endianity, dimensions));
    else
    {
      MITK_INFO << "Error while reading raw file: Dimensionality or pixel type not supported or not properly set" << std::endl;
    }
  }
  else
  {
    MITK_INFO << "Error while reading raw file: Dimensionality not supported" << std::endl;
  }

  return result;
}

template < typename TPixel, unsigned int VImageDimensions >
mitk::BaseData::Pointer mitk::RawImageFileReaderService::TypedRead(const std::string& path, EndianityType endianity, int* size)
{
  typedef itk::Image< TPixel, VImageDimensions > ImageType;
  typedef itk::ImageFileReader< ImageType > ReaderType;
  typedef itk::RawImageIO< TPixel, VImageDimensions >  IOType;

  typename ReaderType::Pointer reader = ReaderType::New();
  typename IOType::Pointer io = IOType::New();

  io->SetFileDimensionality(VImageDimensions);

  for (unsigned short int dim = 0; dim < VImageDimensions; ++dim)
  {
    io->SetDimensions(dim, size[dim] );
  }

  if (endianity == LITTLE)
  {
    io->SetByteOrderToLittleEndian();
  }
  else if (endianity == BIG)
  {
    io->SetByteOrderToBigEndian();
  }
  else
  {
    MITK_INFO << "Warning: endianity not properly set. Resulting image might be incorrect";
  }

  reader->SetImageIO( io );
  reader->SetFileName(path);

  try
  {
    reader->Update();
  }
  catch( itk::ExceptionObject & err )
  {
    MITK_ERROR <<"An error occurred during the raw image reading process: ";
    MITK_INFO << err << std::endl;
  }

  mitk::Image::Pointer image = mitk::Image::New();
  mitk::CastToMitkImage(reader->GetOutput(), image);
  image->SetVolume(  reader->GetOutput()->GetBufferPointer());
  return image.GetPointer();
}

mitk::RawImageFileReaderService*mitk::RawImageFileReaderService::Clone() const
{
  return new RawImageFileReaderService(*this);
}
