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

//#include <mitkRegionVoxelCounter.h>

//Itk Iterators
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>

//Set Functions
template <typename TPixel, unsigned int VImageDimension>
void mitk::RegionVoxelCounter<TPixel, VImageDimension>::SetRegion(typename ImageType::RegionType region)
{
  m_Region = region;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::RegionVoxelCounter<TPixel, VImageDimension>::SetImage(itk::Image<TPixel, VImageDimension> * image)
{
  m_Image = image;
}

//Other Functions
template <typename TPixel, unsigned int VImageDimension>
int mitk::RegionVoxelCounter<TPixel, VImageDimension>::VoxelWithValue(int value)
{
  itk::ImageRegionIterator<ImageType> it_region(m_Image, m_Region);
  int counter(0);

  for (it_region.GoToBegin(); !it_region.IsAtEnd(); ++it_region)
  {
    if (it_region.Value() == value) //Found Voxel with chosen value
    {
      counter++;
    }
  }
  return counter;
}

template <typename TPixel, unsigned int VImageDimension>
double mitk::RegionVoxelCounter<TPixel, VImageDimension>::PercentageVoxelWithValueZeroExcluded(int value)
{
  itk::Size<3> regionSize = m_Region.GetSize();
  double volume = regionSize[0] * regionSize[1] * regionSize[2];

  double measurement = this->VoxelWithValue(value) / (volume - this->VoxelWithValue(0));
  return measurement;
}