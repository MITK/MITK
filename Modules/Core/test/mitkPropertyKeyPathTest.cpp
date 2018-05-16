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

#include "mitkPropertyKeyPath.h"

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <regex>

class mitkPropertyKeyPathTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPropertyKeyPathTestSuite);

  MITK_TEST(AccessFunctions);
  MITK_TEST(PropertyKeyPathToPropertyRegEx);
  MITK_TEST(PropertyKeyPathToPersistenceKeyRegEx);
  MITK_TEST(PropertyKeyPathToPersistenceKeyTemplate);
  MITK_TEST(PropertyKeyPathToPersistenceNameTemplate);
  MITK_TEST(PropertyNameToPropertyKeyPath);
  MITK_TEST(PropertyKeyPathToPropertyName);
  MITK_TEST(ExecutePropertyRegEx);
  MITK_TEST(Comparison);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::PropertyKeyPath simplePath;
  mitk::PropertyKeyPath simplePath2;
  mitk::PropertyKeyPath deepPath;
  mitk::PropertyKeyPath deepPath_withAnyElement;
  mitk::PropertyKeyPath deepPath_withAnySelection;
  mitk::PropertyKeyPath deepPath_withSelection;
  mitk::PropertyKeyPath verydeepPath;

  mitk::PropertyKeyPath emptyPath;

public:
  void setUp() override
  {
    simplePath.AddElement("simple");

    simplePath2.AddElement("AA-11");

    deepPath.AddElement("a").AddElement("b2").AddElement("c3");

    deepPath_withAnyElement.AddElement("a");
    deepPath_withAnyElement.AddAnyElement();
    deepPath_withAnyElement.AddElement("c3");

    deepPath_withAnySelection.AddElement("a");
    deepPath_withAnySelection.AddAnySelection("b");
    deepPath_withAnySelection.AddElement("c");

    deepPath_withSelection.AddElement("a");
    deepPath_withSelection.AddSelection("b", 6);
    deepPath_withSelection.AddElement("c");

    verydeepPath.AddAnySelection("a");
    verydeepPath.AddAnyElement();
    verydeepPath.AddElement("c");
    verydeepPath.AddSelection("d", 4);
    verydeepPath.AddElement("e");
  }

  void tearDown() override {}

  void AccessFunctions()
  {
    const auto constEmptyPath = emptyPath;
    const auto constVerydeepPath = verydeepPath;

    CPPUNIT_ASSERT_THROW(emptyPath.GetFirstNode(), mitk::InvalidPathNodeException);
    CPPUNIT_ASSERT_THROW(emptyPath.GetLastNode(), mitk::InvalidPathNodeException);
    CPPUNIT_ASSERT_THROW(emptyPath.GetNode(0), mitk::InvalidPathNodeException);
    CPPUNIT_ASSERT_THROW(constEmptyPath.GetFirstNode(), mitk::InvalidPathNodeException);
    CPPUNIT_ASSERT_THROW(constEmptyPath.GetLastNode(), mitk::InvalidPathNodeException);
    CPPUNIT_ASSERT_THROW(constEmptyPath.GetNode(0), mitk::InvalidPathNodeException);

    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing GetFirstNode with 'a.[*].*.c.d.[4].e'", std::string("a"), verydeepPath.GetFirstNode().name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing const GetFirstNode with 'a.[*].*.c.d.[4].e'", std::string("a"), constVerydeepPath.GetFirstNode().name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing GetLastNode with 'a.[*].*.c.d.[4].e'", std::string("e"), verydeepPath.GetLastNode().name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing GetLastNode with 'a.[*].*.c.d.[4].e'", std::string("e"), constVerydeepPath.GetLastNode().name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing GetNode(3) with 'a.[*].*.c.d.[4].e'", std::string("d"), verydeepPath.GetNode(3).name);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing GetNode(3) with 'a.[*].*.c.d.[4].e'", std::string("d"), constVerydeepPath.GetNode(3).name);

    CPPUNIT_ASSERT(5 == constVerydeepPath.GetSize());
    CPPUNIT_ASSERT(0 == emptyPath.GetSize());
  }

  void PropertyKeyPathToPropertyRegEx()
  {
    std::string result = mitk::PropertyKeyPathToPropertyRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyRegEx() with 'simple'", std::string("simple"), result);
    result = mitk::PropertyKeyPathToPropertyRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyRegEx() with 'a.b2.c3'", std::string("a\\.b2\\.c3"), result);
    result = mitk::PropertyKeyPathToPropertyRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyRegEx() with 'a.*.c3'", std::string("a\\.([a-zA-Z0-9- ]+)\\.c3"), result);
    result = mitk::PropertyKeyPathToPropertyRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyRegEx() with 'a.b.[*].c'", std::string("a\\.b\\.\\[(\\d*)\\]\\.c"), result);
    result = mitk::PropertyKeyPathToPropertyRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyRegEx() with 'a.b.[6].c'", std::string("a\\.b\\.\\[6\\]\\.c"), result);
    result = mitk::PropertyKeyPathToPropertyRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyKeyPathToPropertyRegEx() with 'a.[*].*.c.d.[4].e'",
                                 std::string("a\\.\\[(\\d*)\\]\\.([a-zA-Z0-9- ]+)\\.c\\.d\\.\\[4\\]\\.e"),
                                 result);
  }

  void PropertyKeyPathToPersistenceKeyRegEx()
  {
    std::string result = mitk::PropertyKeyPathToPersistenceKeyRegEx(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyRegEx() with 'simple'", std::string("simple"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyRegEx(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyRegEx() with 'a.b2.c3'", std::string("a_b2_c3"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyRegEx(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyRegEx() with 'a.*.c3'", std::string("a_([a-zA-Z0-9- ]+)_c3"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyRegEx(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyRegEx() with 'a.b.[*].c'", std::string("a_b_\\[(\\d*)\\]_c"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyRegEx(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyRegEx() with 'a.b.[6].c'", std::string("a_b_\\[6\\]_c"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyRegEx(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyKeyPathToPersistenceKeyRegEx() with 'a.[*].*.c.d.[4].e'",
                                 std::string("a_\\[(\\d*)\\]_([a-zA-Z0-9- ]+)_c_d_\\[4\\]_e"),
                                 result);
  }

  void PropertyKeyPathToPersistenceKeyTemplate()
  {
    std::string result = mitk::PropertyKeyPathToPersistenceKeyTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyTemplate() with 'simple'", std::string("simple"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyTemplate() with 'a.b2.c3'", std::string("a_b2_c3"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyTemplate() with 'a.*.c3'", std::string("a_$1_c3"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyTemplate() with 'a.b.[*].c'", std::string("a_b_[$1]_c"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceKeyTemplate() with 'a.b.[6].c'", std::string("a_b_[6]_c"), result);
    result = mitk::PropertyKeyPathToPersistenceKeyTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyKeyPathToPersistenceKeyTemplate() with 'a.[*].*.c.d.[4].e'",
                                 std::string("a_[$1]_$2_c_d_[4]_e"),
                                 result);
  }

  void PropertyKeyPathToPersistenceNameTemplate()
  {
    std::string result = mitk::PropertyKeyPathToPersistenceNameTemplate(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceNameTemplate() with 'simple'", std::string("simple"), result);
    result = mitk::PropertyKeyPathToPersistenceNameTemplate(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceNameTemplate() with 'a.b2.c3'", std::string("a.b2.c3"), result);
    result = mitk::PropertyKeyPathToPersistenceNameTemplate(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceNameTemplate() with 'a.*.c3'", std::string("a.$1.c3"), result);
    result = mitk::PropertyKeyPathToPersistenceNameTemplate(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceNameTemplate() with 'a.b.[*].c'", std::string("a.b.[$1].c"), result);
    result = mitk::PropertyKeyPathToPersistenceNameTemplate(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPersistenceNameTemplate() with 'a.b.[6].c'", std::string("a.b.[6].c"), result);
    result = mitk::PropertyKeyPathToPersistenceNameTemplate(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyKeyPathToPersistenceNameTemplate() with 'a.[*].*.c.d.[4].e'",
                                 std::string("a.[$1].$2.c.d.[4].e"),
                                 result);
  }

  void PropertyNameToPropertyKeyPath()
  {
    mitk::PropertyKeyPath result = mitk::PropertyNameToPropertyKeyPath("simple");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with 'simple'", simplePath, result);
    result = mitk::PropertyNameToPropertyKeyPath("a.b2.c3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with 'a.b2.c3'", deepPath, result);
    result = mitk::PropertyNameToPropertyKeyPath("a.*.c3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyNameToPropertyKeyPath() with 'a.*.c3'", deepPath_withAnyElement, result);
    result = mitk::PropertyNameToPropertyKeyPath("a.b.[*].c");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyNameToPropertyKeyPath() with 'a.b.[*].c'", deepPath_withAnySelection, result);
    result = mitk::PropertyNameToPropertyKeyPath("a.b.[6].c");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyNameToPropertyKeyPath() with 'a.b.[6].c'", deepPath_withSelection, result);
    result = mitk::PropertyNameToPropertyKeyPath("a.[*].*.c.d.[4].e");
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyNameToPropertyKeyPath() with 'a.[*].*.c.d.[4].e'", verydeepPath, result);

    result = mitk::PropertyNameToPropertyKeyPath("AA-11");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with 'AA-11'", simplePath2, result);

    result = mitk::PropertyNameToPropertyKeyPath("$$$IlligalNameChar.sub");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with wrong path", emptyPath, result);
    result = mitk::PropertyNameToPropertyKeyPath("emptyNode..sub");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with wrong path", emptyPath, result);
    result = mitk::PropertyNameToPropertyKeyPath("wrongIndex.[d]");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Testing PropertyNameToPropertyKeyPath() with wrong path", emptyPath, result);
  }

  void PropertyKeyPathToPropertyName()
  {
    std::string result = mitk::PropertyKeyPathToPropertyName(simplePath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'simple'", result, std::string("simple"));
    result = mitk::PropertyKeyPathToPropertyName(deepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'a.b2.c3'", result, std::string("a.b2.c3"));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withAnyElement);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'a.*.c3'", result, std::string("a.*.c3"));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withAnySelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'a.b.[*].c'", result, std::string("a.b.[*].c"));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withSelection);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'a.b.[6].c'", result, std::string("a.b.[6].c"));
    result = mitk::PropertyKeyPathToPropertyName(verydeepPath);
    CPPUNIT_ASSERT_EQUAL_MESSAGE(
      "Testing PropertyKeyPathToPropertyName() with 'a.[*].*.c.d.[4].e'", result, std::string("a.[*].*.c.d.[4].e"));
  }

  void Comparison()
  {
    mitk::PropertyKeyPath deepPath_noSelection =
      mitk::PropertyKeyPath().AddElement("a").AddElement("b").AddElement("c");

    CPPUNIT_ASSERT(deepPath_noSelection < deepPath);
    CPPUNIT_ASSERT(deepPath_noSelection < deepPath_withSelection);
    CPPUNIT_ASSERT(deepPath_withSelection < deepPath);
    CPPUNIT_ASSERT(deepPath_withSelection < deepPath_withAnySelection);
    CPPUNIT_ASSERT(deepPath_withAnyElement < deepPath_noSelection);

    CPPUNIT_ASSERT(!(deepPath_noSelection < deepPath_noSelection));
    CPPUNIT_ASSERT(!(deepPath_noSelection > deepPath_noSelection));
    CPPUNIT_ASSERT(deepPath_noSelection <= deepPath_noSelection);
    CPPUNIT_ASSERT(deepPath_noSelection >= deepPath_noSelection);
  }

  void ExecutePropertyRegEx()
  {
    std::regex regEx(mitk::PropertyKeyPathToPropertyRegEx(simplePath));
    std::string result = mitk::PropertyKeyPathToPropertyName(simplePath);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));
    regEx = std::regex(mitk::PropertyKeyPathToPropertyRegEx(deepPath));
    result = mitk::PropertyKeyPathToPropertyName(deepPath);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));

    regEx = std::regex(mitk::PropertyKeyPathToPropertyRegEx(deepPath_withAnyElement));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withAnyElement);
    auto position = result.find("*");
    if (std::string::npos != position)
    {
      result.replace(position, 1, "ConcreteNode1");
      CPPUNIT_ASSERT(std::regex_match(result, regEx));
    }

    regEx = std::regex(mitk::PropertyKeyPathToPropertyRegEx(deepPath_withAnySelection));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withAnySelection);
    position = result.find("[*]");
    if (std::string::npos != position)
    {
      result.replace(position, 3, "[10]");
      CPPUNIT_ASSERT(std::regex_match(result, regEx));
    }

    regEx = std::regex(mitk::PropertyKeyPathToPropertyRegEx(deepPath_withSelection));
    result = mitk::PropertyKeyPathToPropertyName(deepPath_withSelection);
    CPPUNIT_ASSERT(std::regex_match(result, regEx));

    regEx = std::regex(mitk::PropertyKeyPathToPropertyRegEx(verydeepPath));
    result = mitk::PropertyKeyPathToPropertyName(verydeepPath);
    position = result.find("[*]");
    if (std::string::npos != position)
    {
      result.replace(position, 3, "[1]");
      position = result.find("*");
      if (std::string::npos != position)
      {
        result.replace(position, 1, "ConcreteNode2");
        CPPUNIT_ASSERT(std::regex_match(result, regEx));
      }
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPropertyKeyPath)
