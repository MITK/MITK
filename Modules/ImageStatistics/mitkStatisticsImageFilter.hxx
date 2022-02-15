/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkStatisticsImageFilter_hxx
#define mitkStatisticsImageFilter_hxx

#include <mitkStatisticsImageFilter.h>
#include <mitkHistogramStatisticsCalculator.h>
#include <itkImageScanlineConstIterator.h>

template <typename TInputImage>
mitk::StatisticsImageFilter<TInputImage>::StatisticsImageFilter()
  : m_ComputeHistogram(false),
    m_HistogramSize(0),
    m_HistogramLowerBound(itk::NumericTraits<RealType>::NonpositiveMin()),
    m_HistogramUpperBound(itk::NumericTraits<RealType>::max()),
    m_Sum(1),
    m_SumOfPositivePixels(1),
    m_SumOfSquares(1),
    m_SumOfCubes(1),
    m_SumOfQuadruples(1),
    m_Count(1),
    m_CountOfPositivePixels(1),
    m_Min(1),
    m_Max(1)
{
  this->SetNumberOfRequiredInputs(1);

  this->SetMinimum(itk::NumericTraits<PixelType>::max());
  this->SetMaximum(itk::NumericTraits<PixelType>::NonpositiveMin());
  this->SetMean(itk::NumericTraits<RealType>::max());
  this->SetSigma(itk::NumericTraits<RealType>::max());
  this->SetVariance(itk::NumericTraits<RealType>::max());
  this->SetSum(0);
  this->SetSumOfSquares(0);
  this->SetSumOfCubes(0);
  this->SetSumOfQuadruples(0);
  this->SetSkewness(0);
  this->SetKurtosis(0);
  this->SetMPP(0);
  this->SetEntropy(-1.0);
  this->SetUniformity(0);
  this->SetUPP(0);
  this->SetMedian(0);
}

template <typename TInputImage>
mitk::StatisticsImageFilter<TInputImage>::~StatisticsImageFilter()
{
}

template <typename TInputImage>
typename mitk::StatisticsImageFilter<TInputImage>::DataObjectPointer mitk::StatisticsImageFilter<TInputImage>::MakeOutput(const DataObjectIdentifierType& name)
{
  if (name == "Minimum" ||
      name == "Maximum")
  {
    return PixelObjectType::New();
  }

  if (name == "Mean" ||
      name == "Sigma" ||
      name == "Variance" ||
      name == "Sum" ||
      name == "SumOfSquares" ||
      name == "SumOfCubes" ||
      name == "SumOfQuadruples" ||
      name == "Skewness" ||
      name == "Kurtosis" ||
      name == "MPP" ||
      name == "Entropy" || 
      name == "Uniformity" || 
      name == "UPP" || 
      name == "Median")
  {
    return RealObjectType::New();
  }

  if (name == "Histogram")
  {
    return HistogramType::New();
  }

  return Superclass::MakeOutput(name);
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::SetHistogramParameters(unsigned int size, RealType lowerBound, RealType upperBound)
{
  bool modified = false;

  if (m_HistogramSize != size)
  {
    m_HistogramSize = size;
    modified = true;
  }

  if (m_HistogramLowerBound != lowerBound)
  {
    m_HistogramLowerBound = lowerBound;
    modified = true;
  }

  if (m_HistogramUpperBound != upperBound)
  {
    m_HistogramUpperBound = upperBound;
    modified = true;
  }

  m_ComputeHistogram = true;

  if (modified)
    this->Modified();
}

template <typename TInputImage>
auto mitk::StatisticsImageFilter<TInputImage>::CreateInitializedHistogram() const -> HistogramPointer
{
  typename HistogramType::SizeType size;
  size.SetSize(1);
  size.Fill(m_HistogramSize);

  typename HistogramType::MeasurementVectorType lowerBound;
  lowerBound.SetSize(1);
  lowerBound.Fill(m_HistogramLowerBound);

  typename HistogramType::MeasurementVectorType upperBound;
  upperBound.SetSize(1);
  upperBound.Fill(m_HistogramUpperBound);

  auto histogram = HistogramType::New();
  histogram->SetMeasurementVectorSize(1);
  histogram->Initialize(size, lowerBound, upperBound);

  return histogram;
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::BeforeStreamedGenerateData()
{
  Superclass::BeforeStreamedGenerateData();

  m_Sum = 0;
  m_SumOfPositivePixels = 0;
  m_SumOfSquares = 0;
  m_SumOfCubes = 0;
  m_SumOfQuadruples = 0;
  m_Count = 0;
  m_CountOfPositivePixels = 0;
  m_Min = itk::NumericTraits<PixelType>::max();
  m_Max = itk::NumericTraits<PixelType>::NonpositiveMin();

  if (m_ComputeHistogram)
    m_Histogram = this->CreateInitializedHistogram();
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::ThreadedStreamedGenerateData(const RegionType& regionForThread)
{
  itk::CompensatedSummation<RealType> sum = 0;
  itk::CompensatedSummation<RealType> sumOfPositivePixels = 0;
  itk::CompensatedSummation<RealType> sumOfSquares = 0;
  itk::CompensatedSummation<RealType> sumOfCubes = 0;
  itk::CompensatedSummation<RealType> sumOfQuadruples = 0;
  itk::SizeValueType count = 0;
  itk::SizeValueType countOfPositivePixels = 0;
  auto min = itk::NumericTraits<PixelType>::max();
  auto max = itk::NumericTraits<PixelType>::NonpositiveMin();
  RealType realValue = 0;
  RealType squareValue = 0;

  HistogramPointer histogram;
  typename HistogramType::MeasurementVectorType histogramMeasurement;
  typename HistogramType::IndexType histogramIndex;

  if (m_ComputeHistogram) // Initialize histogram
  {
    histogram = this->CreateInitializedHistogram();
    histogramMeasurement.SetSize(1);
  }

  itk::ImageScanlineConstIterator<TInputImage> it(this->GetInput(), regionForThread);

  while (!it.IsAtEnd())
  {
    while (!it.IsAtEndOfLine())
    {
      const auto& value = it.Get();
      realValue = static_cast<RealType>(value);

      if (m_ComputeHistogram) // Compute histogram
      {
        histogramMeasurement[0] = realValue;
        histogram->GetIndex(histogramMeasurement, histogramIndex);
        histogram->IncreaseFrequencyOfIndex(histogramIndex, 1);
      }

      min = std::min(min, value);
      max = std::max(max, value);
      squareValue = realValue * realValue;

      sum += realValue;
      sumOfSquares += squareValue;
      sumOfCubes += squareValue * realValue;
      sumOfQuadruples += squareValue * squareValue;
      ++count;

      if (0 < realValue)
      {
        sumOfPositivePixels += realValue;
        ++countOfPositivePixels;
      }

      ++it;
    }

    it.NextLine();
  }

  std::lock_guard<std::mutex> mutexHolder(m_Mutex);

  if (m_ComputeHistogram) // Merge histograms
  {
    typename HistogramType::ConstIterator histogramIt = histogram->Begin();
    typename HistogramType::ConstIterator histogramEnd = histogram->End();

    while (histogramIt != histogramEnd)
    {
      m_Histogram->GetIndex(histogramIt.GetMeasurementVector(), histogramIndex);
      m_Histogram->IncreaseFrequencyOfIndex(histogramIndex, histogramIt.GetFrequency());
      ++histogramIt;
    }
  }

  m_Sum += sum;
  m_SumOfPositivePixels += sumOfPositivePixels;
  m_SumOfSquares += sumOfSquares;
  m_SumOfCubes += sumOfCubes;
  m_SumOfQuadruples += sumOfQuadruples;
  m_Count += count;
  m_CountOfPositivePixels += countOfPositivePixels;
  m_Min = std::min(min, m_Min);
  m_Max = std::max(max, m_Max);
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::AfterStreamedGenerateData()
{
  Superclass::AfterStreamedGenerateData();

  const RealType sum = m_Sum.GetSum();
  const RealType sumOfPositivePixels = m_SumOfPositivePixels.GetSum();
  const RealType sumOfSquares = m_SumOfSquares.GetSum();
  const RealType sumOfCubes = m_SumOfCubes.GetSum();
  const RealType sumOfQuadruples = m_SumOfQuadruples.GetSum();
  const itk::SizeValueType count = m_Count;
  const itk::SizeValueType countOfPositivePixels = m_CountOfPositivePixels;
  const PixelType minimum = m_Min;
  const PixelType maximum = m_Max;

  const RealType mean = sum / static_cast<RealType>(count);
  const RealType variance = (sumOfSquares - (sum * sum / static_cast<RealType>(count))) / (static_cast<RealType>(count) - 1);
  const RealType sigma = std::sqrt(variance);

  const RealType secondMoment = sumOfSquares / static_cast<RealType>(count);
  const RealType thirdMoment = sumOfCubes / static_cast<RealType>(count);
  const RealType fourthMoment = sumOfQuadruples / static_cast<RealType>(count);
  const RealType skewness = (thirdMoment - 3 * secondMoment * mean + 2 * std::pow(mean, 3)) / std::pow(secondMoment - std::pow(mean, 2), 1.5);
  const RealType kurtosis = (fourthMoment - 4 * thirdMoment * mean + 6 * secondMoment * std::pow(mean, 2) - 3 * std::pow(mean, 4)) / std::pow(secondMoment - std::pow(mean, 2), 2);
  const RealType meanOfPositivePixels = sumOfPositivePixels / static_cast<RealType>(countOfPositivePixels);

  this->SetMinimum(minimum);
  this->SetMaximum(maximum);
  this->SetMean(mean);
  this->SetSigma(sigma);
  this->SetVariance(variance);
  this->SetSum(sum);
  this->SetSumOfSquares(sumOfSquares);
  this->SetSumOfCubes(sumOfCubes);
  this->SetSumOfQuadruples(sumOfQuadruples);
  this->SetSkewness(skewness);
  this->SetKurtosis(kurtosis);
  this->SetMPP(meanOfPositivePixels);

  if (m_ComputeHistogram)
  {
    this->SetHistogram(m_Histogram);

    mitk::HistogramStatisticsCalculator histogramStatisticsCalculator;
    histogramStatisticsCalculator.SetHistogram(m_Histogram);
    histogramStatisticsCalculator.CalculateStatistics();

    this->SetEntropy(histogramStatisticsCalculator.GetEntropy());
    this->SetUniformity(histogramStatisticsCalculator.GetUniformity());
    this->SetUPP(histogramStatisticsCalculator.GetUPP());
    this->SetMedian(histogramStatisticsCalculator.GetMedian());
  }
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "SumOfCubes: " << this->GetSumOfCubes() << std::endl;
  os << indent << "SumOfQuadruples: " << this->GetSumOfQuadruples() << std::endl;
  os << indent << "Skewness: " << this->GetSkewness() << std::endl;
  os << indent << "Kurtosis: " << this->GetKurtosis() << std::endl;
  os << indent << "MPP: " << this->GetMPP() << std::endl;
  os << indent << "Entropy: " << this->GetEntropy() << std::endl;
  os << indent << "Uniformity: " << this->GetUniformity() << std::endl;
  os << indent << "UPP: " << this->GetUPP() << std::endl;
  os << indent << "Median: " << this->GetMedian() << std::endl;
}

#endif
