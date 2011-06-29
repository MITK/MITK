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

#ifndef MITKPYRAMIDALREGISTRATIONMETHODACCESSFUNCTOR_H
#define MITKPYRAMIDALREGISTRATIONMETHODACCESSFUNCTOR_H

#include <mitkImageAccessByItk.h>

namespace mitk
{
  class PyramidalRegistrationMethod;

  struct PyramidalRegistrationMethodAccessFunctor
  {
    typedef PyramidalRegistrationMethodAccessFunctor Self;

    void operator()(const mitk::Image* img, PyramidalRegistrationMethod* method)
    {
      AccessByItk_1(img, AccessItkImage, method)
    }

    template < typename TPixel, unsigned int VImageDimension >
    void AccessItkImage( itk::Image<TPixel, VImageDimension>* itkImage1,
                         PyramidalRegistrationMethod* method);
  };
}

#endif // MITKPYRAMIDALREGISTRATIONMETHODACCESSFUNCTOR_H

