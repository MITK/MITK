/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExceptionMacro.h>
#include <mitkHistogramGenerator.h>
#include <mitkImage.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageTimeSelector.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImageToHistogramFilter.h>

#include <random>
#include <sstream>
#include <vector>

namespace
{
  using HistogramType = mitk::HistogramGenerator::HistogramType;

  constexpr unsigned int NUMBER_OF_BINS = 256;

  /** The ITK filter the generator promises to bin as. */
  template <typename TPixel, unsigned int VDimension>
  void ComputeReferenceHistogram(const itk::Image<TPixel, VDimension> *itkImage, HistogramType::ConstPointer &histogram)
  {
    using FilterType = itk::Statistics::ImageToHistogramFilter<itk::Image<TPixel, VDimension>>;

    auto filter = FilterType::New();
    filter->SetInput(itkImage);

    typename FilterType::HistogramSizeType size(1);
    size.Fill(NUMBER_OF_BINS);

    filter->SetHistogramSize(size);
    filter->SetAutoMinimumMaximum(true);
    filter->Update();

    histogram = filter->GetOutput();
  }

  HistogramType::ConstPointer ComputeReferenceHistogram(const mitk::Image *image)
  {
    auto firstTimeStep = mitk::SelectImageByTimeStep(image, 0);

    HistogramType::ConstPointer histogram;
    AccessByItk_1(firstTimeStep, ComputeReferenceHistogram, histogram);

    return histogram;
  }

  HistogramType::ConstPointer ComputeHistogram(const mitk::Image *image)
  {
    auto generator = mitk::HistogramGenerator::New();
    generator->SetImage(image);
    generator->SetSize(NUMBER_OF_BINS);
    generator->ComputeHistogram();

    return generator->GetHistogram();
  }

  template <typename TPixel>
  mitk::Image::Pointer CreateImage(const std::vector<TPixel> &values, const std::vector<unsigned int> &dimensions)
  {
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<TPixel>(), static_cast<unsigned int>(dimensions.size()),
                      dimensions.data());

    mitk::ImageWriteAccessor accessor(image);
    std::copy(values.begin(), values.end(), static_cast<TPixel *>(accessor.GetData()));

    return image;
  }

  template <typename TPixel, typename TDistribution>
  mitk::Image::Pointer CreateRandomImage(TDistribution distribution)
  {
    // Large enough to be split into several runs per binning step.
    const std::vector<unsigned int> dimensions = {67, 53, 41};

    std::mt19937 generator(42);
    std::vector<TPixel> values(67 * 53 * 41);

    for (auto &value : values)
      value = static_cast<TPixel>(distribution(generator));

    return CreateImage(values, dimensions);
  }

  void AssertEqualHistograms(const HistogramType *expected, const HistogramType *actual)
  {
    CPPUNIT_ASSERT_MESSAGE("No histogram", expected != nullptr && actual != nullptr);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong number of bins", expected->GetSize(0), actual->GetSize(0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong clipping at the ends", expected->GetClipBinsAtEnds(), actual->GetClipBinsAtEnds());

    for (unsigned int bin = 0; bin < expected->GetSize(0); ++bin)
    {
      std::ostringstream where;
      where << " in bin " << bin;

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong lower edge" + where.str(), expected->GetBinMin(0, bin), actual->GetBinMin(0, bin));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong upper edge" + where.str(), expected->GetBinMax(0, bin), actual->GetBinMax(0, bin));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong frequency" + where.str(), expected->GetFrequency(bin), actual->GetFrequency(bin));
    }
  }

  void AssertMatchesReference(const mitk::Image *image)
  {
    AssertEqualHistograms(ComputeReferenceHistogram(image), ComputeHistogram(image));
  }
}

class mitkHistogramGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkHistogramGeneratorTestSuite);
  MITK_TEST(UnsignedCharImage_MatchesReference);
  MITK_TEST(ShortImage_MatchesReference);
  MITK_TEST(IntImage_MatchesReference);
  MITK_TEST(FloatImage_MatchesReference);
  MITK_TEST(DoubleImage_MatchesReference);
  MITK_TEST(ValuesOnBinEdges_MatchReference);
  MITK_TEST(ConstantImage_MatchesReference);
  MITK_TEST(TimeSeries_BinsFirstTimeStep);
  MITK_TEST(InvalidInput_Throws);
  CPPUNIT_TEST_SUITE_END();

public:
  void UnsignedCharImage_MatchesReference()
  {
    AssertMatchesReference(CreateRandomImage<unsigned char>(std::uniform_int_distribution<int>(0, 255)));
  }

  void ShortImage_MatchesReference()
  {
    AssertMatchesReference(CreateRandomImage<short>(std::uniform_int_distribution<int>(-1024, 3071)));
  }

  void IntImage_MatchesReference()
  {
    AssertMatchesReference(CreateRandomImage<int>(std::uniform_int_distribution<int>(-3024, 3071)));
  }

  void FloatImage_MatchesReference()
  {
    AssertMatchesReference(CreateRandomImage<float>(std::normal_distribution<float>(0.3f, 0.1f)));
  }

  void DoubleImage_MatchesReference()
  {
    AssertMatchesReference(CreateRandomImage<double>(std::uniform_real_distribution<double>(-1e6, 1e6)));
  }

  /** Every edge is computed in float precision, so this is where an arithmetic
   * bin lookup would part ways with the search ITK runs. */
  void ValuesOnBinEdges_MatchReference()
  {
    const double margin = (1.0 / NUMBER_OF_BINS) / 100.0;

    auto layout = HistogramType::New();
    layout->SetMeasurementVectorSize(1);

    HistogramType::SizeType size(1);
    size.Fill(NUMBER_OF_BINS);
    HistogramType::MeasurementVectorType lowerBound(1);
    lowerBound.Fill(0.0);
    HistogramType::MeasurementVectorType upperBound(1);
    upperBound.Fill(1.0 + margin);
    layout->Initialize(size, lowerBound, upperBound);

    // 1 and the lower edge of the first bin, 0, pin the range, and with it the
    // edges the other values sit on. Each upper edge is the next bin's lower
    // one, except for the last, which lies past the range.
    std::vector<double> values = {1.0};

    for (unsigned int bin = 0; bin < NUMBER_OF_BINS; ++bin)
      values.push_back(layout->GetBinMin(0, bin));

    auto image = CreateImage(values, {static_cast<unsigned int>(values.size()), 1, 1});
    auto histogram = ComputeHistogram(image);

    AssertEqualHistograms(ComputeReferenceHistogram(image), histogram);

    for (unsigned int bin = 0; bin < NUMBER_OF_BINS; ++bin)
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("A value is not on the edge it was meant for",
        layout->GetBinMin(0, bin), histogram->GetBinMin(0, bin));
    }
  }

  void ConstantImage_MatchesReference()
  {
    AssertMatchesReference(CreateImage(std::vector<short>(1000, 7), {10, 10, 10}));
  }

  void TimeSeries_BinsFirstTimeStep()
  {
    std::vector<short> values(2 * 1000);

    for (std::size_t i = 0; i < values.size(); ++i)
      values[i] = static_cast<short>(i < 1000 ? i % 100 : 5000 + i);

    auto image = CreateImage(values, {10, 10, 10, 2});
    auto histogram = ComputeHistogram(image);

    AssertEqualHistograms(ComputeReferenceHistogram(image), histogram);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Range reaches past the first time step", 0.0, histogram->GetBinMin(0, 0));
  }

  void InvalidInput_Throws()
  {
    auto generator = mitk::HistogramGenerator::New();
    CPPUNIT_ASSERT_THROW(generator->ComputeHistogram(), mitk::Exception);

    auto vectorImage = mitk::Image::New();
    unsigned int dimensions[3] = {2, 2, 2};
    vectorImage->Initialize(mitk::MakePixelType<float, itk::Vector<float, 3>, 3>(), 3, dimensions);

    generator->SetImage(vectorImage);
    CPPUNIT_ASSERT_THROW(generator->ComputeHistogram(), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkHistogramGenerator)
