/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkMemoryUtilities.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <locale>

class mitkMemoryUtilitiesTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMemoryUtilitiesTestSuite);
  MITK_TEST(FormatSize_Bytes);
  MITK_TEST(FormatSize_Units);
  MITK_TEST(FormatSize_RoundsIntoNextUnit);
  MITK_TEST(FormatSize_IgnoresGlobalLocale);
  CPPUNIT_TEST_SUITE_END();

public:
  void FormatSize_Bytes()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("0 B"), mitk::MemoryUtilities::FormatSize(0));
    CPPUNIT_ASSERT_EQUAL(std::string("1023 B"), mitk::MemoryUtilities::FormatSize(1023));
  }

  void FormatSize_Units()
  {
    constexpr std::uint64_t KB = 1024;

    CPPUNIT_ASSERT_EQUAL(std::string("1.0 KB"), mitk::MemoryUtilities::FormatSize(KB));
    CPPUNIT_ASSERT_EQUAL(std::string("1.5 KB"), mitk::MemoryUtilities::FormatSize(KB + KB / 2));
    CPPUNIT_ASSERT_EQUAL(std::string("401.0 MB"), mitk::MemoryUtilities::FormatSize(401 * KB * KB));
    CPPUNIT_ASSERT_EQUAL(std::string("12.5 GB"), mitk::MemoryUtilities::FormatSize(12 * KB * KB * KB + KB * KB * KB / 2));
    CPPUNIT_ASSERT_EQUAL(std::string("5.0 TB"), mitk::MemoryUtilities::FormatSize(5 * KB * KB * KB * KB));
    CPPUNIT_ASSERT_EQUAL(std::string("2048.0 TB"), mitk::MemoryUtilities::FormatSize(2048 * KB * KB * KB * KB));
  }

  void FormatSize_RoundsIntoNextUnit()
  {
    // Just below 1 MB would round to "1024.0 KB".
    CPPUNIT_ASSERT_EQUAL(std::string("1.0 MB"), mitk::MemoryUtilities::FormatSize(1024 * 1024 - 1));
  }

  void FormatSize_IgnoresGlobalLocale()
  {
    // A locale with a decimal comma would turn "1.5 KB" into "1,5 KB".
    struct CommaNumpunct : std::numpunct<char>
    {
      char do_decimal_point() const override { return ','; }
    };

    const auto previous = std::locale::global(std::locale(std::locale::classic(), new CommaNumpunct));
    const auto formatted = mitk::MemoryUtilities::FormatSize(1536);
    std::locale::global(previous);

    CPPUNIT_ASSERT_EQUAL(std::string("1.5 KB"), formatted);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMemoryUtilities)
