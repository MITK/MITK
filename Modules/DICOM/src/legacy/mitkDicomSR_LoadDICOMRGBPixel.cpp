/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDicomSeriesReader.txx"

namespace mitk
{
  Image::Pointer DicomSeriesReader::MultiplexLoadDICOMByITKRGBPixel(const StringContainer &filenames,
                                                                    bool correctTilt,
                                                                    const GantryTiltInformation &tiltInfo,
                                                                    DcmIoType::Pointer &io,
                                                                    CallbackCommand *command,
                                                                    Image::Pointer preLoadedImageBlock)
  {
    switch (io->GetComponentType())
    {
      case itk::IOComponentEnum::UCHAR:
        return LoadDICOMByITK<itk::RGBPixel<unsigned char>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::CHAR:
        return LoadDICOMByITK<itk::RGBPixel<char>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::USHORT:
        return LoadDICOMByITK<itk::RGBPixel<unsigned short>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::SHORT:
        return LoadDICOMByITK<itk::RGBPixel<short>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::UINT:
        return LoadDICOMByITK<itk::RGBPixel<unsigned int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::INT:
        return LoadDICOMByITK<itk::RGBPixel<int>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::ULONG:
        return LoadDICOMByITK<itk::RGBPixel<long unsigned int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::LONG:
        return LoadDICOMByITK<itk::RGBPixel<long int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::FLOAT:
        return LoadDICOMByITK<itk::RGBPixel<float>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case itk::IOComponentEnum::DOUBLE:
        return LoadDICOMByITK<itk::RGBPixel<double>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      default:
        MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        return nullptr;
    }
  }

} // end namespace mitk
