#pragma once

#include "mitkNumericConstants.h"
#include "mitkImageRegionAccessor.h"

namespace mitk {

MITKCORE_EXPORT ScalarType UnlockedSinglePixelAccess(itk::SmartPointer<Image> im, itk::Index<3> idx, int timestep, int component = 0);

// Doesn't work with regions, only whole data can be accessed
class MITKCORE_EXPORT ImageVtkAccessor : public ImageRegionAccessor
{
public:
  void setRegion(int dim, int min, int max) override {};
  void* getPixel(int index, int timestep) override { return nullptr; }
  void* getData(int timestep) override { return nullptr; };

  vtkImageData* getVtkImageData(int timestep = 0);

  ImageVtkAccessor(itk::SmartPointer<Image> image);
  ~ImageVtkAccessor();
protected:
  std::vector<vtkImageData*> m_TimeVolumes;
};

}
