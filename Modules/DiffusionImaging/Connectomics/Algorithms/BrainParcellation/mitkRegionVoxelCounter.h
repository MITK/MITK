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

#ifndef mitkRegionVoxelCounter_h
#define mitkRegionVoxelCounter_h

#include<itkImage.h>

#include "mitkCommon.h"
#include "MitkConnectomicsExports.h"

namespace mitk
{

  template <typename TPixel, unsigned int VImageDimension>
  class RegionVoxelCounter
  {
  public:
    typedef itk::Image< TPixel, VImageDimension > ImageType;

    void SetRegion(typename ImageType::RegionType);
    void SetImage(itk::Image<TPixel, VImageDimension> *);

    /** \brief Counts all voxels with the chosen value in the set region*/
    int VoxelWithValue(int value);
    /** \brief Gives back the percentage of the number of voxels with the chosen value in comparison to the number of voxels with other values (except zero)*/
    double PercentageVoxelWithValueZeroExcluded(int value);

  private:
    typename ImageType::RegionType m_Region;
    ImageType * m_Image;
  };

}

#include "mitkRegionVoxelCounter.cpp"

#endif /* mitkRegionVoxelCounter_h */