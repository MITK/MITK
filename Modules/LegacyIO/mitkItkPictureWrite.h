/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

/** @file mitkItkPictureWrite.h */

#ifndef MITKITKPICTUREWRITE_H
#define MITKITKPICTUREWRITE_H

#include <itkImage.h>
#include <mitkCommon.h>

/**
 * @brief ITK-Like method to be called for writing an single-component image using the AccessByItk Macros
 *
 * @param itkImage an image with single-component pixel type
 * @param fileName the filename
 *
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
 */
template <typename TPixel, unsigned int VImageDimension>
DEPRECATED(void _mitkItkPictureWrite(itk::Image<TPixel, VImageDimension> *itkImage, const std::string &fileName));

/**
 * @brief ITK-Like method to be called for writing an image
 *
 * @param itkImage an Image with single-component or composite pixel type
 * @param fileName the filename
 *
 * @deprecatedSince{2014_10} Use mitk::IOUtils or mitk::FileReaderRegistry instead.
 */
template <typename TPixel, unsigned int VImageDimension>
DEPRECATED(void _mitkItkPictureWriteComposite(itk::Image<TPixel, VImageDimension> *itkImage,
                                              const std::string &fileName));

#endif /* MITKITKPICTUREWRITE_H */
