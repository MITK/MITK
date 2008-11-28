/*=========================================================================

Program:   openCherry Platform
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <exception>
#include <string>
#include <iostream>

#include <cherryTestManager.h>

namespace cherry {
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
#define CHERRY_TEST_OUTPUT_NO_ENDL(x) \
  std::cout x ;

/** \brief Output some text. */
#define CHERRY_TEST_OUTPUT(x) \
  CHERRY_TEST_OUTPUT_NO_ENDL(x << "\n")

/** \brief Do some general test preparations. Must be called first in the main test function. */
#define CHERRY_TEST_BEGIN(testName)      \
  std::string cherryTestName(#testName);   \
  cherry::TestManager::GetInstance()->Initialize();  \
  try {

/** \brief Fail and finish test with message MSG */
#define CHERRY_TEST_FAILED_MSG(MSG)  \
  CHERRY_TEST_OUTPUT(MSG) \
  throw cherry::TestFailedException();

/** \brief Must be called last in the main test function. */
#define CHERRY_TEST_END()                                   \
  } catch (cherry::TestFailedException ex) {                      \
    CHERRY_TEST_OUTPUT(<< "Further test execution skipped.") \
    cherry::TestManager::GetInstance()->TestFailed(); \
  }\
  if (cherry::TestManager::GetInstance()->NumberOfFailedTests() > 0) {  \
    CHERRY_TEST_OUTPUT(<< cherryTestName << ": [DONE FAILED] , subtests passed: " << \
    cherry::TestManager::GetInstance()->NumberOfPassedTests() << " failed: " << \
    cherry::TestManager::GetInstance()->NumberOfFailedTests() )    \
    return EXIT_FAILURE;                                  \
  } else {                                                \
    CHERRY_TEST_OUTPUT(<< cherryTestName << ": "<< cherry::TestManager::GetInstance()->NumberOfPassedTests() << " tests [DONE PASSED]")    \
    return EXIT_SUCCESS;                                  \
  }                                                       \

#define CHERRY_TEST_CONDITION(COND,MSG) \
  CHERRY_TEST_OUTPUT_NO_ENDL(<< MSG) \
  if ( ! (COND) ) {        \
    cherry::TestManager::GetInstance()->TestFailed(); \
    CHERRY_TEST_OUTPUT(<< " [FAILED]\n" << "In " << __FILE__        \
                     << ", line " << __LINE__    \
                     << ":  " #COND " : [FAILED]")    \
  } else {                         \
    CHERRY_TEST_OUTPUT(<< " [PASSED]") \
    cherry::TestManager::GetInstance()->TestPassed(); \
 }

#define CHERRY_TEST_CONDITION_REQUIRED(COND,MSG) \
  CHERRY_TEST_OUTPUT_NO_ENDL(<< MSG) \
  if ( ! (COND) ) {        \
    CHERRY_TEST_FAILED_MSG(<< " [FAILED]\n" << "  +--> in " << __FILE__  \
                     << ", line " << __LINE__                      \
                     << ", expression is false: \"" #COND "\"")    \
  } else {                         \
    CHERRY_TEST_OUTPUT(<< " [PASSED]") \
    cherry::TestManager::GetInstance()->TestPassed(); \
 }

/**
 * \brief Begin block which should be checked for exceptions
 *
 * This macro, together with CHERRY_TEST_FOR_EXCEPTION_END, can be used
 * to test whether a code block throws an expected exception. The test FAILS if the exception is NOT thrown. A simple example:
 *

 CHERRY_TEST_FOR_EXCEPTION_BEGIN(itk::ImageFileReaderException)
    typedef itk::ImageFileReader< itk::Image<unsigned char,2> > ReaderType;
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName("/tmp/not-existing");
    reader->Update();
 CHERRY_TEST_FOR_EXCEPTION_END(itk::ImageFileReaderException)

 *
 */
#define CHERRY_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS) \
  try {

#define CHERRY_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS) \
    cherry::TestManager::GetInstance()->TestFailed(); \
    CHERRY_TEST_OUTPUT( << "Expected an '" << #EXCEPTIONCLASS << "' exception. [FAILED]") \
  } \
  catch (EXCEPTIONCLASS) { \
    CHERRY_TEST_OUTPUT( << "Caught an expected '" << #EXCEPTIONCLASS << "' exception. [PASSED]") \
    cherry::TestManager::GetInstance()->TestPassed(); \
  }


/**
 * \brief Simplified version of CHERRY_TEST_FOR_EXCEPTION_BEGIN / END for
 * a single statement
 */
#define CHERRY_TEST_FOR_EXCEPTION(EXCEPTIONCLASS, STATEMENT) \
  CHERRY_TEST_FOR_EXCEPTION_BEGIN(EXCEPTIONCLASS) \
  STATEMENT ; \
  CHERRY_TEST_FOR_EXCEPTION_END(EXCEPTIONCLASS)

