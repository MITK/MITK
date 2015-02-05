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

#ifndef mitkTestingMacros_h
#define mitkTestingMacros_h

#include <exception>
#include <string>
#include <iostream>

#include <itkMacro.h>
#include <mitkTestManager.h>
#include <mitkTestCaller.h>
#include <mitkException.h>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

namespace mitk {
  /** @brief Indicate a failed test. */
  class TestFailedException : public std::exception {
    public:
      TestFailedException() {}
  };
}

/**
 * @brief Output some text without generating a terminating newline. Include
 *
 * @ingroup MITKTestingAPI
 */
#define MITK_TEST_OUTPUT_NO_ENDL(x) \
  std::cout x ;

/**
 * @brief Output some text.
 *
 * @ingroup MITKTestingAPI
 */
#define MITK_TEST_OUTPUT(x) \
  MITK_TEST_OUTPUT_NO_ENDL(x << "\n")

/**
 * @brief Do some general test preparations. Must be called first in the
 *    main test function.
 *
 *  @deprecatedSince{2013_09} Use MITK_TEST_SUITE_REGISTRATION instead.
 *  @ingroup MITKTestingAPI
 */
#define MITK_TEST_BEGIN(testName)      \
  std::string mitkTestName(#testName);   \
  mitk::TestManager::GetInstance()->Initialize();  \
  try {

/**
 * @brief Fail and finish test with message MSG
 *
 * @deprecatedSince{2013_09} Use CPPUNIT_FAIL instead
 * @ingroup MITKTestingAPI
 */
#define MITK_TEST_FAILED_MSG(MSG)  \
  MITK_TEST_OUTPUT(MSG) \
  throw mitk::TestFailedException();

/**
 * @brief Must be called last in the main test function.
 *
 * @deprecatedSince{2013_09} Use MITK_TEST_SUITE_REGISTRATION instead.
 * @ingroup MITKTestingAPI
 */
#define MITK_TEST_END()                                   \
  } catch (const mitk::TestFailedException&) {                      \
    MITK_TEST_OUTPUT(<< "Further test execution skipped.") \
    mitk::TestManager::GetInstance()->TestFailed(); \
  } catch (const std::exception& ex) { \
    MITK_TEST_OUTPUT(<< "std::exception occured " << ex.what()) \
    mitk::TestManager::GetInstance()->TestFailed(); \
  }                                                     \
  if (mitk::TestManager::GetInstance()->NumberOfFailedTests() > 0) {  \
    MITK_TEST_OUTPUT(<< mitkTestName << ": [DONE FAILED] , subtests passed: " << \
    mitk::TestManager::GetInstance()->NumberOfPassedTests() << " failed: " << \
    mitk::TestManager::GetInstance()->NumberOfFailedTests() )    \
    return EXIT_FAILURE;                                  \
  } else {                                                \
    MITK_TEST_OUTPUT(<< mitkTestName << ": "              \
      << mitk::TestManager::GetInstance()->NumberOfPassedTests() \
      << " tests [DONE PASSED]")    \
    return EXIT_SUCCESS;                                  \
  }                                                       \

/**
 * @deprecatedSince{2013_09} Use CPPUNIT_ASSERT or CPPUNIT_ASSERT_MESSAGE instead.
 */
#define MITK_TEST_CONDITION(COND,MSG) \
  MITK_TEST_OUTPUT_NO_ENDL(<< MSG) \
  if ( ! (COND) ) {        \
    mitk::TestManager::GetInstance()->TestFailed(); \
    MITK_TEST_OUTPUT(<< " [FAILED]\n" << "In " << __FILE__        \
                     << ", line " << __LINE__    \
                     << ":  " #COND " : [FAILED]")    \
  } else {                         \
    MITK_TEST_OUTPUT(<< " [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
 }

/**
 * @deprecatedSince{2013_09} Use CPPUNIT_ASSERT or CPPUNIT_ASSERT_MESSAGE instead.
 */
#define MITK_TEST_CONDITION_REQUIRED(COND,MSG) \
  MITK_TEST_OUTPUT_NO_ENDL(<< MSG) \
  if ( ! (COND) ) {        \
    MITK_TEST_FAILED_MSG(<< " [FAILED]\n" << "  +--> in " << __FILE__  \
                     << ", line " << __LINE__                      \
                     << ", expression is false: \"" #COND "\"")    \
  } else {                         \
    MITK_TEST_OUTPUT(<< " [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
 }

/**
 * \brief Begin block which should be checked for exceptions
 *
 * @deprecatedSince{2013_09} Use CPPUNIT_ASSERT_THROW instead.
 * @ingroup MITKTestingAPI
 *
 * This macro, together with MITK_TEST_FOR_EXCEPTION_END, can be used
 * to test whether a code block throws an expected exception. The test FAILS if the
 * exception is NOT thrown. A simple example:
 *

 MITK_TEST_FOR_EXCEPTION_BEGIN(itk::ImageFileReaderException)
    typedef itk::ImageFileReader< itk::Image<unsigned char,2> > ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName("/tmp/not-existing");
    reader->Update();
 MITK_TEST_FOR_EXCEPTION_END(itk::ImageFileReaderException)

 *
 */
#define MITK_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS) \
  try {

/**
 * @deprecatedSince{2013_09}
 */
#define MITK_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS) \
    mitk::TestManager::GetInstance()->TestFailed(); \
    MITK_TEST_OUTPUT( << "Expected an '" << #EXCEPTIONCLASS << "' exception. [FAILED]") \
  } \
  catch (EXCEPTIONCLASS) { \
    MITK_TEST_OUTPUT( << "Caught an expected '" << #EXCEPTIONCLASS  \
                      << "' exception. [PASSED]") \
    mitk::TestManager::GetInstance()->TestPassed(); \
  }


/**
 * @brief Simplified version of MITK_TEST_FOR_EXCEPTION_BEGIN / END for
 * a single statement
 *
 * @deprecatedSince{2013_09} Use CPPUNIT_ASSERT_THROW instead.
 * @ingroup MITKTestingAPI
 */
#define MITK_TEST_FOR_EXCEPTION(EXCEPTIONCLASS, STATEMENT) \
  MITK_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS) \
  STATEMENT ; \
  MITK_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS)

/**
 * @brief Testing macro to test if two objects are equal.
 *
 * @ingroup MITKTestingAPI
 *
 * This macro uses mitk::eps and the corresponding mitk::Equal methods for all
 * comparisons and will give verbose output on the dashboard/console.
 * Feel free to implement mitk::Equal for your own datatype or purpose.
 *
 * @deprecatedSince{2013_09} Use MITK_ASSERT_EQUAL instead.
 *
 * @param OBJ1 First object.
 * @param OBJ2 Second object.
 * @param MSG Message to appear with the test.
 */
#define MITK_TEST_EQUAL(OBJ1,OBJ2,MSG) \
  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(OBJ1, OBJ2, mitk::eps, true)==true, MSG)

/**
 * @brief Testing macro to test if two objects are equal.
 *
 * @ingroup MITKTestingAPI
 *
 * This macro uses mitk::eps and the corresponding mitk::Equal methods for all
 * comparisons and will give verbose output on the dashboard/console.
 * Feel free to implement mitk::Equal for your own datatype or purpose.
 *
 * @param EXPECTED First object.
 * @param ACTUAL Second object.
 * @param MSG Message to appear with the test.
 * @throw Throws mitkException if a NULL pointer is given as input.
 */
#define MITK_ASSERT_EQUAL(EXPECTED, ACTUAL, MSG) \
  if(((EXPECTED).IsNull()) || ((ACTUAL).IsNull())) { \
    mitkThrow() << "mitk::Equal does not work with NULL pointer input."; \
  } \
  CPPUNIT_ASSERT_MESSAGE(MSG, mitk::Equal(*(EXPECTED), *(ACTUAL), mitk::eps, true))

/**
 * @brief Testing macro to test if two objects are not equal.
 *
 * @ingroup MITKTestingAPI
 *
 * This macro uses mitk::eps and the corresponding mitk::Equal methods for all
 * comparisons and will give verbose output on the dashboard/console.
 *
 * @deprecatedSince{2013_09} Use MITK_ASSERT_NOT_EQUAL instead.
 *
 * @param OBJ1 First object.
 * @param OBJ2 Second object.
 * @param MSG Message to appear with the test.
 *
 * \sa MITK_TEST_EQUAL
 */
#define MITK_TEST_NOT_EQUAL(OBJ1,OBJ2,MSG) \
  CPPUNIT_ASSERT_MESSAGE(MSG, !mitk::Equal(*(OBJ1), *(OBJ2), mitk::eps, true))

/**
 * @brief Testing macro to test if two objects are not equal.
 *
 * @ingroup MITKTestingAPI
 *
 * This macro uses mitk::eps and the corresponding mitk::Equal methods for all
 * comparisons and will give verbose output on the dashboard/console.
 *
 * @param OBJ1 First object.
 * @param OBJ2 Second object.
 * @param MSG Message to appear with the test.
 * @throw Throws mitkException if a NULL pointer is given as input.
 *
 * \sa MITK_ASSERT_EQUAL
 */
#define MITK_ASSERT_NOT_EQUAL(OBJ1, OBJ2, MSG) \
  if(((OBJ1).IsNull()) || ((OBJ2).IsNull())) { \
    mitkThrow() << "mitk::Equal does not work with NULL pointer input."; \
  } \
  CPPUNIT_ASSERT_MESSAGE(MSG, !mitk::Equal(*(OBJ1), *(OBJ2), mitk::eps, true))

/**
 * @brief Registers the given test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * @param TESTSUITE_NAME The name of the test suite class, without "TestSuite"
 *        at the end.
 */
#define MITK_TEST_SUITE_REGISTRATION(TESTSUITE_NAME)         \
  int TESTSUITE_NAME ## Test(int /*argc*/, char* /*argv*/[]) \
  {                                                          \
    CppUnit::TextUi::TestRunner runner;                      \
    runner.addTest(TESTSUITE_NAME ## TestSuite::suite());    \
    return runner.run() ? 0 : 1;                             \
  }

/**
 * @brief Adds a test to the current test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * Use this macro after the CPPUNIT_TEST_SUITE() macro to add test cases.
 * The macro internally just calls the CPPUNIT_TEST macro.
 *
 * @param TESTMETHOD The name of the member funtion test.
 */
#define MITK_TEST(TESTMETHOD) CPPUNIT_TEST(TESTMETHOD)

/**
 * @brief Adds a parameterized test to the current test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * Use this macro after the CPPUNIT_TEST_SUITE() macro to add test cases
 * which need custom parameters.
 *
 * @param TESTMETHOD The name of the member function test.
 * @param ARGS A std::vector<std::string> object containing test parameter.
 *
 * @note Use the macro MITK_PARAMETERIZED_TEST only if you know what
 * you are doing. If you are not sure, use MITK_TEST instead.
 */
#define MITK_PARAMETERIZED_TEST(TESTMETHOD, ARGS)              \
{                                                              \
  std::string testName = #TESTMETHOD;                          \
  for (std::size_t i = 0; i < ARGS.size(); ++i)                \
  {                                                            \
    testName += "_" + ARGS[i];                                 \
  }                                                            \
  CPPUNIT_TEST_SUITE_ADD_TEST(                                 \
      ( new mitk::TestCaller<TestFixtureType>(                 \
                context.getTestNameFor(testName),              \
                &TestFixtureType::TESTMETHOD,                  \
                context.makeFixture(), args ) ) );             \
}

/**
 * @brief Adds a parameterized test to the current test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * Use this macro after the CPPUNIT_TEST_SUITE() macro to add test cases
 * which need parameters from the command line.
 *
 * @warning Use the macro MITK_PARAMETERIZED_CMD_LINE_TEST only
 * if you know what you are doing. If you are not sure, use
 * MITK_TEST instead. MITK_PARAMETERIZED_CMD_LINE_TEST is meant
 * for migrating from ctest to CppUnit. If you implement new
 * tests, the MITK_TEST macro will be sufficient.
 *
 * @param TESTMETHOD The name of the member function test.
 */
#define MITK_PARAMETERIZED_CMD_LINE_TEST(TESTMETHOD)           \
  CPPUNIT_TEST_SUITE_ADD_TEST(                                 \
      ( new mitk::TestCaller<TestFixtureType>(                 \
                context.getTestNameFor( #TESTMETHOD),          \
                &TestFixtureType::TESTMETHOD,                  \
                context.makeFixture() ) ) );

/**
 * @brief Adds a parameterized test to the current test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * Use this macro after the CPPUNIT_TEST_SUITE() macro to add test cases
 * which need one custom parameter.
 *
 * @param TESTMETHOD The name of the member function test.
 * @param arg1 A custom string parameter being passed to the fixture.
 *
 * @note Use the macro MITK_PARAMETERIZED_TEST_1 only if you know what
 * you are doing. If you are not sure, use MITK_TEST instead.
 *
 * @see MITK_PARAMETERIZED_TEST
 */
#define MITK_PARAMETERIZED_TEST_1(TESTMETHOD, arg1)            \
{                                                              \
  std::vector<std::string> args;                               \
  args.push_back(arg1);                                        \
  MITK_PARAMETERIZED_TEST(TESTMETHOD, args)                    \
}

/**
 * @brief Adds a parameterized test to the current test suite.
 *
 * @ingroup MITKTestingAPI
 *
 * Use this macro after the CPPUNIT_TEST_SUITE() macro to add test cases
 * which need two custom parameter.
 *
 * @param TESTMETHOD The name of the member function test.
 * @param arg1 A custom string parameter being passed to the fixture.
 *
 * @note Use the macro MITK_PARAMETERIZED_TEST_2 only if you know what
 * you are doing. If you are not sure, use MITK_TEST instead.
 *
 * @see MITK_PARAMETERIZED_TEST
 */
#define MITK_PARAMETERIZED_TEST_2(TESTMETHOD, arg1, arg2)      \
{                                                              \
  std::vector<std::string> args;                               \
  args.push_back(arg1);                                        \
  args.push_back(arg2);                                        \
  MITK_PARAMETERIZED_TEST(TESTMETHOD, args)                    \
}
#endif
