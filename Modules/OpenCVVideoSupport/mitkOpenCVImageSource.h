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
#ifndef MITKOPENCVIMAGESOURCE_H
#define MITKOPENCVIMAGESOURCE_H

#include <cv.h>
#include <mitkCommon.h>

namespace mitk
{
  ///
  /// interface for a class providing opencv images
  ///
  class OpenCVImageSource: virtual public itk::Object
  {
  public:
    ///
    /// provide smart pointer defs
    ///
    mitkClassMacro( OpenCVImageSource, itk::Object );

    ///
    /// \return a image as opencv 2 Mat
    ///
    virtual cv::Mat GetImage() = 0;
  };
}

#endif // MITKOPENCVIMAGESOURCE_H
