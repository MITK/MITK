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

#ifndef mitkCompositePixelValueToString_h
#define mitkCompositePixelValueToString_h

#include <mitkImage.h>
#include <string>

namespace mitk
{
  /** \brief Converts composite pixel values to a displayable string
  *
  * \throws mitk::Exception If the image is nullptr.
  * \throws mitk::AccessByItkException for pixel types which are not part of MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ
  */
  std::string MITKCORE_EXPORT ConvertCompositePixelValueToString(Image::Pointer image, itk::Index<3> index);
}

#endif
