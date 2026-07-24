/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkFormulaParser.h>
#include <mitkGenericParamModel.h>

class mitkGenericParamModelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGenericParamModelTestSuite);
  MITK_TEST(RepeatedEvaluationTest);
  MITK_TEST(FunctionStringChangeTest);
  MITK_TEST(CloneTest);
  MITK_TEST(UnknownVariableTest);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::GenericParamModel::Pointer m_Model;

  static mitk::ModelBase::TimeGridType MakeTimeGrid()
  {
    mitk::ModelBase::TimeGridType grid(5);

    for (unsigned int i = 0; i < grid.GetSize(); ++i)
    {
      grid[i] = i;
    }

    return grid;
  }

  static mitk::ModelBase::ParametersType MakeParameters(double a, double b)
  {
    mitk::ModelBase::ParametersType parameters(2);
    parameters[0] = a;
    parameters[1] = b;
    return parameters;
  }

  static void AssertSignal(const mitk::ModelBase::ModelResultType& signal,
    const std::vector<double>& expected)
  {
    CPPUNIT_ASSERT_EQUAL(expected.size(), static_cast<std::size_t>(signal.GetSize()));

    for (std::size_t i = 0; i < expected.size(); ++i)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(expected[i], signal[i], 1e-9);
    }
  }

public:
  void setUp() override
  {
    m_Model = mitk::GenericParamModel::New();
    m_Model->SetFunctionString("a + b*x");
    m_Model->SetNumberOfParameters(2);
    m_Model->SetTimeGrid(MakeTimeGrid());
  }

  void tearDown() override
  {
    m_Model = nullptr;
  }

  void RepeatedEvaluationTest()
  {
    // repeated evaluations on the same instance exercise the reused member
    // parser (the formula is compiled once and evaluated against updated
    // parameter values)
    AssertSignal(m_Model->GetSignal(MakeParameters(1.0, 2.0)), { 1.0, 3.0, 5.0, 7.0, 9.0 });
    AssertSignal(m_Model->GetSignal(MakeParameters(2.0, 0.5)), { 2.0, 2.5, 3.0, 3.5, 4.0 });
  }

  void FunctionStringChangeTest()
  {
    // changing the function string between evaluations must invalidate the
    // cached compiled formula
    AssertSignal(m_Model->GetSignal(MakeParameters(1.0, 2.0)), { 1.0, 3.0, 5.0, 7.0, 9.0 });

    m_Model->SetFunctionString("a*x");
    AssertSignal(m_Model->GetSignal(MakeParameters(3.0, 0.0)), { 0.0, 3.0, 6.0, 9.0, 12.0 });
  }

  void CloneTest()
  {
    // a clone's parser must be bound to the clone's own variable map, even
    // when the source was already evaluated (warm cache) before cloning
    AssertSignal(m_Model->GetSignal(MakeParameters(1.0, 2.0)), { 1.0, 3.0, 5.0, 7.0, 9.0 });

    auto clone = m_Model->Clone();
    clone->SetTimeGrid(MakeTimeGrid());

    AssertSignal(clone->GetSignal(MakeParameters(10.0, 1.0)), { 10.0, 11.0, 12.0, 13.0, 14.0 });
    AssertSignal(m_Model->GetSignal(MakeParameters(1.0, 2.0)), { 1.0, 3.0, 5.0, 7.0, 9.0 });
  }

  void UnknownVariableTest()
  {
    m_Model->SetFunctionString("a + q");
    CPPUNIT_ASSERT_THROW(m_Model->GetSignal(MakeParameters(1.0, 2.0)), mitk::FormulaParserException);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkGenericParamModel)
