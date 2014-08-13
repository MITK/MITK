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

#ifndef MITKIMAGEREGISTRATIONMETHODACCESSFUNCTOR_H
#define MITKIMAGEREGISTRATIONMETHODACCESSFUNCTOR_H

#include <mitkImageAccessByItk.h>

namespace mitk
{
  class ImageRegistrationMethod;

  struct ImageRegistrationMethodAccessFunctor
  {
    typedef ImageRegistrationMethodAccessFunctor Self;

    void operator()(const mitk::Image* img, ImageRegistrationMethod* method)
    {
      AccessByItk_1(img, AccessItkImage, method)
    }

    template < typename TPixel, unsigned int VImageDimension >
    void AccessItkImage( const itk::Image<TPixel, VImageDimension>* itkImage1,
                         ImageRegistrationMethod* method);
  };
}

#endif // MITKIMAGEREGISTRATIONMETHODACCESSFUNCTOR_H

