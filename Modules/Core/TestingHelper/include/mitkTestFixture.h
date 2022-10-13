/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKTESTFIXTURE_H
#define MITKTESTFIXTURE_H

#include <cppunit/TestFixture.h>
#include <mitkTestingConfig.h>

#include <itksys/SystemTools.hxx>

#include <string>
#include <vector>

namespace mitk
{
  /**
   * \brief Test fixture for parameterized tests
   *
   * This class is a drop-in replacement for CppUnit::TextFixture and
   * enables test methods to access individual parameters. You can also
   * invoke one method multiple times with different parameters.
   *
   *
   * The following simple example creates a single test without custom
   * parameters:
   *
   * \code
   * class MySimpleTestSuite : public mitk::TestFixture
   * {
   *   CPPUNIT_TEST_SUITE(MySimpleTestSuite);
   *   MITK_TEST(FivePlusFiveTest);
   *   CPPUNIT_TEST_SUITE_END();
   *
   * public:
   *   void FivePlusFiveTest()
   *   {
   *      CPPUNIT_ASSERT(5+5 == 10);
   *   }
   * };
   * MITK_TEST_SUITE_REGISTRATION(MySimpleTestSuite)
   * \endcode
   *
   *
   * The following example creates a test class containing only
   * one test method, but the associated test suite contains three tests,
   * using different parameters for each call of the same method. Use
   * the macro MITK_PARAMETERIZED_TEST_1 only if you know what you are
   * doing. If you are not sure, use MITK_TEST instead.
   *
   * \code
   * class MyTestSuite : public mitk::TestFixture
   * {
   *   CPPUNIT_TEST_SUITE(MyTestSuite);
   *   MITK_PARAMETERIZED_TEST_1(TestSomething, "One");
   *   MITK_PARAMETERIZED_TEST_1(TestSomething, "Two");
   *   MITK_PARAMETERIZED_TEST_1(TestSomething, "Three");
   *   CPPUNIT_TEST_SUITE_END();
   *
   * public:
   *
   *   void TestSomething()
   *   {
   *     std::vector<std::string> parameter = GetTestParameter();
   *     CPPUNIT_ASSERT(parameter.size() == 1);
   *     std::string testParam = parameter[0];
   *
   *     MITK_INFO << "Parameter: " << testParam;
   *   }
   * };
   * MITK_TEST_SUITE_REGISTRATION(MyTestSuite)
   * \endcode
   *
   * \sa MITK_PARAMETERIZED_TEST
   * \sa MITK_PARAMETERIZED_TEST_1
   */
  class TestFixture : public CppUnit::TestFixture
  {
  protected:
    /**
     * \brief Get parameters for this test fixture
     *
     * This method can be called in tests added via the MITK_PARAMETERIZED_TEST
     * macro or one of its variants.
     *
     * \return The list of \c std::string parameters passed to previous calls
     *         of the MITK_PARAMETERIZED_TEST macro or one of its variants.
     *
     */
    std::vector<std::string> GetTestParameter() const { return m_Parameter; }
    /**
     * \brief Get the absolute path for test data.
     *
     * \param testData The realative path in the MITK test data repository.
     *
     * \return The absolute path for the test data.
     */
    static std::string GetTestDataFilePath(const std::string &testData)
    {
      return GetTestDataFilePath(testData, MITK_DATA_DIR);
    }


    /**
     * \brief Get the absolute path for project specific test data.
     *
     * \param testData The realative path in the project specific test data repository.
     *
     * \return The absolute path for the project specific test data.
     */
    static std::string GetTestDataFilePath(const std::string &testData, const std::string &project )
    {
      if (itksys::SystemTools::FileIsFullPath(testData.c_str()))
        return testData;
      return project + "/" + testData;
    }

  private:
    template <class P>
    friend class TestCaller;

    std::vector<std::string> m_Parameter;
  };
}

#endif // MITKTESTFIXTURE_H
