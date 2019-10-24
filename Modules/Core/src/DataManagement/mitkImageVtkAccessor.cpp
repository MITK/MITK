#include "mitkImageVtkAccessor.h"

#include <vtkCharArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedShortArray.h>
#include <vtkIntArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkLongArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>

#include <vtkPointData.h>

#include "mitkImage.h"

namespace mitk {

ImageVtkAccessor::ImageVtkAccessor(itk::SmartPointer<Image> image) : ImageRegionAccessor(image) {
}

ImageVtkAccessor::~ImageVtkAccessor()
{
  for (const auto& volume : m_TimeVolumes) {
    volume->Delete();
  }
}

vtkImageData* ImageVtkAccessor::getVtkImageData(int timestep)
{
  return m_Image->GetVolumeData(timestep)->getVtkImageData(m_Image);
}

ScalarType UnlockedSinglePixelAccess(itk::SmartPointer<Image> im, itk::Index<3> idx, int timestep, int component)
{
  ImageVtkAccessor accessor(im);
  vtkImageData* data = accessor.getVtkImageData(timestep);
  unsigned int* dims = im->GetDimensions();
  int globalOffset = 0;
  globalOffset += idx[2] * dims[0] * dims[1];
  globalOffset += idx[0] + idx[1] * dims[0];

  double* scalaras = data->GetPointData()->GetScalars()->GetTuple(globalOffset);

  return scalaras[component];
}

}