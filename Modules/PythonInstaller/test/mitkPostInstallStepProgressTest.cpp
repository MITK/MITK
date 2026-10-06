/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPostInstallStepProgress.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

class mitkPostInstallStepProgressTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPostInstallStepProgressTestSuite);
  MITK_TEST(TestProgressLine);
  MITK_TEST(TestLineBreaks);
  MITK_TEST(TestOtherLines);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestProgressLine()
  {
    // More than fits into 32 bits, as the bytes of a large download do.
    const auto progress = mitk::ParsePostInstallStepProgress("MITK_PROGRESS 4294967296 8589934592");

    CPPUNIT_ASSERT_MESSAGE("A progress line should be recognized", progress.has_value());
    CPPUNIT_ASSERT_EQUAL(std::uint64_t(4294967296), progress->Done);
    CPPUNIT_ASSERT_EQUAL(std::uint64_t(8589934592), progress->Total);

    const auto unknown = mitk::ParsePostInstallStepProgress("MITK_PROGRESS 0 0");

    CPPUNIT_ASSERT_MESSAGE("A progress line without a total should be recognized", unknown.has_value());
    CPPUNIT_ASSERT_EQUAL(std::uint64_t(0), unknown->Total);
  }

  void TestLineBreaks()
  {
    CPPUNIT_ASSERT_MESSAGE("A line break should be accepted", mitk::ParsePostInstallStepProgress("MITK_PROGRESS 1 2\n").has_value());
    CPPUNIT_ASSERT_MESSAGE("A Windows line break should be accepted", mitk::ParsePostInstallStepProgress("MITK_PROGRESS 1 2\r\n").has_value());
  }

  void TestOtherLines()
  {
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress("").has_value());
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress("Downloading model").has_value());
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress("MITK_PROGRESS 1").has_value());
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress("MITK_PROGRESS -1 2").has_value());
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress("MITK_PROGRESS 1.5 2").has_value());
    CPPUNIT_ASSERT(!mitk::ParsePostInstallStepProgress(" MITK_PROGRESS 1 2").has_value());
    CPPUNIT_ASSERT_MESSAGE("A number beyond 64 bits should be rejected",
                           !mitk::ParsePostInstallStepProgress("MITK_PROGRESS 1 99999999999999999999999").has_value());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPostInstallStepProgress)
