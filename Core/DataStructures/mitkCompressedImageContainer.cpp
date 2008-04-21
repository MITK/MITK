/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#include "mitkCompressedImageContainer.h"
#include "itkSmartPointerForwardReference.txx"

#include "itk_zlib.h"

#include <stdlib.h>

mitk::CompressedImageContainer::CompressedImageContainer()
{
}

mitk::CompressedImageContainer::~CompressedImageContainer()
{
  for (std::vector< std::pair<unsigned char*, unsigned long> >::iterator iter = m_ByteBuffers.begin();
       iter != m_ByteBuffers.end();
       ++iter)
  {
    free( iter->first );
  }
}

void mitk::CompressedImageContainer::SetImage( Image* image )
{
  for (std::vector< std::pair<unsigned char*, unsigned long> >::iterator iter = m_ByteBuffers.begin();
       iter != m_ByteBuffers.end();
       ++iter)
  {
    free( iter->first );
  }

  m_ByteBuffers.clear();

  // Compress diff image using zlib (will be restored on demand)
  // determine memory size occupied by voxel data
  m_ImageDimension = image->GetDimension();
  m_ImageDimensions.clear();
  m_PixelType = image->GetPixelType();
  m_OneTimeStepImageSizeInBytes = m_PixelType.GetBpe() >> 3; // bits per element divided by 8
  for (unsigned int i = 0; i < m_ImageDimension; ++i)
  {
    unsigned int currentImageDimension = image->GetDimension(i);
    m_ImageDimensions.push_back( currentImageDimension );
    if (i < 3)
    {
      m_OneTimeStepImageSizeInBytes *= currentImageDimension; // only the 3D memory size
    }
  }

  m_ImageGeometry = image->GetGeometry();

  m_NumberOfTimeSteps = 1;
  if (m_ImageDimension > 3)
  {
    m_NumberOfTimeSteps = image->GetDimension(3);
  }

  for (unsigned int timestep = 0; timestep < m_NumberOfTimeSteps; ++timestep)
  {
    // allocate a buffer as specified by zlib
    unsigned long bufferSize = m_OneTimeStepImageSizeInBytes + static_cast<unsigned long>(m_OneTimeStepImageSizeInBytes * 0.2) + 12;
    unsigned char* byteBuffer = (unsigned char*) malloc(bufferSize);
    
    if (itk::Object::GetDebug())
    {
    // compress image here into a buffer
      std::cout << "Using ZLib version: '" << zlibVersion() << "'" << std::endl;
      std::cout << "Attempting to compress " << m_OneTimeStepImageSizeInBytes << " image bytes into a buffer of size " << bufferSize << std::endl;
    }

    ::Bytef* dest(byteBuffer);
    ::uLongf destLen(bufferSize);
    ::Bytef* source( static_cast<unsigned char*>(image->GetVolumeData(timestep)->GetData()) );
    ::uLongf sourceLen( m_OneTimeStepImageSizeInBytes );
    int zlibRetVal = ::compress(dest, &destLen, source, sourceLen);
    if (itk::Object::GetDebug())
    {
      if (zlibRetVal == Z_OK)
      {
        std::cout << "Success, using " << destLen << " bytes of the buffer (ratio " << ((double)destLen / (double)sourceLen) << ")" << std::endl;
      }
      else
      {
        std::cout << "Failure, ";
        switch ( zlibRetVal )
        {
          case Z_MEM_ERROR:
            std::cout << "not enough memory" << std::endl;
            break;
          case Z_BUF_ERROR:
            std::cout << "output buffer too small" << std::endl;
            break;
          default:
            std::cout << "other, unspecified error" << std::endl;
            break;
        }
      }
    }

    // only use the neccessary amount of memory, realloc the buffer!
    byteBuffer = (unsigned char*) realloc( byteBuffer, destLen );
    bufferSize = destLen;
    //std::cout << "Using " << bufferSize << " bytes to store compressed image (" << destLen << " needed)" << std::endl;
  
    m_ByteBuffers.push_back( std::pair<unsigned char*, unsigned long>( byteBuffer, bufferSize ) );
  }
}

mitk::Image::Pointer mitk::CompressedImageContainer::GetImage()
{
  if (m_ByteBuffers.empty()) return NULL;

  // uncompress image data, create an Image
  Image::Pointer image = Image::New();
  unsigned int dims[20]; // more than 20 dimensions and bang
  for (unsigned int dim = 0; dim < m_ImageDimension; ++dim)
    dims[dim] = m_ImageDimensions[dim];
  
  image->Initialize( m_PixelType, m_ImageDimension, dims ); // this IS needed, right ?? But it does allocate memory -> does create one big lump of memory (also in windows)

  unsigned int timeStep(0);
  for (std::vector< std::pair<unsigned char*, unsigned long> >::iterator iter = m_ByteBuffers.begin();
       iter != m_ByteBuffers.end();
       ++iter, ++timeStep)
  {
    ::Bytef* dest( static_cast<unsigned char*>(image->GetVolumeData(timeStep)->GetData()) );
    ::uLongf destLen(m_OneTimeStepImageSizeInBytes);
    ::Bytef* source( iter->first );
    ::uLongf sourceLen( iter->second );
    int zlibRetVal = ::uncompress(dest, &destLen, source, sourceLen);
    if (itk::Object::GetDebug())
    {
      if (zlibRetVal == Z_OK)
      {
        std::cout << "Success, destLen now " << destLen << " bytes" << std::endl;
      }
      else
      {
        std::cout << "Failure, ";
        switch ( zlibRetVal )
        {
          case Z_DATA_ERROR:
            std::cout << "compressed data corrupted" << std::endl;
            break;
          case Z_MEM_ERROR:
            std::cout << "not enough memory" << std::endl;
            break;
          case Z_BUF_ERROR:
            std::cout << "output buffer too small" << std::endl;
            break;
          default:
            std::cout << "other, unspecified error" << std::endl;
            break;
        }
      }
    }
  }

  image->SetGeometry( m_ImageGeometry );
  image->Modified();

  return image;
}

