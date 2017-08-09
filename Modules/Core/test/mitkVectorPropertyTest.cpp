/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkVectorProperty.h"

CPPUNIT_NS_BEGIN

/**
 \brief cppunit Test helper for std::vector types.
*/
template <typename DATA>
struct assertion_traits<std::vector<DATA>> // specialization for the std::string type
{
  static bool equal(const std::vector<DATA> &x, const std::vector<DATA> &y) { return x == y; }
  static std::string toString(const std::vector<DATA> &values)
  {
    OStringStream ost;
    for (auto v : values)
      ost << "'" << v << "' "; // adds quote around the string to see whitespace
    return ost.str();
  }
};

/**
 \brief cppunit Test helper for mitk::VectorProperty
*/
template <typename DATA>
struct assertion_traits<mitk::VectorProperty<DATA>> // specialization for the std::string type
{
  static bool equal(const mitk::VectorProperty<DATA> &x, const mitk::VectorProperty<DATA> &y)
  {
    return x == y; // use BaseProperty implementation of things
  }

  static std::string toString(const mitk::VectorProperty<DATA> &prop) { return prop.GetValueAsString(); }
};

CPPUNIT_NS_END

/**
 \brief Test for VectorProperty
*/
class mitkVectorPropertyTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkVectorPropertyTestSuite);
  MITK_TEST(Instantiate);
  MITK_TEST(TestSetGet);
  MITK_TEST(TestIsEqual);
  MITK_TEST(TestClone);
  CPPUNIT_TEST_SUITE_END();

  mitk::DoubleVectorProperty::Pointer m_DoubleData;
  mitk::IntVectorProperty::Pointer m_IntData;

public:
  void setUp() override
  {
    m_DoubleData = mitk::DoubleVectorProperty::New();
    m_IntData = mitk::IntVectorProperty::New();
  }

  void tearDown() override
  {
    m_DoubleData = nullptr;
    m_IntData = nullptr;
  }

  /// Done by setup/tearDown, verifies that classes can be instantiated.
  void Instantiate() {}
  template <typename T>
  std::vector<T> MakeSimpleList()
  {
    std::vector<T> data;

    data.push_back(-2);
    data.push_back(1);
    data.push_back(3);

    return data;
  }

  /// Verifies that GetValue() return what we provided by Setalue()
  template <typename T>
  void TestSetGet(mitk::VectorProperty<T> &prop)
  {
    std::vector<T> data = MakeSimpleList<T>();

    prop.SetValue(data);
    std::vector<T> stored = prop.GetValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Result of GetValue() should equal parameter of SetValue(data)", data, stored);
  }

  void TestSetGet()
  {
    TestSetGet(*m_DoubleData);
    TestSetGet(*m_IntData);
  }

  /// Verifies that IsEqual() notices when elements are different or missing.
  template <typename T>
  void TestIsEqual(mitk::VectorProperty<T> &prop)
  {
    std::vector<T> data = MakeSimpleList<T>();
    prop.SetValue(data);

    std::vector<T> modifiedData(data);
    modifiedData.back() = -modifiedData.back(); // change last element
    typename mitk::VectorProperty<T>::Pointer modifiedProperty = mitk::VectorProperty<T>::New();
    modifiedProperty->SetValue(modifiedData);

    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("Modified list shall be recognized by IsEqual()",
                                          CPPUNIT_ASSERT_EQUAL(*modifiedProperty, prop));

    modifiedData.pop_back(); // remove last element
    modifiedProperty->SetValue(modifiedData);
    CPPUNIT_ASSERT_ASSERTION_FAIL_MESSAGE("Removed element shall be recognized by IsEqual()",
                                          CPPUNIT_ASSERT_EQUAL(*modifiedProperty, prop));
  }

  void TestIsEqual()
  {
    TestIsEqual(*m_DoubleData);
    TestIsEqual(*m_IntData);
  }

  /// Verifies that Clone() creates an equal copy.
  template <typename T>
  void TestClone(mitk::VectorProperty<T> &prop)
  {
    std::vector<T> data = MakeSimpleList<T>();
    prop.SetValue(data);

    typename mitk::VectorProperty<T>::Pointer clone = prop.Clone();
    CPPUNIT_ASSERT(clone.IsNotNull());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Result of Clone() shall equal original property", *clone, prop);

    std::vector<T> origData = prop.GetValue();
    std::vector<T> cloneData = clone->GetValue();

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clone shall have a copy of original data", cloneData, origData);
  }

  void TestClone()
  {
    TestClone(*m_DoubleData);
    TestClone(*m_IntData);
  }

}; // class

MITK_TEST_SUITE_REGISTRATION(mitkVectorProperty)
