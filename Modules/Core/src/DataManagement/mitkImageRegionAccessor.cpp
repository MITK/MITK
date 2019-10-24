#include "mitkImageRegionAccessor.h"

#include "mitkImage.h"

namespace mitk {

ImageRegionAccessor::ImageRegionAccessor(itk::SmartPointer<Image> image)
{
  m_Image = image;

  int dim = m_Image->GetDimension();
  m_Ranges = new Range[dim];

  for (int i = 0; i < dim; i++) {
    m_Ranges[i].min = 0;
    m_Ranges[i].max = m_Image->GetDimension(i);
  }
}

ImageRegionAccessor::~ImageRegionAccessor()
{
  delete[] m_Ranges;
}

void ImageRegionAccessor::setRegion(int dim, int min, int max)
{
  assert(max >= min);
  assert(dim >= 0 && dim < m_Image->GetDimension());

  m_Ranges[dim].min = min;
  m_Ranges[dim].max = max;
}

bool ImageRegionAccessor::overlap(ImageRegionAccessor& other)
{
  assert(other.m_Image == m_Image);

  bool overlap = true;
  for (unsigned int i = 0; i < m_Image->GetDimension(); i++) {
    overlap &= other.m_Ranges[i].min <= m_Ranges[i].max && m_Ranges[i].min <= other.m_Ranges[i].max;
  }
  return overlap;
}

Image::Pointer ImageRegionAccessor::getImage() const
{
  return m_Image;
}

void* ImageRegionAccessor::getPixel(int index, int timestep) {
  int dims = m_Image->GetDimension();
  int globalOffset = 0;
  for (int i = 0; i < dims; i++) {
    globalOffset += (m_Ranges[i].min + index / (m_Ranges[i].max - m_Ranges[i].min)) * m_Image->m_OffsetTable[i] * m_Image->GetChannelDescriptor().GetPixelType().GetSize();
    index %= m_Ranges[i].max - m_Ranges[i].min;
  }
  return (char*)getData(timestep) + globalOffset;
}

void* ImageRegionAccessor::getPixel(const itk::Index<3> index, int timestep) {
  unsigned int* dims = m_Image->GetDimensions();
  int globalOffset = 0;
  globalOffset += index[2] * dims[0] * dims[1];
  globalOffset += index[0] + index[1] * dims[0];
  return ((char*)getData(timestep) + globalOffset);
}

void* ImageRegionAccessor::getData(int timestep)
{
  return m_Image->GetVolumeData(timestep)->GetData();
}

}

