/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCompositePixelValueToString_h
#define mitkCompositePixelValueToString_h

#include <mitkImage.h>
#include <string>

namespace mitk
{
  /** \brief Convert composite pixel values (RGB, RGBA) to a displayable string.
   *
   * \param[in] image The image containing composite pixel data.
   * \param[in] index The 3D index of the pixel to convert.
   * \return A string representation of the pixel value, or "Out of bounds" if the index is outside the image.
   * \throws mitk::Exception If the image is nullptr.
   * \throws mitk::AccessByItkException For pixel types which are not part of MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ.
   */
  std::string MITKCORE_EXPORT ConvertCompositePixelValueToString(Image::Pointer image, itk::Index<3> index);
}

#endif
