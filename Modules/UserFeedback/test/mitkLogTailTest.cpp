/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkLogTail.h>

#include <mitkException.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <filesystem>
#include <fstream>

class mitkLogTailTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLogTailTestSuite);
  MITK_TEST(ShortFileIsReturnedWhole);
  MITK_TEST(LongFileIsCutToTheLastLines);
  MITK_TEST(MissingTrailingNewlineIsAdded);
  MITK_TEST(CrLfIsNormalized);
  MITK_TEST(EmptyFileGivesNoLines);
  MITK_TEST(MissingFileThrows);
  MITK_TEST(FileLargerThanOneReadBlock);
  CPPUNIT_TEST_SUITE_END();

  std::filesystem::path m_Directory;

  std::filesystem::path Write(const std::string& content)
  {
    const auto path = m_Directory / "test.log";
    std::ofstream(path, std::ios::binary) << content;
    return path;
  }

  static std::string Lines(int first, int last)
  {
    std::string lines;

    for (int i = first; i <= last; ++i)
      lines += "line " + std::to_string(i) + '\n';

    return lines;
  }

public:
  void setUp() override
  {
    m_Directory = mitk::IOUtil::CreateTemporaryDirectory("mitkLogTailTest_XXXXXX");
  }

  void tearDown() override
  {
    std::error_code error;
    std::filesystem::remove_all(m_Directory, error);
  }

  void ShortFileIsReturnedWhole()
  {
    const auto tail = mitk::ReadLogTail(this->Write(Lines(1, 3)), 5);

    CPPUNIT_ASSERT_EQUAL(Lines(1, 3), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), tail.linesTaken);
    CPPUNIT_ASSERT(!tail.truncated);
  }

  void LongFileIsCutToTheLastLines()
  {
    const auto tail = mitk::ReadLogTail(this->Write(Lines(1, 10)), 4);

    CPPUNIT_ASSERT_EQUAL(Lines(7, 10), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(4), tail.linesTaken);
    CPPUNIT_ASSERT(tail.truncated);

    const auto exact = mitk::ReadLogTail(this->Write(Lines(1, 4)), 4);
    CPPUNIT_ASSERT_EQUAL(Lines(1, 4), exact.text);
    CPPUNIT_ASSERT(!exact.truncated);
  }

  void MissingTrailingNewlineIsAdded()
  {
    const auto tail = mitk::ReadLogTail(this->Write("a\nb\nc"), 2);

    CPPUNIT_ASSERT_EQUAL(std::string("b\nc\n"), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), tail.linesTaken);
    CPPUNIT_ASSERT(tail.truncated);
  }

  void CrLfIsNormalized()
  {
    const auto tail = mitk::ReadLogTail(this->Write("a\r\nb\r\nc\r\n"), 2);

    CPPUNIT_ASSERT_EQUAL(std::string("b\nc\n"), tail.text);
    CPPUNIT_ASSERT(tail.truncated);
  }

  void EmptyFileGivesNoLines()
  {
    const auto tail = mitk::ReadLogTail(this->Write(""));

    CPPUNIT_ASSERT(tail.text.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t(0), tail.linesTaken);
    CPPUNIT_ASSERT(!tail.truncated);
  }

  void MissingFileThrows()
  {
    CPPUNIT_ASSERT_THROW(mitk::ReadLogTail(m_Directory / "missing.log"), mitk::Exception);
  }

  void FileLargerThanOneReadBlock()
  {
    // About 1.6 MB, so the tail spans several of the reader's blocks.
    const auto tail = mitk::ReadLogTail(this->Write(Lines(1, 100000)), 30000);

    CPPUNIT_ASSERT_EQUAL(Lines(70001, 100000), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(30000), tail.linesTaken);
    CPPUNIT_ASSERT(tail.truncated);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLogTail)
