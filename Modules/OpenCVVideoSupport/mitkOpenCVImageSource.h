#ifndef MITKOPENCVIMAGESOURCE_H
#define MITKOPENCVIMAGESOURCE_H

#include <cv.h>

namespace mitk
{
  struct OpenCVImageSource
  {
    virtual const IplImage * GetImage() = 0;
    virtual void FetchFrame() = 0;
  };
}

#endif // MITKOPENCVIMAGESOURCE_H
