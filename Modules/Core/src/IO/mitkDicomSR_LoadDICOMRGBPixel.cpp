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
      case DcmIoType::UCHAR:
        return LoadDICOMByITK<itk::RGBPixel<unsigned char>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::CHAR:
        return LoadDICOMByITK<itk::RGBPixel<char>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::USHORT:
        return LoadDICOMByITK<itk::RGBPixel<unsigned short>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::SHORT:
        return LoadDICOMByITK<itk::RGBPixel<short>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::UINT:
        return LoadDICOMByITK<itk::RGBPixel<unsigned int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::INT:
        return LoadDICOMByITK<itk::RGBPixel<int>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::ULONG:
        return LoadDICOMByITK<itk::RGBPixel<long unsigned int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::LONG:
        return LoadDICOMByITK<itk::RGBPixel<long int>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::FLOAT:
        return LoadDICOMByITK<itk::RGBPixel<float>>(filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::DOUBLE:
        return LoadDICOMByITK<itk::RGBPixel<double>>(
          filenames, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      default:
        MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        return nullptr;
    }
  }

} // end namespace mitk
