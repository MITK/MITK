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

#include "mitkMaskCleaningOperation.h"

#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageCast.h>

#include <itkImage.h>
#include <itkBinaryFunctorImageFilter.h>
#include <itkImageRegionConstIterator.h>

#include "itkCastImageFilter.h"

namespace mitk
{
  namespace Functor
  {
    template< class TInput, class TOutput>
    class RangeBasedMasking
    {
    public:
      RangeBasedMasking() {};
      ~RangeBasedMasking() {};
      bool operator!=(const RangeBasedMasking &) const
      {
        return false;
      }
      bool operator==(const RangeBasedMasking & other) const
      {
        return !(*this != other);
      }
      inline TOutput operator()(const TInput & A, const TOutput & B) const
      {
        unsigned short erg = B;
        if (lowerLimitOn && (A < lowerLimit))
        {
          erg = 0;
        }
        if (upperLimitOn && (upperLimit < A))
        {
          erg = 0;
        }
        return erg;
      }

      bool lowerLimitOn = false;
      bool upperLimitOn = false;
      double lowerLimit = 0;
      double upperLimit = 1;
    };
  }
}



template<typename TPixel, unsigned int VImageDimension>
static void ExecuteRangeBasedMasking(itk::Image<TPixel, VImageDimension>* image, mitk::Image::Pointer & mask, bool lowerLimitOn, double lowerLimit, bool upperLimitOn, double upperLimit, mitk::Image::Pointer & resultImage)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskImageType;
  typedef itk::BinaryFunctorImageFilter< ImageType, MaskImageType, MaskImageType, mitk::Functor::RangeBasedMasking<TPixel, unsigned short> > DefaultFilterType;

  typename MaskImageType::Pointer itkMask = MaskImageType::New();
  mitk::CastToItkImage(mask.GetPointer(), itkMask);

  typename DefaultFilterType::Pointer filter = DefaultFilterType::New();
  filter->SetInput1(image);
  filter->SetInput2(itkMask);
  filter->GetFunctor().lowerLimitOn = lowerLimitOn;
  filter->GetFunctor().upperLimitOn = upperLimitOn;
  filter->GetFunctor().lowerLimit = lowerLimit;
  filter->GetFunctor().upperLimit = upperLimit;
  filter->Update();

  CastToMitkImage(filter->GetOutput(), resultImage);
}

mitk::Image::Pointer mitk::MaskCleaningOperation::RangeBasedMasking(Image::Pointer & image, Image::Pointer & mask, bool lowerLimitOn, double lowerLimit, bool upperLimitOn, double upperLimit)
{
  Image::Pointer resultImage;
  AccessByItk_n(image, ExecuteRangeBasedMasking, (mask, lowerLimitOn, lowerLimit, upperLimitOn, upperLimit, resultImage));
  return resultImage;
}


template<typename TPixel, unsigned int VImageDimension>
static void CalculateMeanAndStd(itk::Image<TPixel, VImageDimension>* image, mitk::Image::Pointer & mask, double &mean, double &std)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<unsigned short, VImageDimension> MaskImageType;

  typename MaskImageType::Pointer itkMask = MaskImageType::New();
  mitk::CastToItkImage(mask.GetPointer(), itkMask);

  itk::ImageRegionConstIterator<ImageType> image_iter(image, image->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<MaskImageType> mask_iter(itkMask, itkMask->GetLargestPossibleRegion());

  unsigned int number_of_voxels = 0;
  while (!image_iter.IsAtEnd())
  {
    if (mask_iter.Get() > 0)
    {
      mean += image_iter.Get();
      std += image_iter.Get() * image_iter.Get();
      ++number_of_voxels;
    }

    ++image_iter;
    ++mask_iter;
  }
  mean /= number_of_voxels;
  std /= number_of_voxels;
  std -= mean;
  std = sqrt(std);
}


mitk::Image::Pointer mitk::MaskCleaningOperation::MaskOutlierFiltering(Image::Pointer & image, Image::Pointer & mask)
{
  Image::Pointer resultImage;
  double mean = 0;
  double std = 0;
  AccessByItk_n(image, CalculateMeanAndStd, (mask, mean, std));
  return MaskCleaningOperation::RangeBasedMasking(image, mask, true, mean - 3 * std, true, mean + 3 * std);

}