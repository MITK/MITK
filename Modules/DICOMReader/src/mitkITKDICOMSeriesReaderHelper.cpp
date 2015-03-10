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

//#define MBILOG_ENABLE_DEBUG

#include "mitkITKDICOMSeriesReaderHelper.h"
#include "mitkITKDICOMSeriesReaderHelper.txx"

#define switch3DCase(IOType, T) \
  case IOType: return LoadDICOMByITK< T >(filenames, correctTilt, tiltInfo, io);

bool
mitk::ITKDICOMSeriesReaderHelper
::CanHandleFile(const std::string& filename)
{
  MITK_DEBUG << "ITKDICOMSeriesReaderHelper::CanHandleFile " << filename;
  itk::GDCMImageIO::Pointer tester = itk::GDCMImageIO::New();
  return tester->CanReadFile(filename.c_str());
}

mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper
::Load( const StringContainer& filenames, bool correctTilt, const GantryTiltInformation& tiltInfo )
{
  if( filenames.empty() )
  {
    MITK_DEBUG << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    return NULL; // this is not actually an error but the result is very simple
  }

  typedef itk::GDCMImageIO DcmIoType;
  DcmIoType::Pointer io = DcmIoType::New();

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
          switch3DCase(DcmIoType::UCHAR, unsigned char)
          switch3DCase(DcmIoType::CHAR, char)
          switch3DCase(DcmIoType::USHORT, unsigned short)
          switch3DCase(DcmIoType::SHORT, short)
          switch3DCase(DcmIoType::UINT, unsigned int)
          switch3DCase(DcmIoType::INT, int)
          switch3DCase(DcmIoType::ULONG, long unsigned int)
          switch3DCase(DcmIoType::LONG, long int)
          switch3DCase(DcmIoType::FLOAT, float)
          switch3DCase(DcmIoType::DOUBLE, double)
          default:
            MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        }
      }
      else if (io->GetPixelType() == itk::ImageIOBase::RGB)
      {
        switch (io->GetComponentType())
        {
          switch3DCase(DcmIoType::UCHAR, itk::RGBPixel<unsigned char>)
          switch3DCase(DcmIoType::CHAR, itk::RGBPixel<char>)
          switch3DCase(DcmIoType::USHORT, itk::RGBPixel<unsigned short>)
          switch3DCase(DcmIoType::SHORT, itk::RGBPixel<short>)
          switch3DCase(DcmIoType::UINT, itk::RGBPixel<unsigned int>)
          switch3DCase(DcmIoType::INT, itk::RGBPixel<int>)
          switch3DCase(DcmIoType::ULONG, itk::RGBPixel<long unsigned int>)
          switch3DCase(DcmIoType::LONG, itk::RGBPixel<long int>)
          switch3DCase(DcmIoType::FLOAT, itk::RGBPixel<float>)
          switch3DCase(DcmIoType::DOUBLE, itk::RGBPixel<double>)
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

#define switch3DnTCase(IOType, T) \
  case IOType: return LoadDICOMByITK3DnT< T >(filenamesLists, correctTilt, tiltInfo, io);

mitk::Image::Pointer
mitk::ITKDICOMSeriesReaderHelper
::Load3DnT( const StringContainerList& filenamesLists, bool correctTilt, const GantryTiltInformation& tiltInfo )
{
  if( filenamesLists.empty() || filenamesLists.front().empty() )
  {
    MITK_DEBUG << "Calling LoadDicomSeries with empty filename string container. Probably invalid application logic.";
    return NULL; // this is not actually an error but the result is very simple
  }

  typedef itk::GDCMImageIO DcmIoType;
  DcmIoType::Pointer io = DcmIoType::New();

  try
  {
    if (io->CanReadFile(filenamesLists.front().front().c_str()))
    {
      io->SetFileName(filenamesLists.front().front().c_str());
      io->ReadImageInformation();

      if (io->GetPixelType() == itk::ImageIOBase::SCALAR)
      {
        switch (io->GetComponentType())
        {
          switch3DnTCase(DcmIoType::UCHAR, unsigned char)
          switch3DnTCase(DcmIoType::CHAR, char)
          switch3DnTCase(DcmIoType::USHORT, unsigned short)
          switch3DnTCase(DcmIoType::SHORT, short)
          switch3DnTCase(DcmIoType::UINT, unsigned int)
          switch3DnTCase(DcmIoType::INT, int)
          switch3DnTCase(DcmIoType::ULONG, long unsigned int)
          switch3DnTCase(DcmIoType::LONG, long int)
          switch3DnTCase(DcmIoType::FLOAT, float)
          switch3DnTCase(DcmIoType::DOUBLE, double)
          default:
            MITK_ERROR << "Found unsupported DICOM scalar pixel type: (enum value) " << io->GetComponentType();
        }
      }
      else if (io->GetPixelType() == itk::ImageIOBase::RGB)
      {
        switch (io->GetComponentType())
        {
          switch3DnTCase(DcmIoType::UCHAR, itk::RGBPixel<unsigned char>)
          switch3DnTCase(DcmIoType::CHAR, itk::RGBPixel<char>)
          switch3DnTCase(DcmIoType::USHORT, itk::RGBPixel<unsigned short>)
          switch3DnTCase(DcmIoType::SHORT, itk::RGBPixel<short>)
          switch3DnTCase(DcmIoType::UINT, itk::RGBPixel<unsigned int>)
          switch3DnTCase(DcmIoType::INT, itk::RGBPixel<int>)
          switch3DnTCase(DcmIoType::ULONG, itk::RGBPixel<long unsigned int>)
          switch3DnTCase(DcmIoType::LONG, itk::RGBPixel<long int>)
          switch3DnTCase(DcmIoType::FLOAT, itk::RGBPixel<float>)
          switch3DnTCase(DcmIoType::DOUBLE, itk::RGBPixel<double>)
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
