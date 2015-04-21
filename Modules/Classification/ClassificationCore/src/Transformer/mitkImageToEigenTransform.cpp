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
  ImageType::Pointer current_mask;
  mitk::CastToItkImage(mask,current_mask);

  func_pointer = 0;

  unsigned int n_numSamples = 0;
  {
    auto mit = itk::ImageRegionConstIterator<ImageType>(current_mask, current_mask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
      if(mit.Value() > 0)
        n_numSamples++;
      ++mit;
    }
  }



  return n_numSamples;
}
