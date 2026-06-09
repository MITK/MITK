/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkSegmentationTaskListWidget.h>

#include <mitkException.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <string>

class QmitkSegmentationTaskListWidgetTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkSegmentationTaskListWidgetTestSuite);
  MITK_TEST(ClassifiesStandaloneJson);
  MITK_TEST(ClassifiesStandaloneJsonUpperCase);
  MITK_TEST(ClassifiesStandaloneJsonMixedCase);
  MITK_TEST(ClassifiesStandaloneJsonAcrossDirsAndDots);
  MITK_TEST(ClassifiesMitkZip);
  MITK_TEST(ClassifiesUnpackedIndex);
  MITK_TEST(RejectsUpperCaseMitk);
  MITK_TEST(RejectsUpperCaseSceneIndex);
  MITK_TEST(RejectsPlainJson);
  MITK_TEST(RejectsUnknownExtension);
  MITK_TEST(RejectsMissingExtension);
  MITK_TEST(ErrorMessageListsAllAcceptedForms);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() override {}
  void tearDown() override {}

  void ClassifiesStandaloneJson()
  {
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("scenes/case.mitkscene.json") ==
                   mitk::SceneFileForm::JsonStandalone);
  }

  void ClassifiesStandaloneJsonUpperCase()
  {
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("CASE.MITKSCENE.JSON") ==
                   mitk::SceneFileForm::JsonStandalone);
  }

  void ClassifiesStandaloneJsonMixedCase()
  {
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("Foo.MitkScene.Json") ==
                   mitk::SceneFileForm::JsonStandalone);
  }

  void ClassifiesStandaloneJsonAcrossDirsAndDots()
  {
    // The full-filename suffix match must be unaffected by directory names or
    // dots earlier in the path.
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("/abs/path/with.dots/case.mitkscene.json") ==
                   mitk::SceneFileForm::JsonStandalone);
  }

  void ClassifiesMitkZip()
  {
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("scenes/case.mitk") == mitk::SceneFileForm::Zip);
  }

  void ClassifiesUnpackedIndex()
  {
    CPPUNIT_ASSERT(mitk::ClassifyTaskListSceneFile("scenes/case.mitksceneindex") ==
                   mitk::SceneFileForm::UnpackedIndex);
  }

  void RejectsUpperCaseMitk()
  {
    // .mitk stays case-sensitive, preserving the legacy widget behavior.
    CPPUNIT_ASSERT_THROW(mitk::ClassifyTaskListSceneFile("scenes/case.MITK"), mitk::Exception);
  }

  void RejectsUpperCaseSceneIndex()
  {
    CPPUNIT_ASSERT_THROW(mitk::ClassifyTaskListSceneFile("scenes/case.MITKSCENEINDEX"), mitk::Exception);
  }

  void RejectsPlainJson()
  {
    // extension() is ".json", but the filename does not end with ".mitkscene.json",
    // so a plain JSON file is not a standalone scene.
    CPPUNIT_ASSERT_THROW(mitk::ClassifyTaskListSceneFile("scenes/case.json"), mitk::Exception);
  }

  void RejectsUnknownExtension()
  {
    CPPUNIT_ASSERT_THROW(mitk::ClassifyTaskListSceneFile("scenes/case.txt"), mitk::Exception);
  }

  void RejectsMissingExtension()
  {
    CPPUNIT_ASSERT_THROW(mitk::ClassifyTaskListSceneFile("scenes/case"), mitk::Exception);
  }

  void ErrorMessageListsAllAcceptedForms()
  {
    try
    {
      mitk::ClassifyTaskListSceneFile("scenes/case.txt");
      CPPUNIT_FAIL("Expected mitk::Exception for an unsupported scene file extension");
    }
    catch (const mitk::Exception& e)
    {
      const std::string message = e.GetDescription();
      CPPUNIT_ASSERT(message.find(".mitk") != std::string::npos);
      CPPUNIT_ASSERT(message.find(".mitksceneindex") != std::string::npos);
      CPPUNIT_ASSERT(message.find(".mitkscene.json") != std::string::npos);
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkSegmentationTaskListWidget)
