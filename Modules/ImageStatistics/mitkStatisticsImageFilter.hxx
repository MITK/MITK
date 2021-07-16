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
#include <itkImageScanlineConstIterator.h>

template <typename TInputImage>
mitk::StatisticsImageFilter<TInputImage>::StatisticsImageFilter()
  : m_ThreadSum(1),
    m_SumOfPositivePixels(1),
    m_SumOfSquares(1),
    m_SumOfCubes(1),
    m_SumOfQuadruples(1),
    m_Count(1),
    m_CountOfPositivePixels(1),
    m_ThreadMin(1),
    m_ThreadMax(1)
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

  return Superclass::MakeOutput(name);
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::BeforeStreamedGenerateData()
{
  Superclass::BeforeStreamedGenerateData();

  m_ThreadSum = 0;
  m_SumOfPositivePixels = 0;
  m_SumOfSquares = 0;
  m_SumOfCubes = 0;
  m_SumOfQuadruples = 0;
  m_Count = 0;
  m_CountOfPositivePixels = 0;
  m_ThreadMin = itk::NumericTraits<PixelType>::max();
  m_ThreadMax = itk::NumericTraits<PixelType>::NonpositiveMin();
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
  RealType squareValue = 0;

  itk::ImageScanlineConstIterator<TInputImage> it(this->GetInput(), regionForThread);

  while (!it.IsAtEnd())
  {
    while (!it.IsAtEndOfLine())
    {
      const auto& value = it.Get();
      const auto realValue = static_cast<RealType>(value);
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

  m_ThreadSum += sum;
  m_SumOfPositivePixels += sumOfPositivePixels;
  m_SumOfSquares += sumOfSquares;
  m_SumOfCubes += sumOfCubes;
  m_SumOfQuadruples += sumOfQuadruples;
  m_Count += count;
  m_CountOfPositivePixels += countOfPositivePixels;
  m_ThreadMin = std::min(min, m_ThreadMin);
  m_ThreadMax = std::max(max, m_ThreadMax);
}

template <typename TInputImage>
void mitk::StatisticsImageFilter<TInputImage>::AfterStreamedGenerateData()
{
  Superclass::AfterStreamedGenerateData();

  const RealType sum = m_ThreadSum;
  const RealType sumOfPositivePixels = m_SumOfPositivePixels;
  const RealType sumOfSquares = m_SumOfSquares;
  const RealType sumOfCubes = m_SumOfCubes;
  const RealType sumOfQuadruples = m_SumOfQuadruples;
  const itk::SizeValueType count = m_Count;
  const itk::SizeValueType countOfPositivePixels = m_CountOfPositivePixels;
  const PixelType minimum = m_ThreadMin;
  const PixelType maximum = m_ThreadMax;

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
  this->SetEntropy(entropy);
  this->SetUniformity(uniformity);
  this->SetUPP(upp);
  this->SetMedian(median);
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
