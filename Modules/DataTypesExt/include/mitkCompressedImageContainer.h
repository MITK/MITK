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

#ifndef mitkCompressedImageContainer_h_Included
#define mitkCompressedImageContainer_h_Included

#include "mitkCommon.h"
#include "MitkDataTypesExtExports.h"
#include "mitkImage.h"
#include "mitkGeometry3D.h"
#include "mitkImageDataItem.h"

#include <itkObject.h>

#include <vector>

namespace mitk
{

/**
  \brief Holds one (compressed) mitk::Image

  Uses zlib to compress the data of an mitk::Image.

  $Author$
*/
class MITKDATATYPESEXT_EXPORT CompressedImageContainer : public itk::Object
{
  public:

    mitkClassMacro(CompressedImageContainer, Object);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    /**
     * \brief Creates a compressed version of the image.
     *
     * Will not hold any further SmartPointers to the image.
     *
     */
    void SetImage( Image* );

    /**
     * \brief Creates a full mitk::Image from its compressed version.
     *
     * This Method hold no buffer, so the uncompression algorithm will be
     * executed every time you call this method. Don't overdo it.
     *
     */
    Image::Pointer GetImage();

  protected:

    CompressedImageContainer(); // purposely hidden
    virtual ~CompressedImageContainer();

    PixelType *m_PixelType;

    unsigned int m_ImageDimension;
    std::vector<unsigned int> m_ImageDimensions;

    unsigned long  m_OneTimeStepImageSizeInBytes;

    unsigned int m_NumberOfTimeSteps;

    /// one for each timestep. first = pointer to compressed data; second = size of buffer in bytes
    std::vector< std::pair<unsigned char*, unsigned long> > m_ByteBuffers;

    BaseGeometry::Pointer m_ImageGeometry;
};

} // namespace

#endif
