/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLabelStatisticsImageFilter.h>
#include <mitkStatisticsImageFilter.h>

#include <mitkNumericConstants.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

/**
 * \brief Tests the median of both statistics filters across work units and
 *        stream divisions.
 *
 * The per-value tests of the accumulator itself live in
 * mitkMedianAccumulatorTest. What is exercised here is the path the filters
 * add on top: one accumulator per work unit per stream division, merged into
 * a single result. Both filters are driven with more than one of each, so a
 * regression in a merge changes the reported median.
 */
class mitkStatisticsFilterMedianTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkStatisticsFilterMedianTestSuite);
  MITK_TEST(Unmasked_DenseCounting);
  MITK_TEST(Unmasked_ValueCollection);
  MITK_TEST(Unmasked_DegradesToValueCollection);
  MITK_TEST(Masked_DenseCounting);
  MITK_TEST(Masked_ValueCollection);
  MITK_TEST(Masked_BackgroundHasNoHistogramStatistics);
  CPPUNIT_TEST_SUITE_END();

private:
  // 4000 voxels across 250 slices, so that the work unit and stream division
  // counts below actually split the image
  static constexpr itk::SizeValueType SizeX = 4;
  static constexpr itk::SizeValueType SizeY = 4;
  static constexpr itk::SizeValueType SizeZ = 250;
  static constexpr itk::SizeValueType VoxelCount = SizeX * SizeY * SizeZ;

  static constexpr unsigned int WorkUnits = 4;
  static constexpr unsigned int StreamDivisions = 3;

  // The image is a ramp of 0, 1, ... VoxelCount - 1 in iteration order, so
  // every value is distinct and the median of any contiguous run of it is the
  // mean of the two middle indices.
  static constexpr double RampMedian = 0.5 * ((VoxelCount / 2 - 1) + VoxelCount / 2);

  // The first two runs of 1024 voxels carry a label, the rest is background.
  static constexpr itk::SizeValueType LabelSize = 1024;
  static constexpr mitk::Label::PixelType FirstLabel = 1;
  static constexpr mitk::Label::PixelType SecondLabel = 2;
  static constexpr mitk::Label::PixelType Background = 0;

  static constexpr double FirstLabelMedian = 0.5 * ((LabelSize / 2 - 1) + LabelSize / 2);
  static constexpr double SecondLabelMedian = LabelSize + FirstLabelMedian;

  template <typename TPixel>
  using ImageType = itk::Image<TPixel, 3>;

  using LabelImageType = ImageType<mitk::Label::PixelType>;

  // itk::ImageSink keeps SetNumberOfStreamDivisions protected and leaves it to
  // the concrete filter to expose it. Neither statistics filter does, so the
  // test promotes it rather than widening their interface for one caller.
  template <typename TImage>
  class StreamedStatisticsFilter : public mitk::StatisticsImageFilter<TImage>
  {
  public:
    using Self = StreamedStatisticsFilter;
    using Superclass = mitk::StatisticsImageFilter<TImage>;
    using Pointer = itk::SmartPointer<Self>;

    itkNewMacro(Self);

    using Superclass::SetNumberOfStreamDivisions;

  protected:
    StreamedStatisticsFilter() = default;
  };

  template <typename TImage>
  class StreamedLabelStatisticsFilter : public mitk::LabelStatisticsImageFilter<TImage>
  {
  public:
    using Self = StreamedLabelStatisticsFilter;
    using Superclass = mitk::LabelStatisticsImageFilter<TImage>;
    using Pointer = itk::SmartPointer<Self>;

    itkNewMacro(Self);

    using Superclass::SetNumberOfStreamDivisions;

  protected:
    StreamedLabelStatisticsFilter() = default;
  };

  template <typename TPixel>
  static typename ImageType<TPixel>::RegionType CreateRegion()
  {
    typename ImageType<TPixel>::SizeType size;
    size[0] = SizeX;
    size[1] = SizeY;
    size[2] = SizeZ;

    typename ImageType<TPixel>::IndexType start;
    start.Fill(0);

    return typename ImageType<TPixel>::RegionType(start, size);
  }

  template <typename TPixel>
  static typename ImageType<TPixel>::Pointer CreateRampImage()
  {
    const auto region = CreateRegion<TPixel>();

    auto image = ImageType<TPixel>::New();
    image->SetRegions(region);
    image->Allocate();

    itk::SizeValueType index = 0;

    for (itk::ImageRegionIterator<ImageType<TPixel>> it(image, region); !it.IsAtEnd(); ++it, ++index)
      it.Set(static_cast<TPixel>(index));

    return image;
  }

  static LabelImageType::Pointer CreateLabelImage()
  {
    const auto region = CreateRegion<mitk::Label::PixelType>();

    auto image = LabelImageType::New();
    image->SetRegions(region);
    image->Allocate();

    itk::SizeValueType index = 0;

    for (itk::ImageRegionIterator<LabelImageType> it(image, region); !it.IsAtEnd(); ++it, ++index)
    {
      if (index < LabelSize)
      {
        it.Set(FirstLabel);
      }
      else if (index < 2 * LabelSize)
      {
        it.Set(SecondLabel);
      }
      else
      {
        it.Set(Background);
      }
    }

    return image;
  }

  template <typename TPixel>
  static double UnmaskedMedian(double lowerBound, double upperBound)
  {
    using FilterType = StreamedStatisticsFilter<ImageType<TPixel>>;

    auto filter = FilterType::New();
    filter->SetInput(CreateRampImage<TPixel>());
    filter->SetHistogramParameters(100, lowerBound, upperBound);
    filter->SetNumberOfWorkUnits(WorkUnits);
    filter->SetNumberOfStreamDivisions(StreamDivisions);
    filter->Update();

    return filter->GetMedian();
  }

  template <typename TPixel>
  static typename StreamedLabelStatisticsFilter<ImageType<TPixel>>::Pointer RunMasked()
  {
    using FilterType = StreamedLabelStatisticsFilter<ImageType<TPixel>>;

    std::unordered_map<mitk::Label::PixelType, unsigned int> sizes;
    sizes[FirstLabel] = 100;
    sizes[SecondLabel] = 100;

    std::unordered_map<mitk::Label::PixelType, typename FilterType::RealType> lowerBounds;
    lowerBounds[FirstLabel] = 0;
    lowerBounds[SecondLabel] = LabelSize;

    std::unordered_map<mitk::Label::PixelType, typename FilterType::RealType> upperBounds;
    upperBounds[FirstLabel] = LabelSize - 1;
    upperBounds[SecondLabel] = 2 * LabelSize - 1;

    auto filter = FilterType::New();
    filter->SetInput(CreateRampImage<TPixel>());
    filter->SetLabelInput(CreateLabelImage());
    filter->SetHistogramParameters(sizes, lowerBounds, upperBounds);
    filter->SetNumberOfWorkUnits(WorkUnits);
    filter->SetNumberOfStreamDivisions(StreamDivisions);
    filter->Update();

    return filter;
  }

public:
  void Unmasked_DenseCounting()
  {
    // an integral pixel type whose declared range is counted per value
    CPPUNIT_ASSERT_DOUBLES_EQUAL(RampMedian, UnmaskedMedian<short>(0, VoxelCount - 1), mitk::eps);
  }

  void Unmasked_ValueCollection()
  {
    // a floating point pixel type, which always collects the values instead
    CPPUNIT_ASSERT_DOUBLES_EQUAL(RampMedian, UnmaskedMedian<float>(0, VoxelCount - 1), mitk::eps);
  }

  void Unmasked_DegradesToValueCollection()
  {
    // histogram bounds narrower than the data, so every accumulator falls back
    // to collecting values before it is merged
    CPPUNIT_ASSERT_DOUBLES_EQUAL(RampMedian, UnmaskedMedian<short>(0, 100), mitk::eps);
  }

  void Masked_DenseCounting()
  {
    auto filter = RunMasked<short>();

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(LabelSize), filter->GetCount(FirstLabel));
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(LabelSize), filter->GetCount(SecondLabel));
    CPPUNIT_ASSERT_DOUBLES_EQUAL(FirstLabelMedian, filter->GetMedian(FirstLabel), mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SecondLabelMedian, filter->GetMedian(SecondLabel), mitk::eps);
  }

  void Masked_ValueCollection()
  {
    auto filter = RunMasked<float>();

    CPPUNIT_ASSERT_DOUBLES_EQUAL(FirstLabelMedian, filter->GetMedian(FirstLabel), mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(SecondLabelMedian, filter->GetMedian(SecondLabel), mitk::eps);
  }

  void Masked_BackgroundHasNoHistogramStatistics()
  {
    // No histogram parameters were passed for the background, so it must not
    // have accumulated one voxel of the far larger region it covers.
    auto filter = RunMasked<float>();

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(VoxelCount - 2 * LabelSize), filter->GetCount(Background));
    CPPUNIT_ASSERT_THROW(filter->GetMedian(Background), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkStatisticsFilterMedian)
