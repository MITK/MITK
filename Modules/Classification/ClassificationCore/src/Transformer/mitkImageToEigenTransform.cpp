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
#include <mitkImageCast.h>
#include <itkImageRegionConstIterator.h>
#include <mitkImageStatisticsHolder.h>

mitk::ImageToEigenTransform::MatrixType mitk::ImageToEigenTransform::transform(const mitk::Image::Pointer & image,
                                                                               const mitk::Image::Pointer & mask)
{
  UCharImageType::Pointer current_mask;
  mitk::CastToItkImage(mask,current_mask);

  DoubleImageType::Pointer current_input;
  mitk::CastToItkImage(image,current_input);

  unsigned int n_numSamples = countIf(mask, [](double c){ return c > 0;});

  MatrixType vector(n_numSamples,1);

  auto mit = itk::ImageRegionConstIterator<UCharImageType>(current_mask, current_mask->GetLargestPossibleRegion());
  auto iit = itk::ImageRegionConstIterator<DoubleImageType>(current_input,current_input->GetLargestPossibleRegion());
  unsigned int current_row = 0;
  while (!mit.IsAtEnd()) {
    if(mit.Value() > 0)
      vector(current_row++) = iit.Value();
    ++mit;
    ++iit;
  }

  return vector;
}


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
