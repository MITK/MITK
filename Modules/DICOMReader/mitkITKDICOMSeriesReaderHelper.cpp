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

#include "mitkITKDICOMSeriesReaderHelper.h"
#include "mitkITKDICOMSeriesReaderHelper.txx"

mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper
::Load( const StringContainer& filenames, bool correctTilt, bool itkNotUsed(tiltInfo) )
{
  if( filenames.empty() )
  {
    MITK_DEBUG << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic."; // TODO
    return NULL; // this is not actually an error but the result is very simple
  }

  DcmIoType::Pointer io = DcmIoType::New();

  GantryTiltInformation tiltInfo; // TODO
  Image::Pointer preLoadedImageBlock; // TODO

  try
  {
    if (io->CanReadFile(filenames.front().c_str()))
    {
      io->SetFileName(filenames.front().c_str());
      io->ReadImageInformation();

      if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
      {
        switch (io->GetComponentType())
        {
          case DcmIoType::UCHAR: return LoadDICOMByITK<unsigned char>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::CHAR: return LoadDICOMByITK<char>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::USHORT: return LoadDICOMByITK<unsigned short>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::SHORT: return LoadDICOMByITK<short>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::UINT: return LoadDICOMByITK<unsigned int>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::INT: return LoadDICOMByITK<int>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::ULONG: return LoadDICOMByITK<long unsigned int>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::LONG: return LoadDICOMByITK<long int>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::FLOAT: return LoadDICOMByITK<float>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::DOUBLE: return LoadDICOMByITK<double>(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          default:
            MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        }
      }
      else if (io->GetPixelType() == itk::ImageIOBase::RGB)
      {
        switch (io->GetComponentType())
        {
          case DcmIoType::UCHAR: return LoadDICOMByITK< itk::RGBPixel<unsigned char> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::CHAR: return LoadDICOMByITK<itk::RGBPixel<char> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::USHORT: return LoadDICOMByITK<itk::RGBPixel<unsigned short> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::SHORT: return LoadDICOMByITK<itk::RGBPixel<short> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::UINT: return LoadDICOMByITK<itk::RGBPixel<unsigned int> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::INT: return LoadDICOMByITK<itk::RGBPixel<int> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::ULONG: return LoadDICOMByITK<itk::RGBPixel<long unsigned int> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::LONG: return LoadDICOMByITK<itk::RGBPixel<long int> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::FLOAT: return LoadDICOMByITK<itk::RGBPixel<float> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          case DcmIoType::DOUBLE: return LoadDICOMByITK<itk::RGBPixel<double> >(filenames, correctTilt, tiltInfo, io, preLoadedImageBlock);
          default:
            MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        }
      }

      MITK_ERROR << "Unsupported DICOM pixel type";
      return NULL;
    }
  }
  catch(itk::MemoryAllocationError& e)
  {
    MITK_ERROR << "Out of memory. Cannot load DICOM series: " << e.what();
  }
  catch(std::exception& e)
  {
    MITK_ERROR << "Error encountered when loading DICOM series:" << e.what();
  }
  catch(...)
  {
    MITK_ERROR << "Unspecified error encountered when loading DICOM series.";
  }

  return NULL;
}
