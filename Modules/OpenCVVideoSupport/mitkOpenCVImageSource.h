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
