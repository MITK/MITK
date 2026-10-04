/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkHistogramGenerator.h>

#include <mitkExceptionMacro.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageStatisticsHolder.h>
#include <mitkPixelTypeMultiplex.h>
#include <mitkProgressTask.h>

#include <itkMacro.h>
#include <itkMultiThreaderBase.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <vector>

namespace
{
  using HistogramType = mitk::HistogramGenerator::HistogramType;

  /** \brief How much narrower than a bin the space left above the maximum is.
   *
   * The value itk::Statistics::ImageToHistogramFilter defaults to, which keeps
   * every bin edge where that filter puts it.
   */
  constexpr double MARGINAL_SCALE = 100.0;

  /** \brief The steps of progress the binning reports. */
  constexpr unsigned int BINNING_STEPS = 64;

  /** \brief How many voxels are binned at the least before progress is
   *         reported and a cancel is looked for.
   *
   * Each slice of the voxels is one parallel burst, and short bursts do not
   * get the machine's cores to themselves: a 36 million voxel CT binned in 64
   * bursts took over ten times as long as in one.
   */
  constexpr std::size_t MIN_VALUES_PER_SLICE = std::size_t{1} << 26;

  /** \brief What finding the value range counts for next to the binning.
   *
   * It is one more pass over the voxels, and one that takes about as long.
   */
  constexpr unsigned int RANGE_STEPS = BINNING_STEPS;

  /** \brief Upper bound on the runs each slice is split into across threads. */
  constexpr std::size_t MAX_NUMBER_OF_RUNS = 64;

  struct BinningInput
  {
    const void *Data;
    std::size_t NumberOfValues;
    mitk::ScalarType Minimum;
    mitk::ScalarType Maximum;
    unsigned int NumberOfBins;
    mitk::ProgressTask *Task;
  };

  /** \brief Lay out the bins as itk::Statistics::ImageToHistogramFilter does
   *         for an automatic range: from the minimum to just above the maximum.
   */
  HistogramType::Pointer CreateHistogram(double minimum, double maximum, unsigned int numberOfBins)
  {
    const double margin = ((maximum - minimum) / numberOfBins) / MARGINAL_SCALE;

    auto histogram = HistogramType::New();
    histogram->SetMeasurementVectorSize(1);

    HistogramType::MeasurementVectorType lowerBound(1);
    HistogramType::MeasurementVectorType upperBound(1);

    lowerBound.Fill(minimum);

    if (std::numeric_limits<double>::max() - maximum > margin)
    {
      upperBound.Fill(maximum + margin);
    }
    else
    {
      // No room above the maximum, so it would fall off the last bin unless
      // the bins at the ends are open.
      upperBound.Fill(maximum);
      histogram->SetClipBinsAtEnds(false);
    }

    HistogramType::SizeType size(1);
    size.Fill(numberOfBins);

    histogram->Initialize(size, lowerBound, upperBound);

    return histogram;
  }

  /** \brief Finds the bin of a value as itk::Statistics::Histogram::GetIndex
   *         does, without its search.
   *
   * The edges are computed in float precision, so a bin is not exactly as wide
   * as the others. The arithmetic estimate is therefore settled against the
   * edges themselves, which also puts a value on an edge into the bin above it,
   * as GetIndex does.
   */
  class BinLocator
  {
  public:
    explicit BinLocator(const HistogramType *histogram)
      : m_LastBin(static_cast<std::ptrdiff_t>(histogram->GetSize(0)) - 1)
    {
      for (HistogramType::InstanceIdentifier bin = 0; bin < histogram->GetSize(0); ++bin)
      {
        m_BinMin.push_back(histogram->GetBinMin(0, bin));
        m_BinMax.push_back(histogram->GetBinMax(0, bin));
      }

      m_Lowest = m_BinMin.front();
      m_Highest = m_BinMax.back();
      m_Scale = static_cast<double>(m_LastBin + 1) / (m_Highest - m_Lowest);
    }

    std::ptrdiff_t operator()(double value) const
    {
      // Only the maximum reaches the end, and only where there was no room for
      // a margin above it. GetIndex then counts it into the last bin, as it
      // does for a value on the very end.
      if (value >= m_Highest)
        return m_LastBin;

      const double position = (value - m_Lowest) * m_Scale;

      // Signed: converting a double to an unsigned 64-bit integer takes a
      // branchy library routine rather than a single instruction on x64, and
      // this runs once per voxel.
      auto bin = position > 0.0
        ? static_cast<std::ptrdiff_t>(std::min(position, static_cast<double>(m_LastBin)))
        : std::ptrdiff_t{0};

      while (bin > 0 && value < m_BinMin[bin])
        --bin;

      while (bin < m_LastBin && value >= m_BinMax[bin])
        ++bin;

      return bin;
    }

  private:
    std::ptrdiff_t m_LastBin;
    std::vector<double> m_BinMin;
    std::vector<double> m_BinMax;
    double m_Lowest;
    double m_Highest;
    double m_Scale;
  };

  template <typename TPixel>
  void ComputeBins(const mitk::PixelType &, const BinningInput &input, HistogramType::Pointer &result)
  {
    auto histogram = CreateHistogram(input.Minimum, input.Maximum, input.NumberOfBins);

    const BinLocator locate(histogram);
    const auto *values = static_cast<const TPixel *>(input.Data);

    std::vector<std::uint64_t> frequencies(input.NumberOfBins, 0);
    auto multiThreader = itk::MultiThreaderBase::New();

    const auto numberOfSlices = std::clamp(
      (input.NumberOfValues + MIN_VALUES_PER_SLICE - 1) / MIN_VALUES_PER_SLICE,
      std::size_t{1},
      static_cast<std::size_t>(BINNING_STEPS));

    unsigned int reportedSteps = 0;

    for (std::size_t slice = 0; slice < numberOfSlices; ++slice)
    {
      if (input.Task != nullptr && input.Task->IsCancelRequested())
        throw itk::ProcessAborted(__FILE__, __LINE__);

      const auto sliceBegin = input.NumberOfValues * slice / numberOfSlices;
      const auto sliceEnd = input.NumberOfValues * (slice + 1) / numberOfSlices;
      const auto sliceSize = sliceEnd - sliceBegin;
      const auto numberOfRuns = std::min(sliceSize, MAX_NUMBER_OF_RUNS);

      std::vector<std::vector<std::uint64_t>> runFrequencies(
        numberOfRuns, std::vector<std::uint64_t>(input.NumberOfBins, 0));

      multiThreader->ParallelizeArray(0, numberOfRuns, [&](itk::SizeValueType run)
        {
          const auto first = sliceBegin + sliceSize * run / numberOfRuns;
          const auto last = sliceBegin + sliceSize * (run + 1) / numberOfRuns;

          auto &counts = runFrequencies[run];

          for (auto i = first; i < last; ++i)
          {
            const auto value = static_cast<double>(values[i]);

            // Never counted, rather than counted into whichever bin a search
            // for it happens to end on.
            if constexpr (std::is_floating_point_v<TPixel>)
            {
              if (std::isnan(value))
                continue;
            }

            ++counts[locate(value)];
          }
        },
        nullptr);

      for (const auto &counts : runFrequencies)
      {
        for (std::size_t bin = 0; bin < counts.size(); ++bin)
          frequencies[bin] += counts[bin];
      }

      if (input.Task != nullptr)
      {
        const auto reachedSteps = static_cast<unsigned int>(BINNING_STEPS * (slice + 1) / numberOfSlices);
        input.Task->Progress(reachedSteps - reportedSteps);
        reportedSteps = reachedSteps;
      }
    }

    for (std::size_t bin = 0; bin < frequencies.size(); ++bin)
      histogram->SetFrequency(static_cast<HistogramType::InstanceIdentifier>(bin), frequencies[bin]);

    result = histogram;
  }
}

mitk::HistogramGenerator::HistogramGenerator()
  : m_Image(nullptr),
    m_Size(256),
    m_Histogram(nullptr),
    m_ProgressTask(nullptr)
{
}

mitk::HistogramGenerator::~HistogramGenerator()
{
}

void mitk::HistogramGenerator::SetProgressTask(ProgressTask *task)
{
  m_ProgressTask = task;
}

mitk::ProgressTask *mitk::HistogramGenerator::GetProgressTask() const
{
  return m_ProgressTask;
}

void mitk::HistogramGenerator::ComputeHistogram()
{
  if (m_Image.IsNull())
    mitkThrow() << "Cannot compute a histogram without an image.";

  if (m_Histogram.IsNotNull() && m_Histogram->GetMTime() >= m_Image->GetMTime())
    return;

  if (m_Size <= 0)
    mitkThrow() << "Cannot compute a histogram of " << m_Size << " bins.";

  const auto pixelType = m_Image->GetPixelType();

  if (pixelType.GetNumberOfComponents() != 1)
    mitkThrow() << "Cannot compute a histogram of an image with " << pixelType.GetNumberOfComponents()
                << " components per pixel.";

  // Only a pipeline output can be out of date. An image without a source is
  // left alone, so that this may run on a worker while the GUI thread reads it.
  if (m_Image->GetSource().IsNotNull())
  {
    auto *image = const_cast<mitk::Image *>(m_Image.GetPointer());
    image->SetRequestedRegionToLargestPossibleRegion();
    image->Update();
  }

  if (nullptr != m_ProgressTask)
    m_ProgressTask->AddStepsToDo(RANGE_STEPS + BINNING_STEPS);

  // The statistics are cached on the image, so the range costs nothing where
  // they were asked for before, and nothing later where they were not.
  auto *statistics = m_Image->GetStatistics();
  const auto minimum = statistics->GetScalarValueMin(0);
  const auto maximum = statistics->GetScalarValueMax(0);

  if (nullptr != m_ProgressTask)
    m_ProgressTask->Progress(RANGE_STEPS);

  auto volume = m_Image->GetVolumeData(0);

  if (volume.IsNull())
    mitkThrow() << "Cannot compute a histogram of an image without voxels.";

  const ImageReadAccessor accessor(m_Image, volume.GetPointer());

  std::size_t numberOfValues = 1;

  for (unsigned int i = 0; i < 3; ++i)
    numberOfValues *= m_Image->GetDimension(i);

  const BinningInput input{accessor.GetData(), numberOfValues, minimum, maximum,
                           static_cast<unsigned int>(m_Size), m_ProgressTask};

  HistogramType::Pointer histogram;
  mitkPixelTypeMultiplex2(ComputeBins, pixelType, input, histogram);

  if (histogram.IsNull())
    mitkThrow() << "Cannot compute a histogram of pixel type " << pixelType.GetComponentTypeAsString() << ".";

  m_Histogram = histogram;
}

float mitk::HistogramGenerator::GetMaximumFrequency() const
{
  return CalculateMaximumFrequency(this->m_Histogram);
}

float mitk::HistogramGenerator::CalculateMaximumFrequency(const HistogramType *histogram)
{
  HistogramType::ConstIterator itr = histogram->Begin();
  HistogramType::ConstIterator end = histogram->End();

  float maxFreq = 0;
  while (itr != end)
  {
    maxFreq = std::max(maxFreq, static_cast<float>(itr.GetFrequency()));
    ++itr;
  }
  return maxFreq;
}
