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

// MITK includes
#include <mitkImageToEigenTransform.h>
#include <mitkImageStatisticsHolder.h>




unsigned int mitk::ImageToEigenTransform::countIf(const mitk::Image::Pointer & mask, bool (*func_pointer) (double) )
{
  UCharImageType::Pointer current_mask;
  mitk::CastToItkImage(mask,current_mask);

  unsigned int n_numSamples = 0;
  {
    auto mit = itk::ImageRegionConstIterator<UCharImageType>(current_mask, current_mask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
      if((*func_pointer)(mit.Value()))
        n_numSamples++;
      ++mit;
    }
  }
  return n_numSamples;
}
