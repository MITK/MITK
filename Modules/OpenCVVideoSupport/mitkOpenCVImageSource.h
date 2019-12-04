/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef MITKOPENCVIMAGESOURCE_H
#define MITKOPENCVIMAGESOURCE_H

#include <mitkCommon.h>

#include "opencv2/core.hpp"

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
    mitkClassMacroItkParent( OpenCVImageSource, itk::Object );

    ///
    /// \return a image as opencv 2 Mat
    ///
    virtual cv::Mat GetImage() = 0;
  };
}

#endif // MITKOPENCVIMAGESOURCE_H
