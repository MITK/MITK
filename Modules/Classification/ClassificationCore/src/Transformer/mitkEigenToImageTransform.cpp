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
#include <mitkEigentoImageTransform.h>
#include <mitkImageCast.h>
#include <itkImageRegionConstIterator.h>
#include <itkImageRegionIterator.h>

mitk::Image::Pointer mitk::EigenToImageTransform::transform(const VectorType & matrix, const mitk::Image::Pointer & mask)
{
  UCharImageType::Pointer itkMask;
  mitk::CastToItkImage(mask,itkMask);

  unsigned int n_numSamples = 0;
  {
    auto mit = itk::ImageRegionConstIterator<UCharImageType>(itkMask, itkMask->GetLargestPossibleRegion());
    while (!mit.IsAtEnd())
    {
      if(mit.Value() > 0) n_numSamples++;
      ++mit;
    }
  }

  if(n_numSamples != matrix.rows())
    MITK_ERROR << "Number of samples in matrix and number of points under the masks is not the same!";


  mitk::Image::Pointer output = mask->Clone();

  DoubleImageType::Pointer itkImg;
  mitk::CastToItkImage(output,itkImg);

  auto mit = itk::ImageRegionConstIterator<UCharImageType>(itkMask, itkMask->GetLargestPossibleRegion());
  auto oit = itk::ImageRegionIterator<DoubleImageType>(itkImg, itkImg->GetLargestPossibleRegion());

  unsigned int current_row = 0;
  while(!mit.IsAtEnd())
  {
    if(mit.Value() > 0)
      oit.Set(matrix(current_row++,0));
    else
      oit.Set(0.0);
    ++mit;
    ++oit;
  }

  mitk::CastToMitkImage(itkImg,output);
  return output;
}

mitk::Image::Pointer mitk::EigenToImageTransform::operator()(const VectorType & matrix, const mitk::Image::Pointer & mask)
{
  return transform(matrix,mask);
}
