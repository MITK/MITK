/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCompressedImageContainer_h_Included
#define mitkCompressedImageContainer_h_Included

#include "MitkDataTypesExtExports.h"
#include "mitkCommon.h"
#include "mitkGeometry3D.h"
#include "mitkImage.h"
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
    mitkClassMacroItkParent(CompressedImageContainer, itk::Object);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Creates a compressed version of the image.
       *
       * Will not hold any further SmartPointers to the image.
       *
       */
      void SetImage(const Image *);

    /**
     * \brief Creates a full mitk::Image from its compressed version.
     *
     * This Method hold no buffer, so the uncompression algorithm will be
     * executed every time you call this method. Don't overdo it.
     *
     */
    Image::Pointer GetImage() const;

  protected:
    CompressedImageContainer(); // purposely hidden
    ~CompressedImageContainer() override;

    PixelType *m_PixelType;

    unsigned int m_ImageDimension;
    std::vector<unsigned int> m_ImageDimensions;

    unsigned long m_OneTimeStepImageSizeInBytes;

    unsigned int m_NumberOfTimeSteps;

    /// one for each timestep. first = pointer to compressed data; second = size of buffer in bytes
    std::vector<std::pair<unsigned char *, unsigned long>> m_ByteBuffers;

    BaseGeometry::Pointer m_ImageGeometry;
  };

} // namespace

#endif
