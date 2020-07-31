/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkIntensityQuantifier.h>

// STD
#include <numeric>

// ITK
#include <itkImageRegionConstIterator.h>

// MITK
#include <mitkImageCast.h>
#include <mitkImageAccessByItk.h>

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateImageMinMax(const itk::Image<TPixel, VImageDimension>* itkImage, double &minimum, double &maximum)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;

  minimum = std::numeric_limits<TPixel>::max();
  maximum = std::numeric_limits<TPixel>::lowest();

  itk::ImageRegionConstIterator<ImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());

  while (!iter.IsAtEnd())
  {
    minimum = std::min<TPixel>(minimum, iter.Get());
    maximum = std::max<TPixel>(maximum, iter.Get());
    ++iter;
  }
}

template<typename TPixel, unsigned int VImageDimension>
static void
CalculateImageRegionMinMax(const itk::Image<TPixel, VImageDimension>* itkImage, const mitk::Image* mask, double &minimum, double &maximum)
{
  typedef itk::Image<TPixel, VImageDimension> ImageType;
  typedef itk::Image<int, VImageDimension> MaskType;

  typename MaskType::Pointer itkMask = MaskType::New();
  mitk::CastToItkImage(mask, itkMask);

  minimum = std::numeric_limits<TPixel>::max();
  maximum = std::numeric_limits<TPixel>::lowest();

  itk::ImageRegionConstIterator<ImageType> iter(itkImage, itkImage->GetLargestPossibleRegion());
  itk::ImageRegionConstIterator<MaskType> maskIter(itkMask, itkMask->GetLargestPossibleRegion());

  while (!iter.IsAtEnd())
  {
    if (maskIter.Get() > 0)
    {
      minimum = std::min<TPixel>(minimum, iter.Get());
      maximum = std::max<TPixel>(maximum, iter.Get());
    }
    ++iter;
    ++maskIter;
  }
}

mitk::IntensityQuantifier::IntensityQuantifier() :
      m_Initialized(false),
      m_Bins(0),
      m_Binsize(0),
      m_Minimum(0),
      m_Maximum(0)
{}

void mitk::IntensityQuantifier::InitializeByMinimumMaximum(double minimum, double maximum, unsigned int bins) {
  m_Minimum = minimum;
  m_Maximum = maximum;
  m_Bins = bins;
  m_Binsize = (maximum - minimum) / bins;
  m_Initialized = true;
}

void mitk::IntensityQuantifier::InitializeByBinsizeAndBins(double minimum, unsigned int bins, double binsize) {
  m_Minimum = minimum;
  m_Maximum = minimum + bins*binsize;
  m_Bins = bins;
  m_Binsize = binsize;
  m_Initialized = true;
}

void mitk::IntensityQuantifier::InitializeByBinsizeAndMaximum(double minimum, double maximum, double binsize) {
  m_Minimum = minimum;
  m_Bins = std::ceil((maximum - minimum) / binsize);
  m_Maximum = minimum + m_Bins*binsize;
  m_Binsize = binsize;
  m_Initialized = true;
}

void mitk::IntensityQuantifier::InitializeByImage(const Image* image, unsigned int bins) {
  double minimum, maximum;
  AccessByItk_2(image, CalculateImageMinMax, minimum, maximum);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageAndMinimum(const Image* image, double minimum, unsigned int bins) {
  double tmp, maximum;
  AccessByItk_2(image, CalculateImageMinMax, tmp, maximum);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageAndMaximum(const Image* image, double maximum, unsigned int bins) {
  double minimum, tmp;
  AccessByItk_2(image, CalculateImageMinMax, minimum, tmp);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageRegion(const Image* image, const Image* mask, unsigned int bins) {
  double minimum, maximum;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, minimum, maximum);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageRegionAndMinimum(const Image* image, const Image* mask, double minimum, unsigned int bins) {
  double tmp, maximum;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, tmp, maximum);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageRegionAndMaximum(const Image* image, const Image* mask, double maximum, unsigned int bins) {
  double minimum, tmp;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, minimum, tmp);
  InitializeByMinimumMaximum(minimum, maximum, bins);
}

void mitk::IntensityQuantifier::InitializeByImageAndBinsize(const Image* image, double binsize) {
  double minimum, maximum;
  AccessByItk_2(image, CalculateImageMinMax, minimum, maximum);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

void mitk::IntensityQuantifier::InitializeByImageAndBinsizeAndMinimum(const Image* image, double minimum, double binsize) {
  double tmp, maximum;
  AccessByItk_2(image, CalculateImageMinMax, tmp, maximum);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

void mitk::IntensityQuantifier::InitializeByImageAndBinsizeAndMaximum(const Image* image, double maximum, double binsize) {
  double minimum, tmp;
  AccessByItk_2(image, CalculateImageMinMax, minimum, tmp);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

void mitk::IntensityQuantifier::InitializeByImageRegionAndBinsize(const Image* image, const Image* mask, double binsize) {
  double minimum, maximum;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, minimum, maximum);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

void mitk::IntensityQuantifier::InitializeByImageRegionAndBinsizeAndMinimum(const Image* image, const Image* mask, double minimum, double binsize) {
  double tmp, maximum;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, tmp, maximum);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

void mitk::IntensityQuantifier::InitializeByImageRegionAndBinsizeAndMaximum(const Image* image, const Image* mask, double maximum, double binsize) {
  double minimum, tmp;
  AccessByItk_3(image, CalculateImageRegionMinMax, mask, minimum, tmp);
  InitializeByBinsizeAndMaximum(minimum, maximum, binsize);
}

unsigned int mitk::IntensityQuantifier::IntensityToIndex(double intensity)
{
  double index = std::floor((intensity - m_Minimum) / m_Binsize);
  return std::max<double>(0, std::min<double>(index, m_Bins-1));
}

double mitk::IntensityQuantifier::IndexToMinimumIntensity(unsigned int index)
{
  return index*m_Binsize + m_Minimum;
}

double mitk::IntensityQuantifier::IndexToMeanIntensity(unsigned int index)
{
  return (index + 0.5) * m_Binsize + m_Minimum;
}

double mitk::IntensityQuantifier::IndexToMaximumIntensity(unsigned int index)
{
  return (index + 1) * m_Binsize + m_Minimum;
}
