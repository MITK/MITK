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

//#include <mitkCostFunctionBase.h>

//Itk Iterators
#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>

//Set Functions
template <typename TPixel, unsigned int VImageDimension>
void mitk::CostFunctionBase<TPixel, VImageDimension>::SetRegion(std::pair<RegionType, int> regionPair)
{
  m_RegionPair = regionPair;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::CostFunctionBase<TPixel, VImageDimension>::SetImage(itk::Image<TPixel, VImageDimension> * image)
{
  m_Image = image;
}

//Other Functions
template < typename TPixel, unsigned int VImageDimension >
int mitk::CostFunctionBase<TPixel, VImageDimension>::RegionIsFullEnough ()
{
  //Calculate the volume of the region cuboid
  itk::Size<3> size = m_RegionPair.first.GetSize();
  double volume = size[0] * size[1] * size[2];

  typedef itk::Image< int, VImageDimension > IntegerImageType;
  itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, m_RegionPair.first);

  double voxelsWithRegionValue(1); //Because the possible new voxel hasn't been set to the region value yet
  double voxelsWithValueZero(0);

  for(it_region.GoToBegin(); !it_region.IsAtEnd(); ++it_region)
  {
    if (it_region.Value() == m_RegionPair.second)
    {
      voxelsWithRegionValue++;
    }
    if (it_region.Value() == 0)
    {
      voxelsWithValueZero++;
    }
  }

  double measurement = voxelsWithRegionValue / (volume - voxelsWithValueZero);

  if (measurement > 0.3)
  {
    return 1;
  }
  return 0;
}

template < typename TPixel, unsigned int VImageDimension >
int mitk::CostFunctionBase<TPixel, VImageDimension>::SidesAreNotTooLong ()
{
  itk::Size<3> size = m_RegionPair.first.GetSize();

  if (size[0] < size[1] + size[2] + 1 && size[1] < size[0] + size[2] + 1 && size[2] < size[0] + size[1] + 1)
  {
    return 1;
  }
  return 0;
}

template < typename TPixel, unsigned int VImageDimension >
int mitk::CostFunctionBase<TPixel, VImageDimension>::COMLiesWithinParcel()
{
  std::pair<RegionType, int> chosenRegion = m_RegionPair;

  typedef itk::Image< int, VImageDimension > IntegerImageType;
  itk::ImageRegionIterator<IntegerImageType> it_region(m_Image, chosenRegion.first);
  std::vector<double> centerOfMass;
  centerOfMass = this->GetCenterOfMass();
  typename IntegerImageType::IndexType indexCenterOfMass;
  indexCenterOfMass.SetElement(0, centerOfMass[0] + 0.5);
  indexCenterOfMass.SetElement(1, centerOfMass[1] + 0.5);
  indexCenterOfMass.SetElement(2, centerOfMass[2] + 0.5);
  it_region.SetIndex(indexCenterOfMass);
  int value = it_region.Value();

  if (it_region.Value() == chosenRegion.second || it_region.Value() == 0)
  {
    return 1;
  }
  return 0;
}

template < typename TPixel, unsigned int VImageDimension >
int mitk::CostFunctionBase<TPixel, VImageDimension>::CalculateCost()
{
  int costFunctionValue(0);
  std::vector<int> costVector;
  costVector.push_back(this->RegionIsFullEnough());
  costVector.push_back(this->SidesAreNotTooLong());
  costVector.push_back(this->COMLiesWithinParcel());

  m_weight.clear();
  m_weight.push_back(1); //weight for RegionIsFullEnough
  m_weight.push_back(1); //weight for SidesAreNotTooLong
  m_weight.push_back(0); //weight for COMLiesWithinParcel

  //Vector multiplication
  for (int i = 0; i < costVector.size(); i++)
  {
    costFunctionValue += costVector[i]*m_weight[i];
  }

  return costFunctionValue;
}

template <typename TPixel, unsigned int VImageDimension>
std::vector<double> mitk::CostFunctionBase<TPixel, VImageDimension>::GetCenterOfMass()
{
  //Count all tagged voxels in this region
  itk::ImageRegionIterator<ImageType> it_region(m_Image, m_RegionPair.first);

  typedef itk::Image< TPixel, VImageDimension > ImageType;
  int currentSizeOfRegion (0);
  std::vector<typename ImageType::IndexType> indexVoxel;
  std::vector<double> centerOfMass;
  double xValue(0);
  double yValue(0);
  double zValue(0);

  for (it_region.GoToBegin(); !it_region.IsAtEnd(); ++it_region)
  {
    if(it_region.Value() == m_RegionPair.second)
    {
      indexVoxel.push_back(it_region.GetIndex());
      currentSizeOfRegion++;
    }
  }

  xValue = 0;
  yValue = 0;
  zValue = 0;

  for (int i = 0; i < currentSizeOfRegion; i++)
  {
    xValue += indexVoxel[i][0];
    yValue += indexVoxel[i][1];
    zValue += indexVoxel[i][2];
  }

  centerOfMass.push_back(xValue/currentSizeOfRegion);
  centerOfMass.push_back(yValue/currentSizeOfRegion);
  centerOfMass.push_back(zValue/currentSizeOfRegion);

  return centerOfMass;
}

template <typename TPixel, unsigned int VImageDimension>
int mitk::CostFunctionBase<TPixel, VImageDimension>::MaximalValue()
{
  int maximalValue(0);
  for (int i = 0; i < m_weight.size(); i++)
  {
    maximalValue += m_weight[i];
  }
  return maximalValue;
}
