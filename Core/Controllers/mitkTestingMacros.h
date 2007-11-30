#include <exception>
#include <string>
#include <iostream>

#include <mitkTestManager.h>

namespace mitk {
  /** \brief Indicate a failed test. */ 
  class TestFailedException : public std::exception {
    public:
      TestFailedException() {}
  };
}

/** 
 * 
 * \brief Output some text without generating a terminating newline. Include  
 *
 * */
#define MITK_TEST_OUTPUT_NO_ENDL(x) \
  std::cout x ;

/** \brief Output some text. */
#define MITK_TEST_OUTPUT(x) \
  MITK_TEST_OUTPUT_NO_ENDL(x << "\n")

/** \brief Do some general test preparations. Must be called first in the main test function. */
#define MITK_TEST_BEGIN(testName)      \
  std::string mitkTestName(#testName);   \
  mitk::TestManager::GetInstance()->Initialize();  \
  try {

/** \brief Fail and finish test with message MSG */
#define MITK_TEST_FAILED_MSG(MSG)  \
  MITK_TEST_OUTPUT(MSG) \
  throw mitk::TestFailedException();

/** \brief Must be called last in the main test function. */
#define MITK_TEST_END()                                   \
  } catch (mitk::TestFailedException ex) {                      \
    MITK_TEST_OUTPUT(<< "Further test execution skipped.") \
    mitk::TestManager::GetInstance()->TestFailed(); \
  }                                                         \
  if (mitk::TestManager::GetInstance()->NumberOfFailedTests() > 0) {  \
    MITK_TEST_OUTPUT(<< mitkTestName << ": [DONE FAILED] , subtests passed: " << \
    mitk::TestManager::GetInstance()->NumberOfPassedTests() << " failed: " << \
    mitk::TestManager::GetInstance()->NumberOfFailedTests() )    \
    return EXIT_FAILURE;                                  \
  } else {                                                \
    MITK_TEST_OUTPUT(<< mitkTestName << ": "<< mitk::TestManager::GetInstance()->NumberOfPassedTests() << " tests [DONE PASSED]")    \
    return EXIT_SUCCESS;                                  \
  }                                                       \

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



