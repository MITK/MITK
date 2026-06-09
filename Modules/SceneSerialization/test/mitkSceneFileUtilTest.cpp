/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkSceneFileUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

/**
 * \brief Unit tests for mitk::EndsWithCaseInsensitive.
 *
 * The predicate routes scene files by filename suffix in SceneIO and in the
 * Segmentation Task List scene loader, so its edge cases are pinned here.
 */
class mitkSceneFileUtilTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSceneFileUtilTestSuite);
  MITK_TEST(MatchesExactSuffix);
  MITK_TEST(IgnoresStringCase);
  MITK_TEST(IgnoresSuffixCase);
  MITK_TEST(MatchesJsonIndex);
  MITK_TEST(RejectsSuffixLongerThanTail);
  MITK_TEST(RejectsSuffixLongerThanString);
  MITK_TEST(RejectsDifferentSuffix);
  MITK_TEST(RejectsTailThatOnlyResembles);
  MITK_TEST(EmptySuffixMatchesAny);
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
};

MITK_TEST_SUITE_REGISTRATION(mitkSceneFileUtil)
