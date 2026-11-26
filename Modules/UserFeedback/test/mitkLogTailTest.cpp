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
  MITK_TEST(SingleOverlongLineIsCapped);
  MITK_TEST(CarriageReturnProgressIsCapped);
  MITK_TEST(LinesBeyondTheByteCapAreDroppedWhole);
  MITK_TEST(WindowStartingAtALineStartKeepsThatLine);
  MITK_TEST(FileOfExactlyTheCapIsWhole);
  MITK_TEST(Utf8HardCutKeepsSequences);
  MITK_TEST(TrailingBlankLineCountsAsALine);
  CPPUNIT_TEST_SUITE_END();

  static constexpr std::size_t DefaultMaxBytes = 512 * 1024;

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

  /** One line of exactly \p length bytes including its newline, led by a
   *  zero-padded \p number, so that a test can place the byte cap at a known
   *  offset within the lines. */
  static std::string FixedLine(int number, std::size_t length)
  {
    const std::string digits = std::to_string(number);
    std::string line = "line " + std::string(6 - digits.size(), '0') + digits;

    line.resize(length - 1, 'x');
    line += '\n';
    return line;
  }

  static std::string FixedLines(int first, int last, std::size_t length)
  {
    std::string lines;

    for (int i = first; i <= last; ++i)
      lines += FixedLine(i, length);

    return lines;
  }

  /** Exactly DefaultMaxBytes bytes of lines, a multiple of the line length,
   *  so that the last DefaultMaxBytes bytes of a file that ends with them
   *  start at a line start. */
  static std::string CapSizedLines()
  {
    constexpr std::size_t lineLength = 1024;
    static_assert(DefaultMaxBytes % lineLength == 0, "The lines must fill the byte cap exactly.");

    return FixedLines(1, static_cast<int>(DefaultMaxBytes / lineLength), lineLength);
  }

  static bool IsSuffix(const std::string& whole, const std::string& part)
  {
    return part.size() <= whole.size() && whole.compare(whole.size() - part.size(), part.size(), part) == 0;
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
    // About 1.1 MB, more than the byte cap, while the requested lines lie
    // within the last DefaultMaxBytes bytes.
    const auto tail = mitk::ReadLogTail(this->Write(Lines(1, 100000)), 30000);

    CPPUNIT_ASSERT_EQUAL(Lines(70001, 100000), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(30000), tail.linesTaken);
    CPPUNIT_ASSERT(tail.truncated);
  }

  void SingleOverlongLineIsCapped()
  {
    // The whole file is one line, so only a cut inside that line can keep the
    // text within the byte cap.
    const auto tail = mitk::ReadLogTail(this->Write(std::string(10 * 1024 * 1024, 'x')));

    CPPUNIT_ASSERT(tail.truncated);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.text.size()), tail.text.size() <= DefaultMaxBytes + 1);
    CPPUNIT_ASSERT(IsSuffix(tail.text, "x\n"));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), tail.linesTaken);
  }

  void CarriageReturnProgressIsCapped()
  {
    // A progress display redraws its line with carriage returns and never
    // writes a newline, so the file has no line end to cut at.
    std::string progress;

    while (progress.size() < 10 * 1024 * 1024)
      progress += "progress\r";

    const auto tail = mitk::ReadLogTail(this->Write(progress));

    CPPUNIT_ASSERT(tail.truncated);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.text.size()), tail.text.size() <= DefaultMaxBytes + 1);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), tail.linesTaken);
  }

  void LinesBeyondTheByteCapAreDroppedWhole()
  {
    // The window of the last DefaultMaxBytes bytes has to start inside a line
    // for the line it cuts through to be partial.
    constexpr std::size_t lineLength = 1000;
    static_assert(DefaultMaxBytes % lineLength != 0, "The byte cap must fall inside a line.");

    const auto content = FixedLines(1, 1000, lineLength);
    const auto tail = mitk::ReadLogTail(this->Write(content), 1000);

    CPPUNIT_ASSERT(tail.truncated);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.linesTaken), tail.linesTaken < 1000 && tail.linesTaken >= 500);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.text.size()), tail.text.size() <= DefaultMaxBytes + 1);
    CPPUNIT_ASSERT_MESSAGE("only whole lines remain", tail.text.size() == tail.linesTaken * lineLength);
    CPPUNIT_ASSERT_MESSAGE("the first line is whole", tail.text.rfind("line ", 0) == 0);
    CPPUNIT_ASSERT(IsSuffix(content, tail.text));
  }

  void WindowStartingAtALineStartKeepsThatLine()
  {
    // The byte before the window is the newline of the first line, so the line
    // the window starts with is complete.
    const auto lines = CapSizedLines();
    const auto tail = mitk::ReadLogTail(this->Write("z\n" + lines), 1000);

    CPPUNIT_ASSERT(tail.truncated);
    CPPUNIT_ASSERT_EQUAL(std::size_t(512), tail.linesTaken);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.text.size()), tail.text.size() == lines.size());
    CPPUNIT_ASSERT_MESSAGE("the text is exactly the cap-sized lines", tail.text == lines);
  }

  void FileOfExactlyTheCapIsWhole()
  {
    const auto lines = CapSizedLines();
    const auto tail = mitk::ReadLogTail(this->Write(lines), 1000);

    CPPUNIT_ASSERT(!tail.truncated);
    CPPUNIT_ASSERT_EQUAL(std::size_t(512), tail.linesTaken);
    CPPUNIT_ASSERT_MESSAGE("the text is the whole file", tail.text == lines);
  }

  void Utf8HardCutKeepsSequences()
  {
    // Whole sequences end at the end of the file and the length of the byte
    // window is no multiple of their length, so the window starts inside one.
    static_assert(DefaultMaxBytes % 3 != 0, "The byte cap must fall inside a three-byte sequence.");

    std::string euros;

    for (int i = 0; i < 200000; ++i)
      euros += "\xE2\x82\xAC";

    const auto tail = mitk::ReadLogTail(this->Write(euros));

    CPPUNIT_ASSERT(tail.truncated);
    CPPUNIT_ASSERT(!tail.text.empty());
    CPPUNIT_ASSERT_MESSAGE("the text starts with a lead byte",
      (static_cast<unsigned char>(tail.text.front()) & 0xC0) != 0x80);
    CPPUNIT_ASSERT_MESSAGE("only whole sequences remain", (tail.text.size() - 1) % 3 == 0);
    CPPUNIT_ASSERT_MESSAGE(std::to_string(tail.text.size()), tail.text.size() <= DefaultMaxBytes + 1);
  }

  void TrailingBlankLineCountsAsALine()
  {
    // The final empty line is a line of its own, so one line keeps it and
    // drops "a".
    const auto tail = mitk::ReadLogTail(this->Write("a\n\n"), 1);

    CPPUNIT_ASSERT_EQUAL(std::string("\n"), tail.text);
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), tail.linesTaken);
    CPPUNIT_ASSERT(tail.truncated);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLogTail)
