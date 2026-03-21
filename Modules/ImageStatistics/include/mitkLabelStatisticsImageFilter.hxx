/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLabelStatisticsImageFilter_hxx
#define mitkLabelStatisticsImageFilter_hxx

#include "mitkLabelStatisticsImageFilter.h"

#include <mitkHistogramStatisticsCalculator.h>

#include <itkImageLinearConstIteratorWithIndex.h>
#include <itkImageScanlineConstIterator.h>

template <typename TInputImage>
mitk::LabelStatisticsImageFilter<TInputImage>::LabelStatistics::LabelStatistics()
  : m_Count(0),
    m_CountOfPositivePixels(0),
    m_Min(itk::NumericTraits<RealType>::max()),
    m_Max(itk::NumericTraits<RealType>::NonpositiveMin()),
    m_Mean(0),
    m_Sum(0),
    m_SumOfPositivePixels(0),
    m_SumOfSquares(0),
    m_SumOfCubes(0),
    m_SumOfQuadruples(0),
    m_Sigma(0),
    m_Variance(0),
    m_MPP(0),
    m_Median(0),
    m_Uniformity(0),
    m_UPP(0),
    m_Entropy(0),
    m_Skewness(0),
    m_Kurtosis(0)
{
  m_BoundingBox.resize(ImageDimension * 2);

  for (std::remove_const_t<decltype(ImageDimension)> i = 0; i < ImageDimension * 2; i += 2)
  {
    m_BoundingBox[i] = itk::NumericTraits<itk::IndexValueType>::max();
    m_BoundingBox[i + 1] = itk::NumericTraits<itk::IndexValueType>::NonpositiveMin();
  }
}

template <typename TInputImage>
mitk::LabelStatisticsImageFilter<TInputImage>::LabelStatistics::LabelStatistics(unsigned int size, RealType lowerBound, RealType upperBound)
  : LabelStatistics()
{
  typename HistogramType::SizeType histogramSize;
  histogramSize.SetSize(1);
  histogramSize[0] = size;

  typename HistogramType::MeasurementVectorType histogramLowerBound;
  histogramLowerBound.SetSize(1);
  histogramLowerBound[0] = lowerBound;

  typename HistogramType::MeasurementVectorType histogramUpperBound;
  histogramUpperBound.SetSize(1);
  histogramUpperBound[0] = upperBound;

  m_Histogram = HistogramType::New();
  m_Histogram->SetMeasurementVectorSize(1);
  m_Histogram->Initialize(histogramSize, histogramLowerBound, histogramUpperBound);
}

template <typename TInputImage>
mitk::LabelStatisticsImageFilter<TInputImage>::LabelStatistics::~LabelStatistics()
{
}

template <typename TInputImage>
mitk::LabelStatisticsImageFilter<TInputImage>::LabelStatisticsImageFilter()
  : m_ComputeHistograms(false)
{
  this->AddRequiredInputName("LabelInput");
}

template <typename TInputImage>
mitk::LabelStatisticsImageFilter<TInputImage>::~LabelStatisticsImageFilter()
{
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::BeforeStreamedGenerateData() -> void
{
  this->AllocateOutputs();
  m_LabelStatistics.clear();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::ThreadedStreamedGenerateData(const RegionType& region) -> void
{
  if (0 == region.GetSize(0))
    return;

  MapType localStats;

  typename HistogramType::MeasurementVectorType histogramMeasurement(1);
  typename HistogramType::IndexType histogramIndex(1);

  using TLabelImage = itk::Image<LabelPixelType, ImageDimension>;

  itk::ImageLinearConstIteratorWithIndex<TInputImage> it(this->GetInput(), region);
  itk::ImageScanlineConstIterator<TLabelImage> labelIt(this->GetLabelInput(), region);

  auto mapIt = localStats.end();

  while (!it.IsAtEnd())
  {
    while (!it.IsAtEndOfLine())
    {
      const auto& value = static_cast<RealType>(it.Get());
      const auto& index = it.GetIndex();
      const auto& label = labelIt.Get();

      mapIt = localStats.find(label);

      if (mapIt == localStats.end())
      {
        mapIt = m_ComputeHistograms
          ? localStats.emplace(label, LabelStatistics(m_HistogramSizes[label], m_HistogramLowerBounds[label], m_HistogramUpperBounds[label])).first
          : localStats.emplace(label, LabelStatistics()).first;
      }

      auto& labelStats = mapIt->second;

      labelStats.m_Min = std::min(labelStats.m_Min, value);
      labelStats.m_Max = std::max(labelStats.m_Max, value);
      labelStats.m_Sum += value;
      auto squareValue = value * value;
      labelStats.m_SumOfSquares += squareValue;
      labelStats.m_SumOfCubes += squareValue * value;
      labelStats.m_SumOfQuadruples += squareValue * squareValue;
      ++labelStats.m_Count;

      if (0 < value)
      {
        labelStats.m_SumOfPositivePixels += value;
        ++labelStats.m_CountOfPositivePixels;
      }

      for (unsigned int i = 0; i < (ImageDimension * 2); i += 2)
      {
        labelStats.m_BoundingBox[i] = std::min(labelStats.m_BoundingBox[i], index[i / 2]);
        labelStats.m_BoundingBox[i + 1] = std::max(labelStats.m_BoundingBox[i + 1], index[i / 2]);
      }

      if (m_ComputeHistograms)
      {
        histogramMeasurement[0] = value;
        labelStats.m_Histogram->GetIndex(histogramMeasurement, histogramIndex);
        labelStats.m_Histogram->IncreaseFrequencyOfIndex(histogramIndex, 1);
      }

      ++labelIt;
      ++it;
    }

    labelIt.NextLine();
    it.NextLine();
  }

  // Merge localStats and m_LabelStatistics concurrently safe in a local copy

  while (true)
  {
    std::unique_lock<std::mutex> lock(m_Mutex);

    if (m_LabelStatistics.empty())
    {
      std::swap(m_LabelStatistics, localStats);
      break;
    }
    else
    {
      MapType toMerge;
      std::swap(m_LabelStatistics, toMerge);
      lock.unlock();
      this->MergeMap(localStats, toMerge);
    }
  }
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::AfterStreamedGenerateData() -> void
{
  Superclass::AfterStreamedGenerateData();

  m_ValidLabelValues.clear();
  m_ValidLabelValues.reserve(m_LabelStatistics.size());

  for (auto& val : m_LabelStatistics)
  {
    m_ValidLabelValues.push_back(val.first);
    auto& stats = val.second;

    const auto& sum = stats.m_Sum.GetSum();
    const auto& sumOfSquares = stats.m_SumOfSquares.GetSum();
    const auto& sumOfCubes = stats.m_SumOfCubes.GetSum();
    const auto& sumOfQuadruples = stats.m_SumOfQuadruples.GetSum();
    const auto& sumOfPositivePixels = stats.m_SumOfPositivePixels.GetSum();

    const RealType count = stats.m_Count;
    const RealType countOfPositivePixels = stats.m_CountOfPositivePixels;

    stats.m_Mean = sum / count;
    const auto& mean = stats.m_Mean;

    if (count > 1)
    {
      auto sumSquared = sum * sum;
      stats.m_Variance = (sumOfSquares - sumSquared / count) / (count - 1.0);
    }
    else
    {
      stats.m_Variance = 0.0;
    }

    stats.m_Sigma = std::sqrt(stats.m_Variance);

    const auto secondMoment = sumOfSquares / count;
    const auto thirdMoment = sumOfCubes / count;
    const auto fourthMoment = sumOfQuadruples / count;

    stats.m_Skewness = (thirdMoment - 3 * secondMoment * mean + 2 * std::pow(mean, 3)) / std::pow(secondMoment - std::pow(mean, 2), 1.5);
    stats.m_Kurtosis = (fourthMoment - 4 * thirdMoment * mean + 6 * secondMoment * std::pow(mean, 2) - 3 * std::pow(mean, 4)) / std::pow(secondMoment - std::pow(mean, 2), 2);
    stats.m_MPP = sumOfPositivePixels / countOfPositivePixels;

    if (m_ComputeHistograms)
    {
      mitk::HistogramStatisticsCalculator histogramStatisticsCalculator;
      histogramStatisticsCalculator.SetHistogram(stats.m_Histogram);
      histogramStatisticsCalculator.CalculateStatistics();

      stats.m_Entropy = histogramStatisticsCalculator.GetEntropy();
      stats.m_Uniformity = histogramStatisticsCalculator.GetUniformity();
      stats.m_UPP = histogramStatisticsCalculator.GetUPP();
      stats.m_Median = histogramStatisticsCalculator.GetMedian();
    }
  }
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::SetHistogramParameters(
  const std::unordered_map<LabelPixelType, unsigned int>& sizes,
  const std::unordered_map<LabelPixelType, RealType>& lowerBounds,
  const std::unordered_map<LabelPixelType, RealType>& upperBounds) -> void
{
  bool modified = false;

  if (m_HistogramSizes != sizes)
  {
    m_HistogramSizes = sizes;
    modified = true;
  }

  if (m_HistogramLowerBounds != lowerBounds)
  {
    m_HistogramLowerBounds = lowerBounds;
    modified = true;
  }

  if (m_HistogramUpperBounds != upperBounds)
  {
    m_HistogramUpperBounds = upperBounds;
    modified = true;
  }

  m_ComputeHistograms = true;

  if (modified)
    this->Modified();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::MergeMap(MapType& map1, MapType& map2) const -> void
{
  for (auto& elem2 : map2)
  {
    auto iter1 = map1.find(elem2.first);

    if (map1.end() == iter1)
    {
      map1.emplace(elem2.first, std::move(elem2.second));
    }
    else
    {
      const auto label = iter1->first;
      auto& stats1 = iter1->second;
      auto& stats2 = elem2.second;

      stats1.m_Min = std::min(stats1.m_Min, stats2.m_Min);
      stats1.m_Max = std::max(stats1.m_Max, stats2.m_Max);

      stats1.m_Sum += stats2.m_Sum;
      stats1.m_SumOfSquares += stats2.m_SumOfSquares;
      stats1.m_SumOfCubes += stats2.m_SumOfCubes;
      stats1.m_SumOfQuadruples += stats2.m_SumOfQuadruples;
      stats1.m_Count += stats2.m_Count;
      stats1.m_SumOfPositivePixels += stats2.m_SumOfPositivePixels;
      stats1.m_CountOfPositivePixels += stats2.m_CountOfPositivePixels;

      for (unsigned int i = 0; i < (ImageDimension * 2); i += 2)
      {
        stats1.m_BoundingBox[i] = std::min(stats1.m_BoundingBox[i], stats2.m_BoundingBox[i]);
        stats1.m_BoundingBox[i + 1] = std::max(stats1.m_BoundingBox[i + 1], stats2.m_BoundingBox[i + 1]);
      }

      if (m_ComputeHistograms)
      {
        typename HistogramType::IndexType index;
        index.SetSize(1);

        const auto histogramSize = m_HistogramSizes.at(label);

        for (unsigned int bin = 0; bin < histogramSize; ++bin)
        {
          index[0] = bin;
          stats1.m_Histogram->IncreaseFrequency(bin, stats2.m_Histogram->GetFrequency(bin));
        }
      }
    }
  }
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::HasLabel(LabelPixelType label) const -> bool
{
  return m_LabelStatistics.find(label) != m_LabelStatistics.end();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetNumberOfObjects() const -> unsigned int
{
  return static_cast<unsigned int>(m_LabelStatistics.size());
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetNumberOfLabels() const -> unsigned int
{
  return this->GetNumberOfObjects();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetValidLabelValues() const -> const ValidLabelValuesContainerType&
{
  return m_ValidLabelValues;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetLabelStatistics(LabelPixelType label) const -> const LabelStatistics&
{
  MapConstIterator it = m_LabelStatistics.find(label);

  if (m_LabelStatistics.end() != it)
    return it->second;

  mitkThrow() << "Label " << label << " does not exist";
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetLabelHistogramStatistics(LabelPixelType label) const -> const LabelStatistics&
{
  const auto& labelStatistics = this->GetLabelStatistics(label);

  if (m_ComputeHistograms && labelStatistics.m_Histogram.IsNotNull())
    return labelStatistics;

  mitkThrow() << "Histogram was not computed for label " << label;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetMinimum(LabelPixelType label) const -> PixelType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Min;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetMaximum(LabelPixelType label) const -> PixelType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Max;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetMean(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Mean;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSigma(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Sigma;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetVariance(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Variance;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSkewness(LabelPixelType label) const ->RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Skewness;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetKurtosis(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Kurtosis;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetMPP(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_MPP;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetCount(LabelPixelType label) const-> itk::SizeValueType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Count;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetHistogram(LabelPixelType label) const -> HistogramPointer
{
  const auto& labelStatistics = this->GetLabelHistogramStatistics(label);
  return labelStatistics.m_Histogram;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetEntropy(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelHistogramStatistics(label);
  return labelStatistics.m_Entropy;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetUniformity(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelHistogramStatistics(label);
  return labelStatistics.m_Uniformity;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetUPP(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelHistogramStatistics(label);
  return labelStatistics.m_UPP;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetMedian(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelHistogramStatistics(label);
  return labelStatistics.m_Median;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSum(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_Sum.GetSum();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSumOfSquares(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_SumOfSquares.GetSum();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSumOfCubes(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_SumOfCubes.GetSum();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetSumOfQuadruples(LabelPixelType label) const -> RealType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_SumOfQuadruples.GetSum();
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetBoundingBox(LabelPixelType label) const -> BoundingBoxType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);
  return labelStatistics.m_BoundingBox;
}

template <typename TInputImage>
auto mitk::LabelStatisticsImageFilter<TInputImage>::GetRegion(LabelPixelType label) const -> RegionType
{
  const auto& labelStatistics = this->GetLabelStatistics(label);

  IndexType index;
  SizeType size;

  for (unsigned int i = 0; i < ImageDimension; ++i)
  {
    index[i] = labelStatistics.m_BoundingBox[2 * i];
    size[i] = labelStatistics.m_BoundingBox[2 * i + 1] - labelStatistics.m_BoundingBox[2 * i] + 1;
  }

  RegionType region;
  region.SetIndex(index);
  region.SetSize(size);

  return region;
}

template <typename TInputImage>
void mitk::LabelStatisticsImageFilter<TInputImage>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Number of labels: " << m_LabelStatistics.size() << std::endl;
  os << indent << "Compute histograms: " << m_ComputeHistograms << std::endl;
}

#endif
