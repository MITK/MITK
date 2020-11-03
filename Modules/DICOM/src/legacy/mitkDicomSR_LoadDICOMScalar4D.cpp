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
  Image::Pointer DicomSeriesReader::MultiplexLoadDICOMByITK4DScalar(std::list<StringContainer> &imageBlocks,
                                                                    ImageBlockDescriptor imageBlockDescriptor,
                                                                    bool correctTilt,
                                                                    const GantryTiltInformation &tiltInfo,
                                                                    DcmIoType::Pointer &io,
                                                                    CallbackCommand *command,
                                                                    Image::Pointer preLoadedImageBlock)
  {
    switch (io->GetComponentType())
    {
      case DcmIoType::UCHAR:
        return LoadDICOMByITK4D<unsigned char>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::CHAR:
        return LoadDICOMByITK4D<char>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::USHORT:
        return LoadDICOMByITK4D<unsigned short>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::SHORT:
        return LoadDICOMByITK4D<short>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::UINT:
        return LoadDICOMByITK4D<unsigned int>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::INT:
        return LoadDICOMByITK4D<int>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::ULONG:
        return LoadDICOMByITK4D<long unsigned int>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::LONG:
        return LoadDICOMByITK4D<long int>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::FLOAT:
        return LoadDICOMByITK4D<float>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      case DcmIoType::DOUBLE:
        return LoadDICOMByITK4D<double>(
          imageBlocks, imageBlockDescriptor, correctTilt, tiltInfo, io, command, preLoadedImageBlock);
      default:
        MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        return nullptr;
    }
  }

} // end namespace mitk

#include <legacy/mitkDicomSeriesReader.txx>
