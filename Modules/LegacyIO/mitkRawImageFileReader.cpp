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
#include "mitkImageCast.h"

#include <itkImage.h>
#include <itkRawImageIO.h>
#include <itkImageFileReader.h>

mitk::RawImageFileReader::RawImageFileReader()
    : m_FileName(""), m_FilePrefix(""), m_FilePattern("")
{
}

mitk::RawImageFileReader::~RawImageFileReader()
{
}

void mitk::RawImageFileReader::SetDimensions(unsigned int i, unsigned int dim)
{
  if ( i > 2 ) return;

  this->Modified();               // TODO: this order (first modified, then set the variable) is intended??
  m_Dimensions[i] = dim;
}


unsigned int mitk::RawImageFileReader::GetDimensions(unsigned int i) const
{
  if ( i > 2 ) return 0;

  return m_Dimensions[i];
}

bool mitk::RawImageFileReader::CanReadFile(const std::string filename, const std::string filePrefix, const std::string filePattern)
{
  // First check the extension
  if(  filename == "" )
    return false;

  // check if image is serie
  if( filePattern != "" && filePrefix != "" )
    return false;

  return true;
}

void mitk::RawImageFileReader::GenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();

  if (this->GetOutput()==nullptr)
  {
    MITK_INFO << "Error" << std::endl;
  }

  // Check to see if we can read the file given the name or prefix
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "File Type not supported!" );
    return ;
  }

  // check file dimensionality and pixel type and perform reading according to it
  if (m_Dimensionality == 2)
  {
         if (m_PixelType == SCHAR) TypedGenerateData<signed char, 2 >();
    else if (m_PixelType == UCHAR) TypedGenerateData<unsigned char, 2 >();
    else if (m_PixelType == SSHORT) TypedGenerateData<signed short int, 2 >();
    else if (m_PixelType == USHORT) TypedGenerateData<unsigned short int, 2 >();
    else if (m_PixelType == UINT)   TypedGenerateData<unsigned int, 2 >();
    else if (m_PixelType == SINT)   TypedGenerateData<signed int, 2 >();
    else if (m_PixelType == FLOAT)  TypedGenerateData<float, 2 >();
    else if (m_PixelType == DOUBLE) TypedGenerateData<double, 2 >();
    else
    {
      MITK_INFO << "Error while reading raw file: Dimensionality or pixel type not supported or not properly set" << std::endl;
      return;
    }
   }
  else if (m_Dimensionality==3)
  {
         if (m_PixelType == SCHAR) TypedGenerateData<signed char, 3 >();
    else if (m_PixelType == UCHAR) TypedGenerateData<unsigned char, 3 >();
    else if (m_PixelType == SSHORT) TypedGenerateData<signed short int, 3 >();
    else if (m_PixelType == USHORT) TypedGenerateData<unsigned short int, 3 >();
    else if (m_PixelType == UINT)   TypedGenerateData<unsigned int, 3 >();
    else if (m_PixelType == SINT)   TypedGenerateData<signed int, 3 >();
    else if (m_PixelType == FLOAT)  TypedGenerateData<float, 3 >();
    else if (m_PixelType == DOUBLE) TypedGenerateData<double, 3 >();
    else
    {
      MITK_INFO << "Error while reading raw file: Dimensionality or pixel type not supported or not properly set" << std::endl;
      return;
    }
  }
  else
  {
    MITK_INFO << "Error while reading raw file: Dimensionality not supported" << std::endl;
    return;

  }

  MITK_INFO << "...reading raw finished!" << std::endl;
}

template < typename TPixel, unsigned int VImageDimensions >
void mitk::RawImageFileReader::TypedGenerateData()
{
  mitk::Image::Pointer output = this->GetOutput();

  if (this->GetOutput()==nullptr)
  {
    MITK_INFO << "Error" << std::endl;
  }

  MITK_INFO << "loading " << m_FileName << " via itk::ImageIOFactory... " << std::endl;

  // Check to see if we can read the file given the name or prefix
  if ( m_FileName == "" )
  {
    itkWarningMacro( << "File Type not supported!" );
    return ;
  }

  typedef itk::Image< TPixel, VImageDimensions > ImageType;
  typedef itk::ImageFileReader< ImageType > ReaderType;
  typedef itk::RawImageIO< TPixel, VImageDimensions >  IOType;

  typename ReaderType::Pointer reader = ReaderType::New();
  typename IOType::Pointer io = IOType::New();

  io->SetFileDimensionality(VImageDimensions);

  for (unsigned short int dim = 0; dim < VImageDimensions; ++dim)
  {
    io->SetDimensions(dim, m_Dimensions[dim] );
  }

  if (m_Endianity == LITTLE)
  {
    io->SetByteOrderToLittleEndian();
  }
  else if (m_Endianity == BIG)
  {
    io->SetByteOrderToBigEndian();
  }
  else
  {
    MITK_INFO << "Warning: endianity not properly set. Resulting image might be incorrect";
  }

  reader->SetImageIO( io );
  reader->SetFileName(m_FileName.c_str());

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
  output->Initialize( image );
  output->SetVolume(  reader->GetOutput()->GetBufferPointer());
}
