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

#ifndef MITKTESTFIXTURE_H
#define MITKTESTFIXTURE_H

#include <cppunit/TestFixture.h>
#include <mitkTestingConfig.h>

#include <itksys/SystemTools.hxx>

#include <vector>
#include <string>

namespace mitk {

/**
 * \brief Test fixture for parameterized tests
 *
 * This class is a drop-in replacement for CppUnit::TextFixture and
 * enables test methods to access individual parameters. You can also
 * invoke one method multiple times with different parameters.
 *
 * The following example creates a test class containing only
 * one test method, but the associated test suite contains three tests,
 * using different parameters for each call of the same method.
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
 * private:
 *   std::string testParam;
 *
 * public:
 *   void setUpParameter(const std::vector<std::string>& parameter)
 *   {
 *     CPPUNIT_ASSERT(parameter.size() == 1);
 *     testParam = parameter[0];
 *   }
 *
 *   void TestSomething()
 *   {
 *      MITK_INFO << "Parameter: " << testParam;
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

public:

  /**
   * \brief Set up parameters for this text fixture
   *
   * This method is called for tests added via the MITK_PARAMETERZIED_TEST
   * macro or one of its variants. You should override this method and
   * set-up the test fixture using the supplied parameters.
   *
   */
  virtual void setUpParameter(const std::vector<std::string>& parameter) {}

protected:

  static std::string getTestDataFilePath(const std::string& testData)
  {
    if (itksys::SystemTools::FileIsFullPath(testData.c_str())) return testData;
    return std::string(MITK_DATA_DIR) + "/" + testData;
  }
};

}

#endif // MITKTESTFIXTURE_H
