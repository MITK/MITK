/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
    void AccessItkImage( itk::Image<TPixel, VImageDimension>* itkImage1,
                         ImageRegistrationMethod* method);
  };
}

#endif // MITKIMAGEREGISTRATIONMETHODACCESSFUNCTOR_H

