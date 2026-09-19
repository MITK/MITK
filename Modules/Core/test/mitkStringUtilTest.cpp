/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkStringUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <cstddef>

/**
 * \brief Unit tests for the mitk::StringUtil helpers.
 *
 * EndsWithCaseInsensitive routes scene files by filename suffix in SceneIO and
 * in the Segmentation Task List scene loader. The Split / Trim / ReplaceAll /
 * EqualsCaseInsensitive helpers replaced Boost string algorithms across several
 * modules. Their edge cases (empty-field preservation, in-place trimming,
 * non-overlapping replacement, case-insensitive comparison) are pinned here.
 */
class mitkStringUtilTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkStringUtilTestSuite);
  MITK_TEST(MatchesExactSuffix);
  MITK_TEST(IgnoresStringCase);
  MITK_TEST(IgnoresSuffixCase);
  MITK_TEST(MatchesJsonIndex);
  MITK_TEST(RejectsSuffixLongerThanTail);
  MITK_TEST(RejectsSuffixLongerThanString);
  MITK_TEST(RejectsDifferentSuffix);
  MITK_TEST(RejectsTailThatOnlyResembles);
  MITK_TEST(EmptySuffixMatchesAny);
  MITK_TEST(SplitReturnsWholeStringWhenDelimiterAbsent);
  MITK_TEST(SplitKeepsEmptyFields);
  MITK_TEST(SplitPreservesLeadingAndTrailingEmptyFields);
  MITK_TEST(SplitEmptyStringYieldsSingleEmptyField);
  MITK_TEST(TrimRemovesLeadingAndTrailingWhitespace);
  MITK_TEST(TrimPreservesInteriorWhitespace);
  MITK_TEST(TrimAllWhitespaceYieldsEmpty);
  MITK_TEST(TrimEmptyStaysEmpty);
  MITK_TEST(ReplaceAllReplacesEveryOccurrence);
  MITK_TEST(ReplaceAllHandlesMultiCharacterPattern);
  MITK_TEST(ReplaceAllEmptyPatternIsNoOp);
  MITK_TEST(ReplaceAllTerminatesWhenReplacementContainsPattern);
  MITK_TEST(EqualsIgnoresCase);
  MITK_TEST(EqualsRejectsDifferentLength);
  MITK_TEST(EqualsRejectsDifferentContent);
  MITK_TEST(EqualsEmptyStringsMatch);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override {}
  void tearDown() override {}

  void MatchesExactSuffix()
  {
    CPPUNIT_ASSERT(mitk::EndsWithCaseInsensitive("a.mitkscene.json", ".mitkscene.json"));
  }

  void IgnoresStringCase()
  {
    CPPUNIT_ASSERT(mitk::EndsWithCaseInsensitive("A.MITKSCENE.JSON", ".mitkscene.json"));
  }

  void IgnoresSuffixCase()
  {
    CPPUNIT_ASSERT(mitk::EndsWithCaseInsensitive("a.mitkscene.json", ".MITKSCENE.JSON"));
  }

  void MatchesJsonIndex()
  {
    CPPUNIT_ASSERT(mitk::EndsWithCaseInsensitive("index.json", ".json"));
  }

  void RejectsSuffixLongerThanTail()
  {
    CPPUNIT_ASSERT(!mitk::EndsWithCaseInsensitive("a.json", ".mitkscene.json"));
  }

  void RejectsSuffixLongerThanString()
  {
    CPPUNIT_ASSERT(!mitk::EndsWithCaseInsensitive(".json", ".mitkscene.json"));
  }

  void RejectsDifferentSuffix()
  {
    CPPUNIT_ASSERT(!mitk::EndsWithCaseInsensitive("scene.mitk", ".json"));
  }

  void RejectsTailThatOnlyResembles()
  {
    // Positional tail comparison, not "contains": "jsonx" does not end with ".json".
    CPPUNIT_ASSERT(!mitk::EndsWithCaseInsensitive("jsonx", ".json"));
  }

  void EmptySuffixMatchesAny()
  {
    // Convention: the empty string is a suffix of every string.
    CPPUNIT_ASSERT(mitk::EndsWithCaseInsensitive("anything", ""));
  }

  void SplitReturnsWholeStringWhenDelimiterAbsent()
  {
    const auto parts = mitk::Split("abc", ',');
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), parts.size());
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), parts[0]);
  }

  void SplitKeepsEmptyFields()
  {
    // The empty field between the two delimiters must be preserved.
    const auto parts = mitk::Split("a,,b", ',');
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), parts.size());
    CPPUNIT_ASSERT_EQUAL(std::string("a"), parts[0]);
    CPPUNIT_ASSERT_EQUAL(std::string(""), parts[1]);
    CPPUNIT_ASSERT_EQUAL(std::string("b"), parts[2]);
  }

  void SplitPreservesLeadingAndTrailingEmptyFields()
  {
    const auto parts = mitk::Split(",a,", ',');
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(3), parts.size());
    CPPUNIT_ASSERT_EQUAL(std::string(""), parts[0]);
    CPPUNIT_ASSERT_EQUAL(std::string("a"), parts[1]);
    CPPUNIT_ASSERT_EQUAL(std::string(""), parts[2]);
  }

  void SplitEmptyStringYieldsSingleEmptyField()
  {
    const auto parts = mitk::Split("", ',');
    CPPUNIT_ASSERT_EQUAL(static_cast<std::size_t>(1), parts.size());
    CPPUNIT_ASSERT_EQUAL(std::string(""), parts[0]);
  }

  void TrimRemovesLeadingAndTrailingWhitespace()
  {
    std::string str = " \t hi \n ";
    mitk::Trim(str);
    CPPUNIT_ASSERT_EQUAL(std::string("hi"), str);
  }

  void TrimPreservesInteriorWhitespace()
  {
    std::string str = "  a b  ";
    mitk::Trim(str);
    CPPUNIT_ASSERT_EQUAL(std::string("a b"), str);
  }

  void TrimAllWhitespaceYieldsEmpty()
  {
    std::string str = " \t\n ";
    mitk::Trim(str);
    CPPUNIT_ASSERT_EQUAL(std::string(""), str);
  }

  void TrimEmptyStaysEmpty()
  {
    std::string str;
    mitk::Trim(str);
    CPPUNIT_ASSERT_EQUAL(std::string(""), str);
  }

  void ReplaceAllReplacesEveryOccurrence()
  {
    std::string str = "a/b/c";
    mitk::ReplaceAll(str, "/", "_");
    CPPUNIT_ASSERT_EQUAL(std::string("a_b_c"), str);
  }

  void ReplaceAllHandlesMultiCharacterPattern()
  {
    std::string str = "a\r\nb\r\nc";
    mitk::ReplaceAll(str, "\r\n", "\n");
    CPPUNIT_ASSERT_EQUAL(std::string("a\nb\nc"), str);
  }

  void ReplaceAllEmptyPatternIsNoOp()
  {
    std::string str = "abc";
    mitk::ReplaceAll(str, "", "x");
    CPPUNIT_ASSERT_EQUAL(std::string("abc"), str);
  }

  void ReplaceAllTerminatesWhenReplacementContainsPattern()
  {
    // The search resumes past the inserted text, not just past the match, so a
    // replacement that contains the pattern must terminate, not loop forever.
    std::string str = "a";
    mitk::ReplaceAll(str, "a", "aa");
    CPPUNIT_ASSERT_EQUAL(std::string("aa"), str);
  }

  void EqualsIgnoresCase()
  {
    CPPUNIT_ASSERT(mitk::EqualsCaseInsensitive("AbC", "aBc"));
  }

  void EqualsRejectsDifferentLength()
  {
    CPPUNIT_ASSERT(!mitk::EqualsCaseInsensitive("abc", "ab"));
  }

  void EqualsRejectsDifferentContent()
  {
    CPPUNIT_ASSERT(!mitk::EqualsCaseInsensitive("abc", "abd"));
  }

  void EqualsEmptyStringsMatch()
  {
    CPPUNIT_ASSERT(mitk::EqualsCaseInsensitive("", ""));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkStringUtil)
