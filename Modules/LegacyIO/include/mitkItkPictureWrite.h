/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkItkPictureWrite_h
#define mitkItkPictureWrite_h

#include <itkImage.h>
#include <mitkCommon.h>

/**
 * \brief Write an ITK image with single-component pixel type to a picture file.
 *
 * This is an ITK-style template function intended to be called via the MITK
 * AccessByItk macros for writing images to standard picture formats (e.g., PNG, TIFF).
 * It internally uses ITK image writers.
 *
 * \tparam TPixel The pixel type of the ITK image (must be a single-component scalar type).
 * \tparam VImageDimension The dimension of the ITK image.
 * \param[in] itkImage Pointer to the ITK image to write.
 * \param[in] fileName The output file path including the extension that determines
 *            the image format.
 */
template <typename TPixel, unsigned int VImageDimension>
void _mitkItkPictureWrite(itk::Image<TPixel, VImageDimension> *itkImage, const std::string &fileName);

/**
 * \brief Write an ITK image with single-component or composite pixel type to a picture file.
 *
 * Similar to _mitkItkPictureWrite(), but additionally supports composite (multi-component)
 * pixel types such as RGB or RGBA.
 *
 * \tparam TPixel The pixel type of the ITK image (may be scalar or composite).
 * \tparam VImageDimension The dimension of the ITK image.
 * \param[in] itkImage Pointer to the ITK image to write.
 * \param[in] fileName The output file path including the extension that determines
 *            the image format.
 */
template <typename TPixel, unsigned int VImageDimension>
void _mitkItkPictureWriteComposite(itk::Image<TPixel, VImageDimension> *itkImage,
                                   const std::string &fileName);

#endif
