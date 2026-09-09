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

#include <utility>

/**
 * \brief Test class for mitk::MedianAccumulator
 */
class mitkMedianAccumulatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMedianAccumulatorTestSuite);
  MITK_TEST(DenseMode_EvenN_ValuesEqualBounds);
  MITK_TEST(DenseMode_OddN);
  MITK_TEST(ValueMode_Float);
  MITK_TEST(ValueMode_WideIntegerRange);
  MITK_TEST(DenseMode_DegradesToValueModeOnOutOfRangeValue);
  MITK_TEST(Merge_DenseWithDense);
  MITK_TEST(Merge_DenseWithValues);
  MITK_TEST(Merge_ValuesWithValues);
  MITK_TEST(ComputeMedian_EmptyAccumulator_Throws);
  MITK_TEST(DefaultConstructed_WorksForIntegralType);
  CPPUNIT_TEST_SUITE_END();

public:
  void DenseMode_EvenN_ValuesEqualBounds()
  {
    // Issue 109: a two-voxel mask with values 78 and 152.
    mitk::MedianAccumulator<short> acc(78, 152);
    acc.Add(78);
    acc.Add(152);

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(2), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(115.0, acc.ComputeMedian(), mitk::eps);
  }

  void DenseMode_OddN()
  {
    mitk::MedianAccumulator<short> acc(78, 200);
    acc.Add(200);
    acc.Add(78);
    acc.Add(152);

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(3), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(152.0, acc.ComputeMedian(), mitk::eps);
  }

  void ValueMode_Float()
  {
    mitk::MedianAccumulator<float> acc(1.5, 11.0);
    acc.Add(10.0f);
    acc.Add(1.5f);
    acc.Add(11.0f);
    acc.Add(2.5f);

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

    CPPUNIT_ASSERT_EQUAL(itk::SizeValueType(4), acc.GetCount());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(7.5, acc.ComputeMedian(), mitk::eps);
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

  void ComputeMedian_EmptyAccumulator_Throws()
  {
    mitk::MedianAccumulator<int> acc;
    CPPUNIT_ASSERT_THROW(acc.ComputeMedian(), mitk::Exception);
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
