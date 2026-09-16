/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMedianAccumulator.h>
#include <mitkNumericConstants.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <utility>
#include <vector>

/**
 * \brief Test class for mitk::MedianAccumulator
 */
class mitkMedianAccumulatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMedianAccumulatorTestSuite);
  MITK_TEST(DenseMode_EvenN_ValuesEqualBounds);
  MITK_TEST(DenseMode_OddN);
  MITK_TEST(DenseMode_NegativeOffset);
  MITK_TEST(ValueMode_Float);
  MITK_TEST(ValueMode_WideIntegerRange);
  MITK_TEST(DenseMode_DegradesToValueModeOnOutOfRangeValue);
  MITK_TEST(BoundsOutsidePixelTypeRange_ClampToDenseMode);
  MITK_TEST(BoundsBelowUnsignedPixelTypeRange_ClampToDenseMode);
  MITK_TEST(ValueMode_NaN_YieldsNaNMedian);
  MITK_TEST(Merge_DenseWithDense);
  MITK_TEST(Merge_DenseWithDense_DifferentBounds);
  MITK_TEST(Merge_DenseWithValues);
  MITK_TEST(Merge_ValuesWithDense);
  MITK_TEST(Merge_ValuesWithValues);
  MITK_TEST(Merge_WithEmptyAccumulator);
  MITK_TEST(Merge_LeavesSourceEmpty);
  MITK_TEST(Merge_PropagatesNaN);
  MITK_TEST(Merge_IntoItself_Throws);
  MITK_TEST(Finalized_AddAndMergeThrow);
  MITK_TEST(Reserve_DoesNotAffectResult);
  MITK_TEST(ComputeMedian_EmptyAccumulator_Throws);
  MITK_TEST(ComputeMedian_RepeatedCalls_ReturnTheSameValue);
  MITK_TEST(BothModes_AgreeWithSortedReference);
  MITK_TEST(DefaultConstructed_WorksForIntegralType);
  CPPUNIT_TEST_SUITE_END();

private:
  // median of a copy of the values, straight from the definition, to compare
  // the accumulator against
  static double SortedReferenceMedian(std::vector<short> values)
  {
    std::sort(values.begin(), values.end());

    return 0.5 * (values[(values.size() - 1) / 2] + values[values.size() / 2]);
  }

public:
  void DenseMode_EvenN_ValuesEqualBounds()
  {
    // Issue 109: a two-voxel mask with values 78 and 152.
    mitk::MedianAccumulator<short> acc(78, 152);
    acc.Add(78);
    acc.Add(152);

    CPPUNIT_ASSERT(acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(2), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(115.0, acc.ComputeMedian(), mitk::eps);
  }

  void DenseMode_OddN()
  {
    mitk::MedianAccumulator<short> acc(78, 200);
    acc.Add(200);
    acc.Add(78);
    acc.Add(152);

    CPPUNIT_ASSERT(acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(152.0, acc.ComputeMedian(), mitk::eps);
  }

  void DenseMode_NegativeOffset()
  {
    // the usual CT case: dense counting across a range whose offset is negative
    mitk::MedianAccumulator<short> acc(-1024, 3071);
    acc.Add(3071);
    acc.Add(-1024);
    acc.Add(-500);
    acc.Add(0);

    CPPUNIT_ASSERT(acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(-250.0, acc.ComputeMedian(), mitk::eps);
  }

  void ValueMode_Float()
  {
    mitk::MedianAccumulator<float> acc(1.5, 11.0);
    acc.Add(10.0f);
    acc.Add(1.5f);
    acc.Add(11.0f);
    acc.Add(2.5f);

    CPPUNIT_ASSERT(!acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(6.25, acc.ComputeMedian(), mitk::eps);
  }

  void ValueMode_WideIntegerRange()
  {
    // The declared range exceeds MaxDenseRange, so this falls back to value mode.
    mitk::MedianAccumulator<int> acc(0, 1 << 30);
    acc.Add(30);
    acc.Add(10);
    acc.Add(20);

    CPPUNIT_ASSERT(!acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, acc.ComputeMedian(), mitk::eps);
  }

  void DenseMode_DegradesToValueModeOnOutOfRangeValue()
  {
    mitk::MedianAccumulator<short> acc(0, 10);
    acc.Add(0);
    acc.Add(5);
    acc.Add(10);
    acc.Add(500); // outside the declared range, forces conversion to value mode

    CPPUNIT_ASSERT(!acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.5, acc.ComputeMedian(), mitk::eps);
  }

  void BoundsOutsidePixelTypeRange_ClampToDenseMode()
  {
    // short cannot represent these bounds, so casting them would be undefined.
    // Clamping to the pixel type keeps dense counting rather than buffering
    // every value.
    mitk::MedianAccumulator<short> acc(-100000.0, 100000.0);
    acc.Add(10);
    acc.Add(20);
    acc.Add(30);
    acc.Add(200);

    CPPUNIT_ASSERT(acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(25.0, acc.ComputeMedian(), mitk::eps);
  }

  void BoundsBelowUnsignedPixelTypeRange_ClampToDenseMode()
  {
    // a negative lower bound on an unsigned pixel type, as the texture
    // analysis test passes it
    mitk::MedianAccumulator<unsigned short> acc(-10.0, 10.0);
    acc.Add(2);
    acc.Add(4);
    acc.Add(6);

    CPPUNIT_ASSERT(acc.UsesDenseCounting());
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, acc.ComputeMedian(), mitk::eps);
  }

  void ValueMode_NaN_YieldsNaNMedian()
  {
    // NaN has no place in a sorted order, so the median is NaN, just as the
    // mean and the standard deviation of the same values are
    mitk::MedianAccumulator<float> acc;
    acc.Add(1.0f);
    acc.Add(std::numeric_limits<float>::quiet_NaN());
    acc.Add(3.0f);

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT(std::isnan(acc.ComputeMedian()));
  }

  void Merge_DenseWithDense()
  {
    mitk::MedianAccumulator<int> acc1(0, 10);
    acc1.Add(2);
    acc1.Add(4);

    mitk::MedianAccumulator<int> acc2(0, 10);
    acc2.Add(6);
    acc2.Add(8);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Merge_DenseWithDense_DifferentBounds()
  {
    // mismatched ranges cannot be added bin by bin, so both sides convert
    mitk::MedianAccumulator<int> acc1(0, 10);
    acc1.Add(2);
    acc1.Add(4);

    mitk::MedianAccumulator<int> acc2(100, 110);
    acc2.Add(106);
    acc2.Add(108);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(55.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Merge_DenseWithValues()
  {
    mitk::MedianAccumulator<int> acc1(0, 10);
    acc1.Add(2);
    acc1.Add(4);

    mitk::MedianAccumulator<int> acc2; // default-constructed, always value mode
    acc2.Add(6);
    acc2.Add(8);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Merge_ValuesWithDense()
  {
    mitk::MedianAccumulator<int> acc1; // default-constructed, always value mode
    acc1.Add(2);
    acc1.Add(4);

    mitk::MedianAccumulator<int> acc2(0, 10);
    acc2.Add(6);
    acc2.Add(8);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Merge_ValuesWithValues()
  {
    mitk::MedianAccumulator<int> acc1;
    acc1.Add(1);
    acc1.Add(3);

    mitk::MedianAccumulator<int> acc2;
    acc2.Add(5);
    acc2.Add(7);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(4.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Merge_WithEmptyAccumulator()
  {
    mitk::MedianAccumulator<int> acc1(0, 10);
    acc1.Add(3);
    acc1.Add(5);
    acc1.Add(9);

    mitk::MedianAccumulator<int> empty(0, 10);
    acc1.Merge(std::move(empty));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc1.ComputeMedian(), mitk::eps);

    mitk::MedianAccumulator<int> acc2(0, 10);
    mitk::MedianAccumulator<int> source(0, 10);
    source.Add(3);
    source.Add(5);
    source.Add(9);

    acc2.Merge(std::move(source));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc2.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc2.ComputeMedian(), mitk::eps);
  }

  void Merge_LeavesSourceEmpty()
  {
    mitk::MedianAccumulator<int> acc1(0, 10);
    acc1.Add(2);
    acc1.Add(4);

    mitk::MedianAccumulator<int> acc2(0, 10);
    acc2.Add(6);
    acc2.Add(8);

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(0), acc2.GetCount());

    // merging the drained accumulator again must not count its values twice
    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc1.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, acc1.ComputeMedian(), mitk::eps);
  }

  void Reserve_DoesNotAffectResult()
  {
    mitk::MedianAccumulator<float> values;
    values.Reserve(4);
    values.Add(1.0f);
    values.Add(2.0f);
    values.Add(3.0f);
    values.Add(4.0f);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.5, values.ComputeMedian(), mitk::eps);

    mitk::MedianAccumulator<short> dense(0, 10);
    dense.Reserve(100); // ignored in dense mode
    dense.Add(4);
    dense.Add(6);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, dense.ComputeMedian(), mitk::eps);
  }

  void ComputeMedian_EmptyAccumulator_Throws()
  {
    mitk::MedianAccumulator<int> acc;
    CPPUNIT_ASSERT_THROW(acc.ComputeMedian(), mitk::Exception);
  }

  void ComputeMedian_RepeatedCalls_ReturnTheSameValue()
  {
    mitk::MedianAccumulator<short> dense(0, 10);
    dense.Add(1);
    dense.Add(5);
    dense.Add(9);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, dense.ComputeMedian(), mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, dense.ComputeMedian(), mitk::eps);

    mitk::MedianAccumulator<float> values;
    values.Add(1.0f);
    values.Add(5.0f);
    values.Add(9.0f);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, values.ComputeMedian(), mitk::eps);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, values.ComputeMedian(), mitk::eps);
  }

  void BothModes_AgreeWithSortedReference()
  {
    std::mt19937 generator(42);
    std::uniform_int_distribution<int> distribution(-1024, 3071);

    for (std::size_t count : { std::size_t(4999), std::size_t(5000) })
    {
      mitk::MedianAccumulator<short> dense(-1024, 3071);
      mitk::MedianAccumulator<short> values;
      std::vector<short> reference;
      reference.reserve(count);

      for (std::size_t i = 0; i < count; ++i)
      {
        const auto value = static_cast<short>(distribution(generator));
        dense.Add(value);
        values.Add(value);
        reference.push_back(value);
      }

      const auto expected = SortedReferenceMedian(reference);

      CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, dense.ComputeMedian(), mitk::eps);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(expected, values.ComputeMedian(), mitk::eps);
    }
  }

  void Merge_PropagatesNaN()
  {
    mitk::MedianAccumulator<double> acc1;
    acc1.Add(1.0);
    acc1.Add(2.0);

    mitk::MedianAccumulator<double> acc2;
    acc2.Add(std::numeric_limits<double>::quiet_NaN());

    acc1.Merge(std::move(acc2));

    CPPUNIT_ASSERT(std::isnan(acc1.ComputeMedian()));
  }

  void Merge_IntoItself_Throws()
  {
    mitk::MedianAccumulator<short> acc(0, 10);
    acc.Add(4);

    auto& itself = acc;
    CPPUNIT_ASSERT_THROW(acc.Merge(std::move(itself)), mitk::Exception);
  }

  void Finalized_AddAndMergeThrow()
  {
    mitk::MedianAccumulator<short> finalized(0, 10);
    finalized.Add(4);
    finalized.Add(6);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, finalized.ComputeMedian(), mitk::eps);
    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(0), finalized.GetCount());
    CPPUNIT_ASSERT_THROW(finalized.Add(8), mitk::Exception);

    mitk::MedianAccumulator<short> source(0, 10);
    source.Add(2);
    CPPUNIT_ASSERT_THROW(finalized.Merge(std::move(source)), mitk::Exception);

    // and the other way round, where the phantom count of the finalized source
    // would otherwise make the destination index past its own values
    mitk::MedianAccumulator<short> destination(0, 10);
    destination.Add(4);

    mitk::MedianAccumulator<short> finalizedSource(0, 10);
    finalizedSource.Add(2);
    finalizedSource.ComputeMedian();

    CPPUNIT_ASSERT_THROW(destination.Merge(std::move(finalizedSource)), mitk::Exception);
  }

  void DefaultConstructed_WorksForIntegralType()
  {
    mitk::MedianAccumulator<short> acc;
    acc.Add(3);
    acc.Add(1);
    acc.Add(2);

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, acc.ComputeMedian(), mitk::eps);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMedianAccumulator)
