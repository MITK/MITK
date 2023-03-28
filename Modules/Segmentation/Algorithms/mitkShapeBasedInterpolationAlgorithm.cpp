/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkShapeBasedInterpolationAlgorithm.h"
#include "mitkImageAccessByItk.h"
#include "mitkImageCast.h"
#include <mitkITKImageImport.h>

#include <itkFastChamferDistanceImageFilter.h>
#include <itkInvertIntensityImageFilter.h>
#include <itkIsoContourDistanceImageFilter.h>
#include <itkSubtractImageFilter.h>

#include <thread>

mitk::Image::Pointer mitk::ShapeBasedInterpolationAlgorithm::Interpolate(
  Image::ConstPointer lowerSlice,
  unsigned int lowerSliceIndex,
  Image::ConstPointer upperSlice,
  unsigned int upperSliceIndex,
  unsigned int requestedIndex,
  unsigned int /*sliceDimension*/, // commented variables are not used
  Image::Pointer resultImage,
  unsigned int /*timeStep*/,
  Image::ConstPointer /*referenceImage*/)
{
  auto lowerDistanceImage = this->ComputeDistanceMap(lowerSliceIndex, lowerSlice);
  auto upperDistanceImage = this->ComputeDistanceMap(upperSliceIndex, upperSlice);

  // calculate where the current slice is in comparison to the lower and upper neighboring slices
  float ratio = (float)(requestedIndex - lowerSliceIndex) / (float)(upperSliceIndex - lowerSliceIndex);
  AccessFixedDimensionByItk_3(resultImage, InterpolateIntermediateSlice, 2, upperDistanceImage, lowerDistanceImage, ratio);

  return resultImage;
}

mitk::Image::Pointer mitk::ShapeBasedInterpolationAlgorithm::ComputeDistanceMap(unsigned int sliceIndex, Image::ConstPointer slice)
{
  static const auto MAX_CACHE_SIZE = 2 * std::thread::hardware_concurrency();

  {
    std::lock_guard<std::mutex> lock(m_DistanceImageCacheMutex);

    if (0 != m_DistanceImageCache.count(sliceIndex))
      return m_DistanceImageCache[sliceIndex];

    if (MAX_CACHE_SIZE < m_DistanceImageCache.size())
      m_DistanceImageCache.clear();
  }

  mitk::Image::Pointer distanceImage;
  AccessFixedDimensionByItk_1(slice, ComputeDistanceMap, 2, distanceImage);

  std::lock_guard<std::mutex> lock(m_DistanceImageCacheMutex);

  m_DistanceImageCache[sliceIndex] = distanceImage;

  return distanceImage;
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ShapeBasedInterpolationAlgorithm::ComputeDistanceMap(const itk::Image<TPixel, VImageDimension> *binaryImage,
                                                                mitk::Image::Pointer &result)
{
  typedef itk::Image<TPixel, VImageDimension> DistanceFilterInputImageType;

  typedef itk::FastChamferDistanceImageFilter<DistanceFilterImageType, DistanceFilterImageType> DistanceFilterType;
  typedef itk::IsoContourDistanceImageFilter<DistanceFilterInputImageType, DistanceFilterImageType> IsoContourType;
  typedef itk::InvertIntensityImageFilter<DistanceFilterInputImageType> InvertIntensityImageFilterType;
  typedef itk::SubtractImageFilter<DistanceFilterImageType, DistanceFilterImageType> SubtractImageFilterType;

  typename DistanceFilterType::Pointer distanceFilter = DistanceFilterType::New();
  typename DistanceFilterType::Pointer distanceFilterInverted = DistanceFilterType::New();
  typename IsoContourType::Pointer isoContourFilter = IsoContourType::New();
  typename IsoContourType::Pointer isoContourFilterInverted = IsoContourType::New();
  typename InvertIntensityImageFilterType::Pointer invertFilter = InvertIntensityImageFilterType::New();
  typename SubtractImageFilterType::Pointer subtractImageFilter = SubtractImageFilterType::New();

  // arbitrary maximum distance
  int maximumDistance = 100;

  // this assumes the image contains only 1 and 0
  invertFilter->SetInput(binaryImage);
  invertFilter->SetMaximum(1);

  // do the processing on the image and the inverted image to get inside and outside distance
  isoContourFilter->SetInput(binaryImage);
  isoContourFilter->SetFarValue(maximumDistance + 1);
  isoContourFilter->SetLevelSetValue(0);

  isoContourFilterInverted->SetInput(invertFilter->GetOutput());
  isoContourFilterInverted->SetFarValue(maximumDistance + 1);
  isoContourFilterInverted->SetLevelSetValue(0);

  distanceFilter->SetInput(isoContourFilter->GetOutput());
  distanceFilter->SetMaximumDistance(maximumDistance);

  distanceFilterInverted->SetInput(isoContourFilterInverted->GetOutput());
  distanceFilterInverted->SetMaximumDistance(maximumDistance);

  // inside distance should be negative, outside distance positive
  subtractImageFilter->SetInput2(distanceFilter->GetOutput());
  subtractImageFilter->SetInput1(distanceFilterInverted->GetOutput());
  subtractImageFilter->Update();

  result = mitk::GrabItkImageMemory(subtractImageFilter->GetOutput());
}

template <typename TPixel, unsigned int VImageDimension>
void mitk::ShapeBasedInterpolationAlgorithm::InterpolateIntermediateSlice(itk::Image<TPixel, VImageDimension> *result,
                                                                          const mitk::Image::Pointer &lower,
                                                                          const mitk::Image::Pointer &upper,
                                                                          float ratio)
{
  typename DistanceFilterImageType::Pointer lowerITK = DistanceFilterImageType::New();
  typename DistanceFilterImageType::Pointer upperITK = DistanceFilterImageType::New();

  CastToItkImage(lower, lowerITK);
  CastToItkImage(upper, upperITK);

  itk::ImageRegionConstIteratorWithIndex<DistanceFilterImageType> lowerIter(lowerITK,
                                                                            lowerITK->GetLargestPossibleRegion());

  lowerIter.GoToBegin();

  if (!lowerITK->GetLargestPossibleRegion().IsInside(upperITK->GetLargestPossibleRegion()) ||
      !lowerITK->GetLargestPossibleRegion().IsInside(result->GetLargestPossibleRegion()))
  {
    // TODO Exception etc.
    MITK_ERROR << "The regions of the slices for the 2D interpolation are not equally sized!";
    return;
  }

  float weight[2] = {1.0f - ratio, ratio};
  while (!lowerIter.IsAtEnd())
  {
    typename DistanceFilterImageType::PixelType lowerPixelVal = lowerIter.Get();
    typename DistanceFilterImageType::PixelType upperPixelVal = upperITK->GetPixel(lowerIter.GetIndex());
    typename DistanceFilterImageType::PixelType intermediatePixelVal =
      (weight[0] * upperPixelVal + weight[1] * lowerPixelVal > 0 ? 0 : 1);

    result->SetPixel(lowerIter.GetIndex(), static_cast<TPixel>(intermediatePixelVal));

    ++lowerIter;
  }
}
