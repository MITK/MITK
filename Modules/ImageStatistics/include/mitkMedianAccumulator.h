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
   * it is the middle value.
   *
   * Internally the accumulator either counts occurrences per value (dense
   * mode, available for integral pixel types with a bounded range known up
   * front) or collects the values themselves and selects the median with
   * std::nth_element (value mode, used for floating point pixel types and
   * whenever the declared range is too large to count densely). Which
   * strategy is used is an implementation detail; it does not affect the
   * result.
   *
   * ComputeMedian() reorders and releases the internal storage, so it must
   * only be called once, after all values have been added or merged in.
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
     * added. A value added outside the declared range is still handled
     * correctly; the accumulator then falls back to value mode (see Add()).
     * Floating point pixel types always use value mode.
     *
     * \param[in] lowerBound Lower bound of the expected value range.
     * \param[in] upperBound Upper bound of the expected value range.
     */
    MedianAccumulator(RealType lowerBound, RealType upperBound);

    /** \brief Add a value to the accumulator. */
    void Add(TPixel value);

    /**
     * \brief Merge another accumulator into this one.
     * \param[in] other The accumulator to merge in. Left in an unspecified but
     *            valid state afterwards.
     */
    void Merge(MedianAccumulator&& other);

    /** \brief Get the number of values added or merged in so far. */
    itk::SizeValueType GetCount() const;

    /**
     * \brief Compute and return the exact median of all accumulated values.
     *
     * Reorders and releases the internal storage; call this only once, after
     * all values have been added or merged in.
     *
     * \pre GetCount() > 0
     * \throw mitk::Exception if no value has been added.
     */
    RealType ComputeMedian();

  private:
    // 2^20 counts, 8 MB at 8 bytes per count; bounds the memory of dense mode
    // independently of how many values are added.
    static constexpr double MaxDenseRange = 1 << 20;

    void ConvertToValues();
    TPixel IndexToValue(std::size_t index) const;

    bool m_UseCounts = false;
    TPixel m_Offset{};
    std::vector<itk::SizeValueType> m_Counts;
    std::vector<TPixel> m_Values;
    itk::SizeValueType m_Count = 0;
  };

  template <typename TPixel>
  MedianAccumulator<TPixel>::MedianAccumulator(RealType lowerBound, RealType upperBound)
  {
    if constexpr (std::is_integral_v<TPixel>)
    {
      if (lowerBound <= upperBound)
      {
        const auto range = std::ceil(static_cast<double>(upperBound)) - std::floor(static_cast<double>(lowerBound)) + 1.0;

        if (range <= MaxDenseRange)
        {
          m_UseCounts = true;
          m_Offset = static_cast<TPixel>(std::floor(static_cast<double>(lowerBound)));
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
  void MedianAccumulator<TPixel>::Merge(MedianAccumulator&& other)
  {
    if (m_UseCounts && other.m_UseCounts && m_Offset == other.m_Offset && m_Counts.size() == other.m_Counts.size())
    {
      for (std::size_t index = 0; index < m_Counts.size(); ++index)
        m_Counts[index] += other.m_Counts[index];
    }
    else
    {
      this->ConvertToValues();
      other.ConvertToValues();

      if (m_Values.size() < other.m_Values.size())
        std::swap(m_Values, other.m_Values);

      m_Values.insert(m_Values.end(), other.m_Values.begin(), other.m_Values.end());
    }

    m_Count += other.m_Count;
  }

  template <typename TPixel>
  itk::SizeValueType MedianAccumulator<TPixel>::GetCount() const
  {
    return m_Count;
  }

  template <typename TPixel>
  typename MedianAccumulator<TPixel>::RealType MedianAccumulator<TPixel>::ComputeMedian()
  {
    if (0 == m_Count)
      mitkThrow() << "Cannot compute the median of an empty MedianAccumulator";

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

      m_Counts.clear();
      m_Counts.shrink_to_fit();
    }
    else
    {
      const auto k1It = m_Values.begin() + k1;
      std::nth_element(m_Values.begin(), k1It, m_Values.end());
      v1 = *k1It;
      v2 = (k2 != k1) ? *std::min_element(k1It + 1, m_Values.end()) : v1;

      m_Values.clear();
      m_Values.shrink_to_fit();
    }

    return (static_cast<RealType>(v1) + static_cast<RealType>(v2)) / 2.0;
  }
}

#endif
