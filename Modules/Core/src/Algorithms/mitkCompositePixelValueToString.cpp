/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCompositePixelValueToString.h>
#include <mitkImageAccessByItk.h>
#include <sstream>

namespace mitk
{
  // The output operator for RGBA in ITK does not properly cast to printable type,
  // like the RGB pixel output operator. This here is an indirection to fix this until
  // ITK-3501 is fixed.
  template <typename TComponent>
  std::ostream &operator<<(std::ostream &os, const itk::RGBPixel<TComponent> &c)
  {
    return itk::operator<<(os, c);
  }

  template <typename TComponent>
  std::ostream &operator<<(std::ostream &os, const itk::RGBAPixel<TComponent> &c)
  {
    os << static_cast<typename itk::NumericTraits<TComponent>::PrintType>(c[0]) << "  ";
    os << static_cast<typename itk::NumericTraits<TComponent>::PrintType>(c[1]) << "  ";
    os << static_cast<typename itk::NumericTraits<TComponent>::PrintType>(c[2]) << "  ";
    os << static_cast<typename itk::NumericTraits<TComponent>::PrintType>(c[3]);
    return os;
  }

  template <typename TPixel, unsigned int VImageDimension>
  static void ConvertCompositePixelValueToString(itk::Image<TPixel, VImageDimension> *image,
                                                 itk::Index<3> index3,
                                                 std::string &string)
  {
    itk::Index<VImageDimension> index;
    for (unsigned int i = 0; i < VImageDimension; ++i)
      index[i] = index3[i];

    if (image->GetLargestPossibleRegion().IsInside(index))
    {
      std::ostringstream stream;
      mitk::operator<<(stream, image->GetPixel(index));

      string = stream.str();
    }
    else
    {
      string = "Out of bounds";
    }
  }
}

std::string mitk::ConvertCompositePixelValueToString(Image::Pointer image, itk::Index<3> index)
{
  std::string string;

  if (image.IsNull())
  {
    mitkThrow() << "Image is null.";
  }

  AccessFixedPixelTypeByItk_n(
    image, ConvertCompositePixelValueToString, MITK_ACCESSBYITK_COMPOSITE_PIXEL_TYPES_SEQ, (index, string));

  return string;
}
