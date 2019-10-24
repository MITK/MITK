#pragma once

#include <itkSmartPointer.h>

#include "mitkChannelDescriptor.h"
#include "MitkCoreExports.h"

namespace mitk {

class Image;

class MITKCORE_EXPORT ImageRegionAccessor
{
public:
  ImageRegionAccessor(itk::SmartPointer<Image> image);
  virtual ~ImageRegionAccessor();

  virtual void setRegion(int dim, int min, int max);

  itk::SmartPointer<Image> getImage() const;

  bool overlap(ImageRegionAccessor& other);

  virtual void* getPixel(int index, int timestep = 0);
  virtual void* getPixel(const itk::Index<3> index, int timestep = 0);

  virtual void* getData(int timestep = 0);

  struct Range {
    int min;
    int max;
  };

protected:
  itk::SmartPointer<Image> m_Image;
  Range* m_Ranges;

private:
  ImageRegionAccessor& operator=(const ImageRegionAccessor&) = delete;
  ImageRegionAccessor(const ImageRegionAccessor&) = delete;

};

}

