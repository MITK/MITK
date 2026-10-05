/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include <mitkImageStatisticsHolder.h>

#include <mitkHistogramGenerator.h>
#include <mitkImageAccessByItk.h>
#include <mitkImageReadAccessor.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkProperties.h>

#include <itkMultiThreaderBase.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

//#define BOUNDINGOBJECT_IGNORE

namespace
{
  /** \brief The smallest and the largest value of a run of voxels, the next
   *         value above and below each, and how often each of the two occurs.
   *
   * NaN fails every comparison and so is never counted, which is also why
   * whether any other value was seen has to be kept apart.
   */
  struct Extrema
  {
    mitk::ScalarType Min = itk::NumericTraits<mitk::ScalarType>::max();
    mitk::ScalarType SecondMin = itk::NumericTraits<mitk::ScalarType>::max();
    mitk::ScalarType Max = itk::NumericTraits<mitk::ScalarType>::NonpositiveMin();
    mitk::ScalarType SecondMax = itk::NumericTraits<mitk::ScalarType>::NonpositiveMin();
    std::uint64_t MinCount = 0;
    std::uint64_t MaxCount = 0;
    bool HasValue = false;

    void Add(mitk::ScalarType value)
    {
      HasValue = HasValue || !std::isnan(value);

      if (value < Min)
      {
        SecondMin = Min;
        Min = value;
        MinCount = 1;
      }
      else if (value == Min)
      {
        ++MinCount;
      }
      else if (value < SecondMin)
      {
        SecondMin = value;
      }

      if (value > Max)
      {
        SecondMax = Max;
        Max = value;
        MaxCount = 1;
      }
      else if (value == Max)
      {
        ++MaxCount;
      }
      else if (value > SecondMax)
      {
        SecondMax = value;
      }
    }

    /** \brief Take in the extrema of another run, with the result Add would
     *         have come to had it seen that run's voxels as well.
     */
    void Merge(const Extrema &other)
    {
      HasValue = HasValue || other.HasValue;

      if (other.MinCount > 0)
      {
        if (other.Min < Min)
        {
          SecondMin = std::min(Min, other.SecondMin);
          Min = other.Min;
          MinCount = other.MinCount;
        }
        else if (other.Min == Min)
        {
          SecondMin = std::min(SecondMin, other.SecondMin);
          MinCount += other.MinCount;
        }
        else
        {
          SecondMin = std::min(SecondMin, other.Min);
        }
      }

      if (other.MaxCount > 0)
      {
        if (other.Max > Max)
        {
          SecondMax = std::max(Max, other.SecondMax);
          Max = other.Max;
          MaxCount = other.MaxCount;
        }
        else if (other.Max == Max)
        {
          SecondMax = std::max(SecondMax, other.SecondMax);
          MaxCount += other.MaxCount;
        }
        else
        {
          SecondMax = std::max(SecondMax, other.Max);
        }
      }
    }
  };

  /** \brief Upper bound on the runs the voxels are split into.
   *
   * Several per thread, so that a thread finishing early takes another rather
   * than waiting for the slowest.
   */
  constexpr std::size_t MAX_NUMBER_OF_RUNS = 256;

  template <typename TPixel>
  void ComputeExtrema(const mitk::PixelType &, const void *data, std::size_t numberOfValues, Extrema &extrema)
  {
    const auto *values = static_cast<const TPixel *>(data);
    const auto numberOfRuns = std::min(numberOfValues, MAX_NUMBER_OF_RUNS);

    std::vector<Extrema> runs(numberOfRuns);

    itk::MultiThreaderBase::New()->ParallelizeArray(0, numberOfRuns, [&](itk::SizeValueType run)
      {
        const auto first = numberOfValues * run / numberOfRuns;
        const auto last = numberOfValues * (run + 1) / numberOfRuns;

        auto &runExtrema = runs[run];

        for (auto i = first; i < last; ++i)
          runExtrema.Add(static_cast<mitk::ScalarType>(values[i]));
      },
      nullptr);

    for (const auto &runExtrema : runs)
      extrema.Merge(runExtrema);
  }
}

mitk::ImageStatisticsHolder::ImageStatisticsHolder(mitk::Image *image)
  : m_Image(image)
{
  m_CountOfMinValuedVoxels.resize(1, 0);
  m_CountOfMaxValuedVoxels.resize(1, 0);
  m_ScalarMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.resize(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.resize(1, itk::NumericTraits<ScalarType>::NonpositiveMin());

  mitk::HistogramGenerator::Pointer generator = mitk::HistogramGenerator::New();
  m_HistogramGeneratorObject = generator;
}

mitk::ImageStatisticsHolder::~ImageStatisticsHolder()
{
  m_HistogramGeneratorObject = nullptr;
}

const mitk::ImageStatisticsHolder::HistogramType *mitk::ImageStatisticsHolder::GetScalarHistogram(
  int t, unsigned int /*component*/)
{
  mitk::ImageTimeSelector *timeSelector = this->GetTimeSelector();
  if (timeSelector != nullptr)
  {
    timeSelector->SetTimeNr(t);
    timeSelector->UpdateLargestPossibleRegion();

    auto *generator =
      static_cast<mitk::HistogramGenerator *>(m_HistogramGeneratorObject.GetPointer());
    generator->SetImage(timeSelector->GetOutput());
    generator->ComputeHistogram();
    return static_cast<const mitk::ImageStatisticsHolder::HistogramType *>(generator->GetHistogram());
  }
  return nullptr;
}

bool mitk::ImageStatisticsHolder::IsValidTimeStep(int t) const
{
  return m_Image->IsValidTimeStep(t);
}

mitk::ImageTimeSelector::Pointer mitk::ImageStatisticsHolder::GetTimeSelector()
{
  ImageTimeSelector::Pointer timeSelector =
    ImageTimeSelector::New();
  timeSelector->SetInput(m_Image);

  return timeSelector;
}

void mitk::ImageStatisticsHolder::Expand(unsigned int timeSteps)
{
  if (!m_Image->IsValidTimeStep(timeSteps - 1))
    return;

  // The BaseData needs to be expanded, call the mitk::Image::Expand() method
  m_Image->Expand(timeSteps);

  if (timeSteps > m_ScalarMin.size())
  {
    m_ScalarMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_ScalarMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_Scalar2ndMin.resize(timeSteps, itk::NumericTraits<ScalarType>::max());
    m_Scalar2ndMax.resize(timeSteps, itk::NumericTraits<ScalarType>::NonpositiveMin());
    m_CountOfMinValuedVoxels.resize(timeSteps, 0);
    m_CountOfMaxValuedVoxels.resize(timeSteps, 0);
  }
}

void mitk::ImageStatisticsHolder::ResetImageStatistics()
{
  m_ScalarMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_ScalarMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_Scalar2ndMin.assign(1, itk::NumericTraits<ScalarType>::max());
  m_Scalar2ndMax.assign(1, itk::NumericTraits<ScalarType>::NonpositiveMin());
  m_CountOfMinValuedVoxels.assign(1, 0);
  m_CountOfMaxValuedVoxels.assign(1, 0);
}

void mitk::ImageStatisticsHolder::ComputeScalarExtrema(int t)
{
  const auto pixelType = m_Image->GetPixelType(0);

  // The volume itself rather than an ImageTimeSelector's output: the selector
  // writes its request into the image it selects from, which the mappers on
  // the GUI thread read, and this may run on a worker.
  auto volume = m_Image->GetVolumeData(t);

  if (volume.IsNull())
    return;

  const ImageReadAccessor accessor(m_Image, volume.GetPointer());

  std::size_t numberOfValues = 1;

  for (unsigned int i = 0; i < 3; ++i)
    numberOfValues *= m_Image->GetDimension(i);

  Extrema extrema;
  mitkPixelTypeMultiplex3(ComputeExtrema, pixelType, accessor.GetData(), numberOfValues, extrema);

  if (!extrema.HasValue)
  {
    extrema.Max = 0;
    extrema.Min = 0;
  }

  // A constant image has no second value, so the one it has stands in for it.
  if (extrema.Max == extrema.Min)
    extrema.SecondMax = extrema.SecondMin = extrema.Max;

  m_ScalarMin[t] = extrema.Min;
  m_ScalarMax[t] = extrema.Max;
  m_Scalar2ndMin[t] = extrema.SecondMin;
  m_Scalar2ndMax[t] = extrema.SecondMax;
  m_CountOfMinValuedVoxels[t] = static_cast<unsigned int>(extrema.MinCount);
  m_CountOfMaxValuedVoxels[t] = static_cast<unsigned int>(extrema.MaxCount);
  m_LastRecomputeTimeStamp.Modified();
}

/// \cond SKIP_DOXYGEN
template <typename ItkImageType>
void mitk::_ComputeExtremaInItkVectorImage(const ItkImageType *itkImage,
                                           mitk::ImageStatisticsHolder *statisticsHolder,
                                           int t,
                                           unsigned int component)
{
  typename ItkImageType::RegionType region;
  region = itkImage->GetBufferedRegion();
  if (region.Crop(itkImage->GetRequestedRegion()) == false)
    return;
  if (region != itkImage->GetRequestedRegion())
    return;

  itk::ImageRegionConstIterator<ItkImageType> it(itkImage, region);

  if (statisticsHolder == nullptr || !statisticsHolder->IsValidTimeStep(t))
    return;
  statisticsHolder->Expand(t + 1); // make sure we have initialized all arrays
  statisticsHolder->m_CountOfMinValuedVoxels[t] = 0;
  statisticsHolder->m_CountOfMaxValuedVoxels[t] = 0;

  statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t] = itk::NumericTraits<ScalarType>::max();
  statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t] =
    itk::NumericTraits<ScalarType>::NonpositiveMin();

  while (!it.IsAtEnd())
  {
    double value = it.Get()[component];
#ifdef BOUNDINGOBJECT_IGNORE
    if (value > -32765)
    {
#endif
      // update min
      if (value < statisticsHolder->m_ScalarMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMin[t];
        statisticsHolder->m_ScalarMin[t] = value;
        statisticsHolder->m_CountOfMinValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMin[t])
      {
        ++statisticsHolder->m_CountOfMinValuedVoxels[t];
      }
      else if (value < statisticsHolder->m_Scalar2ndMin[t])
      {
        statisticsHolder->m_Scalar2ndMin[t] = value;
      }

      // update max
      if (value > statisticsHolder->m_ScalarMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_ScalarMax[t];
        statisticsHolder->m_ScalarMax[t] = value;
        statisticsHolder->m_CountOfMaxValuedVoxels[t] = 1;
      }
      else if (value == statisticsHolder->m_ScalarMax[t])
      {
        ++statisticsHolder->m_CountOfMaxValuedVoxels[t];
      }
      else if (value > statisticsHolder->m_Scalar2ndMax[t])
      {
        statisticsHolder->m_Scalar2ndMax[t] = value;
      }
#ifdef BOUNDINGOBJECT_IGNORE
    }
#endif

    ++it;
  }

  //// guard for wrong 2dMin/Max on single constant value images
  if (statisticsHolder->m_ScalarMax[t] == statisticsHolder->m_ScalarMin[t])
  {
    statisticsHolder->m_Scalar2ndMax[t] = statisticsHolder->m_Scalar2ndMin[t] = statisticsHolder->m_ScalarMax[t];
  }
  statisticsHolder->m_LastRecomputeTimeStamp.Modified();
}
/// \endcond SKIP_DOXYGEN
void mitk::ImageStatisticsHolder::ComputeImageStatistics(int t, unsigned int component)
{
  // timestep valid?
  if (!m_Image->IsValidTimeStep(t))
    return;

  // image modified?
  if (this->m_Image->GetMTime() > m_LastRecomputeTimeStamp.GetMTime())
    this->ResetImageStatistics();

  Expand(t + 1);

  // do we have valid information already?
  if (m_ScalarMin[t] != itk::NumericTraits<ScalarType>::max() ||
      m_Scalar2ndMin[t] != itk::NumericTraits<ScalarType>::max())
    return; // Values already calculated before...

  // used to avoid statistics calculation on Odf images. property will be replaced as soons as bug 17928 is merged and
  // the diffusion image refactoring is complete.
  mitk::BoolProperty *isSh = dynamic_cast<mitk::BoolProperty *>(m_Image->GetProperty("IsShImage").GetPointer());
  mitk::BoolProperty *isOdf = dynamic_cast<mitk::BoolProperty *>(m_Image->GetProperty("IsOdfImage").GetPointer());
  const mitk::PixelType pType = m_Image->GetPixelType(0);
  if (pType.GetNumberOfComponents() == 1 && (pType.GetPixelType() != itk::IOPixelEnum::UNKNOWNPIXELTYPE) &&
      (pType.GetPixelType() != itk::IOPixelEnum::VECTOR))
  {
    this->ComputeScalarExtrema(t);
  }
  else if (pType.GetPixelType() == itk::IOPixelEnum::VECTOR &&
           (!isOdf || !isOdf->GetValue()) && (!isSh || !isSh->GetValue())) // we have a vector image
  {
    // recompute
    mitk::ImageTimeSelector::Pointer timeSelector = this->GetTimeSelector();
    if (timeSelector.IsNotNull())
    {
      timeSelector->SetTimeNr(t);
      timeSelector->UpdateLargestPossibleRegion();
      const mitk::Image *image = timeSelector->GetOutput();
      AccessVectorPixelTypeByItk_n(image, _ComputeExtremaInItkVectorImage, (this, t, component));
    }
  }
  else
  {
    m_ScalarMin[t] = 0;
    m_ScalarMax[t] = 255;
    m_Scalar2ndMin[t] = 0;
    m_Scalar2ndMax[t] = 255;
  }
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMin(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_ScalarMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValueMax(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_ScalarMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMin(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_Scalar2ndMin[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetScalarValue2ndMax(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_Scalar2ndMax[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMinValuedVoxels(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_CountOfMinValuedVoxels[t];
}

mitk::ScalarType mitk::ImageStatisticsHolder::GetCountOfMaxValuedVoxels(int t, unsigned int component)
{
  const std::lock_guard<std::mutex> lock(m_StatisticsMutex);
  this->ComputeImageStatistics(t, component);
  return m_CountOfMaxValuedVoxels[t];
}
