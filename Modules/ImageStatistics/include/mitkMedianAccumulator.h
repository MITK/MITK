/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMedianAccumulator_h
#define mitkMedianAccumulator_h

#include <mitkExceptionMacro.h>

#include <itkIntTypes.h>
#include <itkNumericTraits.h>

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <limits>
#include <type_traits>
#include <utility>
#include <vector>

namespace mitk
{
  /**
   * \brief Accumulates pixel values and computes their exact median.
   *
   * Values are collected one at a time via Add() or by combining two
   * accumulators via Merge(), and the exact median of all accumulated
   * values is obtained via ComputeMedian(). For an even count the median is
   * the mean of the two middle values (numpy convention); for an odd count
   * it is the middle value. If any added value is NaN the median is NaN,
   * which is how the mean and the standard deviation of the same values
   * behave as well.
   *
   * ComputeMedian() finalizes the accumulator: it caches the result,
   * releases the accumulated values and leaves GetCount() at zero. Adding
   * to or merging into a finalized accumulator throws rather than silently
   * producing a stale or undefined result; calling ComputeMedian() again
   * returns the cached value.
   *
   * Internally the accumulator either counts occurrences per value (dense
   * mode, available for integral pixel types with a bounded range known up
   * front) or collects the values themselves and selects the median with
   * std::nth_element (value mode, used for floating point pixel types and
   * whenever the declared range is too large to count densely). Which
   * strategy is used does not affect the result; UsesDenseCounting()
   * reports it for diagnostics and tests.
   *
   * Memory use differs sharply between the two. Dense mode needs one count
   * per value in the declared range, occurring or not, and is therefore
   * independent of how many values are added, while value mode holds on to
   * every added value. Feeding a whole floating point image through an
   * accumulator thus allocates on the order of the image size; Reserve()
   * takes the edge off that when the number of values is known up front.
   * The fall back from dense to value mode is one-way, and Merge()
   * propagates it to the accumulator merged into.
   *
   * \tparam TPixel The pixel type of the accumulated values.
   */
  template <typename TPixel>
  class MedianAccumulator
  {
  public:
    using RealType = typename itk::NumericTraits<TPixel>::RealType;

    /** \brief Construct an accumulator in value mode. Works for any pixel type. */
    MedianAccumulator() = default;

    /**
     * \brief Construct an accumulator that may use dense counting.
     *
     * For an integral pixel type whose declared range [lowerBound, upperBound]
     * fits within MaxDenseRange, values are counted per bin instead of
     * collected, bounding memory use independently of how many values are
     * added. Bounds that TPixel cannot represent are clamped to its range. A
     * value added outside the resulting range is still handled correctly; the
     * accumulator then falls back to value mode (see Add()). Floating point
     * pixel types always use value mode.
     *
     * \param[in] lowerBound Lower bound of the expected value range.
     * \param[in] upperBound Upper bound of the expected value range.
     */
    MedianAccumulator(RealType lowerBound, RealType upperBound);

    /**
     * \brief Add a value to the accumulator.
     * \pre ComputeMedian() has not been called yet.
     * \throw mitk::Exception if the accumulator is already finalized.
     */
    void Add(TPixel value);

    /**
     * \brief Reserve room for the given number of values.
     *
     * A pure optimization that avoids repeated reallocation while values are
     * added. It takes effect in value mode only; dense mode allocates its
     * counts up front and ignores the hint.
     *
     * \param[in] count The number of values expected to be added.
     */
    void Reserve(itk::SizeValueType count);

    /**
     * \brief Merge another accumulator into this one.
     * \param[in] other The accumulator to merge in. Left empty afterwards, so
     *            merging it again contributes nothing.
     * \pre Neither accumulator is finalized and other is not this.
     * \throw mitk::Exception if either accumulator is already finalized or if
     *        an accumulator is merged into itself.
     */
    void Merge(MedianAccumulator&& other);

    /** \brief Get the number of values added or merged in so far. */
    itk::SizeValueType GetCount() const;

    /** \brief Check whether the accumulator counts per value instead of collecting them. */
    bool UsesDenseCounting() const;

    /**
     * \brief Compute and return the exact median of all accumulated values.
     *
     * The first call consumes the accumulated values and caches the result;
     * later calls return that cached result.
     *
     * \pre GetCount() > 0
     * \throw mitk::Exception if no value has been added.
     */
    RealType ComputeMedian();

  private:
    // 2^20 counts, 8 MB at 8 bytes per count; bounds the memory of dense mode
    // independently of how many values are added.
    static constexpr std::size_t MaxDenseRange = 1 << 20;

    void ConvertToValues();
    TPixel IndexToValue(std::size_t index) const;

    bool m_UseCounts = false;
    TPixel m_Offset{};
    std::vector<itk::SizeValueType> m_Counts;
    std::vector<TPixel> m_Values;
    itk::SizeValueType m_Count = 0;
    bool m_HasNaN = false;
    bool m_MedianComputed = false;
    RealType m_Median{};
  };

  template <typename TPixel>
  MedianAccumulator<TPixel>::MedianAccumulator(RealType lowerBound, RealType upperBound)
  {
    if constexpr (std::is_integral_v<TPixel>)
    {
      // Bounds that TPixel cannot represent would make the offset cast below
      // undefined. Clamping keeps dense counting available for a range that
      // merely overshoots the pixel type, which is what a caller passing a
      // nominal intensity window does.
      const auto lower = std::max(lowerBound, static_cast<RealType>(itk::NumericTraits<TPixel>::NonpositiveMin()));
      const auto upper = std::min(upperBound, static_cast<RealType>(itk::NumericTraits<TPixel>::max()));

      if (lower <= upper)
      {
        const auto range = std::ceil(static_cast<double>(upper)) - std::floor(static_cast<double>(lower)) + 1.0;

        if (range <= static_cast<double>(MaxDenseRange))
        {
          m_UseCounts = true;
          m_Offset = static_cast<TPixel>(std::floor(static_cast<double>(lower)));
          m_Counts.resize(static_cast<std::size_t>(range), 0);
        }
      }
    }
  }

  template <typename TPixel>
  TPixel MedianAccumulator<TPixel>::IndexToValue(std::size_t index) const
  {
    return static_cast<TPixel>(static_cast<long long>(m_Offset) + static_cast<long long>(index));
  }

  template <typename TPixel>
  void MedianAccumulator<TPixel>::ConvertToValues()
  {
    if (m_UseCounts)
    {
      m_Values.reserve(m_Count);

      for (std::size_t index = 0; index < m_Counts.size(); ++index)
        m_Values.insert(m_Values.end(), m_Counts[index], this->IndexToValue(index));

      m_Counts.clear();
      m_Counts.shrink_to_fit();
      m_UseCounts = false;
    }
  }

  template <typename TPixel>
  void MedianAccumulator<TPixel>::Add(TPixel value)
  {
    if (m_MedianComputed)
      mitkThrow() << "Cannot add to a MedianAccumulator whose median has already been computed";

    if constexpr (std::is_floating_point_v<TPixel>)
    {
      // NaN has no place in a sorted order, so std::nth_element would be
      // undefined on values containing one. Remember it and answer NaN.
      if (std::isnan(value))
        m_HasNaN = true;
    }

    if (m_UseCounts)
    {
      const auto index = static_cast<long long>(value) - static_cast<long long>(m_Offset);

      if (index < 0 || static_cast<std::size_t>(index) >= m_Counts.size())
        this->ConvertToValues();
      else
        ++m_Counts[static_cast<std::size_t>(index)];
    }

    if (!m_UseCounts)
      m_Values.push_back(value);

    ++m_Count;
  }

  template <typename TPixel>
  void MedianAccumulator<TPixel>::Reserve(itk::SizeValueType count)
  {
    if (!m_UseCounts)
      m_Values.reserve(count);
  }

  template <typename TPixel>
  void MedianAccumulator<TPixel>::Merge(MedianAccumulator&& other)
  {
    if (this == &other)
      mitkThrow() << "Cannot merge a MedianAccumulator into itself";

    if (m_MedianComputed || other.m_MedianComputed)
      mitkThrow() << "Cannot merge a MedianAccumulator whose median has already been computed";

    m_HasNaN = m_HasNaN || other.m_HasNaN;

    if (m_UseCounts && other.m_UseCounts && m_Offset == other.m_Offset && m_Counts.size() == other.m_Counts.size())
    {
      for (std::size_t index = 0; index < m_Counts.size(); ++index)
        m_Counts[index] += other.m_Counts[index];
    }
    else
    {
      this->ConvertToValues();
      other.ConvertToValues();

      // Keep whichever buffer can take the combined values with the least
      // reallocation. Comparing capacities rather than sizes is what keeps a
      // destination that was sized up front via Reserve() from being swapped
      // away by the first, still small, accumulator merged into it.
      if (m_Values.capacity() < other.m_Values.capacity())
        std::swap(m_Values, other.m_Values);

      // Leaving the growth to the vector, rather than reserving the exact
      // total here, is what keeps merging many accumulators into one linear
      // in the total number of values.
      m_Values.insert(m_Values.end(), other.m_Values.begin(), other.m_Values.end());
    }

    m_Count += other.m_Count;

    other = MedianAccumulator();
  }

  template <typename TPixel>
  itk::SizeValueType MedianAccumulator<TPixel>::GetCount() const
  {
    return m_Count;
  }

  template <typename TPixel>
  bool MedianAccumulator<TPixel>::UsesDenseCounting() const
  {
    return m_UseCounts;
  }

  template <typename TPixel>
  typename MedianAccumulator<TPixel>::RealType MedianAccumulator<TPixel>::ComputeMedian()
  {
    if (m_MedianComputed)
      return m_Median;

    if (0 == m_Count)
      mitkThrow() << "Cannot compute the median of an empty MedianAccumulator";

    if (m_HasNaN)
    {
      m_Median = std::numeric_limits<RealType>::quiet_NaN();
    }
    else
    {
      const auto k1 = (m_Count - 1) / 2;
      const auto k2 = m_Count / 2;

      TPixel v1{};
      TPixel v2{};

      if (m_UseCounts)
      {
        itk::SizeValueType cumulative = 0;
        bool foundV1 = false;

        for (std::size_t index = 0; index < m_Counts.size(); ++index)
        {
          cumulative += m_Counts[index];

          if (!foundV1 && cumulative > k1)
          {
            v1 = this->IndexToValue(index);
            foundV1 = true;
          }

          if (cumulative > k2)
          {
            v2 = this->IndexToValue(index);
            break;
          }
        }
      }
      else
      {
        const auto k1It = m_Values.begin() + k1;
        std::nth_element(m_Values.begin(), k1It, m_Values.end());
        v1 = *k1It;
        v2 = (k2 != k1) ? *std::min_element(k1It + 1, m_Values.end()) : v1;
      }

      m_Median = (static_cast<RealType>(v1) + static_cast<RealType>(v2)) / 2.0;
    }

    m_Counts.clear();
    m_Counts.shrink_to_fit();
    m_Values.clear();
    m_Values.shrink_to_fit();

    m_UseCounts = false;
    m_Count = 0;
    m_HasNaN = false;
    m_MedianComputed = true;

    return m_Median;
  }
}

#endif
