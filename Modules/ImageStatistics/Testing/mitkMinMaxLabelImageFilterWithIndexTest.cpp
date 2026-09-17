/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>

#include <vector>

namespace
{
  using ImageType = itk::Image<float, 3>;
  using LabelImageType = itk::Image<unsigned short, 3>;
  using FilterType = itk::MinMaxLabelImageFilterWithIndex<ImageType, LabelImageType>;
  using UCharImageType = itk::Image<unsigned char, 3>;
  using UCharFilterType = itk::MinMaxLabelImageFilterWithIndex<UCharImageType, LabelImageType>;

  // builds a 2x2x2 image from raw values in the iteration order of itk::ImageRegionIterator
  template <typename TImage>
  typename TImage::Pointer BuildImage(const std::vector<typename TImage::PixelType>& values)
  {
    typename TImage::SizeType size;
    size.Fill(2);
    typename TImage::IndexType start;
    start.Fill(0);

    auto image = TImage::New();
    image->SetRegions(typename TImage::RegionType(start, size));
    image->Allocate();

    itk::ImageRegionIterator<TImage> it(image, image->GetLargestPossibleRegion());
    auto valueIt = values.cbegin();

    for (it.GoToBegin(); !it.IsAtEnd(); ++it, ++valueIt)
      it.Set(*valueIt);

    return image;
  }

  ImageType::IndexType MakeIndex(itk::IndexValueType x, itk::IndexValueType y, itk::IndexValueType z)
  {
    ImageType::IndexType index;
    index[0] = x;
    index[1] = y;
    index[2] = z;
    return index;
  }
}

class mitkMinMaxLabelImageFilterWithIndexTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMinMaxLabelImageFilterWithIndexTestSuite);
  MITK_TEST(TestPerLabelExtrema);
  MITK_TEST(TestNegativeValues);
  MITK_TEST(TestUniformLabels);
  MITK_TEST(TestRerunDropsStaleResults);
  MITK_TEST(TestUnknownLabelThrows);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestPerLabelExtrema()
  {
    // label 1 holds the first slice (5, 1, 3, 8), label 2 the second (2, 7, 4, 6)
    auto image = BuildImage<ImageType>({ 5.f, 1.f, 3.f, 8.f, 2.f, 7.f, 4.f, 6.f });
    auto labels = BuildImage<LabelImageType>({ 1, 1, 1, 1, 2, 2, 2, 2 });

    auto filter = FilterType::New();
    filter->SetInput(image);
    filter->SetLabelInput(labels);
    filter->Update();

    CPPUNIT_ASSERT_EQUAL(1.f, filter->GetMin(1));
    CPPUNIT_ASSERT_EQUAL(8.f, filter->GetMax(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 0), filter->GetMinIndex(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 1, 0), filter->GetMaxIndex(1));
    CPPUNIT_ASSERT_EQUAL(2.f, filter->GetMin(2));
    CPPUNIT_ASSERT_EQUAL(7.f, filter->GetMax(2));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 1), filter->GetMinIndex(2));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 1), filter->GetMaxIndex(2));
    CPPUNIT_ASSERT_EQUAL(1.f, filter->GetGlobalMin());
    CPPUNIT_ASSERT_EQUAL(8.f, filter->GetGlobalMax());
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 0), filter->GetGlobalMinIndex());
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 1, 0), filter->GetGlobalMaxIndex());
  }

  void TestNegativeValues()
  {
    // a label holding only negative values has to report its maximum as well
    auto image = BuildImage<ImageType>({ -5.f, -1.f, -3.f, -8.f, -2.f, -7.f, -4.f, -6.f });
    auto labels = BuildImage<LabelImageType>({ 1, 1, 1, 1, 1, 1, 1, 1 });

    auto filter = FilterType::New();
    filter->SetInput(image);
    filter->SetLabelInput(labels);
    filter->Update();

    CPPUNIT_ASSERT_EQUAL(-8.f, filter->GetMin(1));
    CPPUNIT_ASSERT_EQUAL(-1.f, filter->GetMax(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 1, 0), filter->GetMinIndex(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 0), filter->GetMaxIndex(1));
    CPPUNIT_ASSERT_EQUAL(-1.f, filter->GetGlobalMax());
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 0), filter->GetGlobalMaxIndex());
  }

  void TestUniformLabels()
  {
    // for unsigned pixel types zero is the lowest value and 255 the highest, so a
    // label holding only one of them never improves on a sentinel seed; the
    // extrema have to come from the first pixel of the label
    auto image = BuildImage<UCharImageType>({ 255, 255, 255, 255, 0, 0, 0, 0 });
    auto labels = BuildImage<LabelImageType>({ 2, 2, 2, 2, 1, 1, 1, 1 });

    auto filter = UCharFilterType::New();
    filter->SetInput(image);
    filter->SetLabelInput(labels);
    filter->Update();

    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(filter->GetMin(1)));
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(filter->GetMax(1)));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 1), filter->GetMinIndex(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 1), filter->GetMaxIndex(1));
    CPPUNIT_ASSERT_EQUAL(255, static_cast<int>(filter->GetMin(2)));
    CPPUNIT_ASSERT_EQUAL(255, static_cast<int>(filter->GetMax(2)));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 0), filter->GetMinIndex(2));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 0), filter->GetMaxIndex(2));
    CPPUNIT_ASSERT_EQUAL(0, static_cast<int>(filter->GetGlobalMin()));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 1), filter->GetGlobalMinIndex());
    CPPUNIT_ASSERT_EQUAL(255, static_cast<int>(filter->GetGlobalMax()));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(0, 0, 0), filter->GetGlobalMaxIndex());
  }

  void TestRerunDropsStaleResults()
  {
    // the second run sees other values and only label 1, so nothing of the
    // first run may survive
    auto image = BuildImage<ImageType>({ 5.f, 1.f, 3.f, 8.f, 2.f, 7.f, 4.f, 6.f });
    auto shiftedImage = BuildImage<ImageType>({ 15.f, 11.f, 13.f, 18.f, 12.f, 17.f, 14.f, 16.f });
    auto twoLabels = BuildImage<LabelImageType>({ 1, 1, 1, 1, 2, 2, 2, 2 });
    auto oneLabel = BuildImage<LabelImageType>({ 1, 1, 1, 1, 1, 1, 1, 1 });

    auto filter = FilterType::New();
    filter->SetInput(image);
    filter->SetLabelInput(twoLabels);
    filter->Update();
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), filter->GetRelevantLabels().size());

    filter->SetInput(shiftedImage);
    filter->SetLabelInput(oneLabel);
    filter->Update();

    const auto labels = filter->GetRelevantLabels();
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), labels.size());
    CPPUNIT_ASSERT_EQUAL(LabelImageType::PixelType(1), labels.front());
    CPPUNIT_ASSERT_EQUAL(11.f, filter->GetMin(1));
    CPPUNIT_ASSERT_EQUAL(18.f, filter->GetMax(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 0, 0), filter->GetMinIndex(1));
    CPPUNIT_ASSERT_EQUAL(MakeIndex(1, 1, 0), filter->GetMaxIndex(1));
  }

  void TestUnknownLabelThrows()
  {
    auto image = BuildImage<ImageType>({ 5.f, 1.f, 3.f, 8.f, 2.f, 7.f, 4.f, 6.f });
    auto labels = BuildImage<LabelImageType>({ 1, 1, 1, 1, 1, 1, 1, 1 });

    auto filter = FilterType::New();
    filter->SetInput(image);
    filter->SetLabelInput(labels);
    filter->Update();

    CPPUNIT_ASSERT_THROW(filter->GetMin(2), itk::ExceptionObject);
    CPPUNIT_ASSERT_THROW(filter->GetMax(2), itk::ExceptionObject);
    CPPUNIT_ASSERT_THROW(filter->GetMinIndex(2), itk::ExceptionObject);
    CPPUNIT_ASSERT_THROW(filter->GetMaxIndex(2), itk::ExceptionObject);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMinMaxLabelImageFilterWithIndex)
