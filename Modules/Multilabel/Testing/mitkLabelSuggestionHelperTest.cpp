/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c)
German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkLabelSuggestionHelper.h>
#include <mitkLabelSetImage.h>

#include <mitkCoreServices.h>
#include <mitkIPreferencesService.h>
#include <mitkIPreferences.h>

#include <filesystem>
#include <algorithm>

class mitkLabelSuggestionHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkLabelSuggestionHelperTestSuite);

  MITK_TEST(TestParseAndGetSuggestions);
  MITK_TEST(TestIsNewInstanceAllowed);
  MITK_TEST(TestFilterAndRenameLogic);
  MITK_TEST(TestAddAndClearSuggestions);
  MITK_TEST(TestSuggestionOncePreferenceChangesBehavior);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::LabelSuggestionHelper::Pointer m_Helper;
  std::string m_TestFile;
  mitk::MultiLabelSegmentation::Pointer m_Segmentation;

  bool HasSuggestion(const mitk::LabelSuggestionHelper::ConstLabelVectorType& vec, const std::string& name)
  {
    return std::any_of(vec.begin(), vec.end(),
      [&name](const mitk::Label* l) { return l->GetName() == name; });
  }

  mitk::IPreferences* GetSegmentationPreferencesNode()
  {
    auto prefsService = mitk::CoreServices::GetPreferencesService();
    return prefsService->GetSystemPreferences()->Node("/org.mitk.views.segmentation");
  }

public:
  void setUp() override
  {
    m_Helper = mitk::LabelSuggestionHelper::New();
    m_TestFile = GetTestDataFilePath("Multilabel/mitkLabelSuggestions_test.json");

    m_Segmentation = mitk::MultiLabelSegmentation::New();
    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 96, 128, 52 };
    regularImage->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);
    m_Segmentation->Initialize(regularImage);


    auto prefsService = mitk::CoreServices::GetPreferencesService();
    auto prefsFile = GetTestDataFilePath("Multilabel/prefs_segmentation_view.xml");
    prefsService->InitializeStorage(prefsFile);
  }

  void tearDown() override
  {
    m_Helper = nullptr;
    auto prefsService = mitk::CoreServices::GetPreferencesService();
    prefsService->UninitializeStorage(false);
  }

  void TestParseAndGetSuggestions()
  {
    bool ok = m_Helper->ParseSuggestions(m_TestFile, true);
    CPPUNIT_ASSERT_MESSAGE("Parsing suggestions from file should succeed", ok);

    auto suggestions = m_Helper->GetAllSuggestions();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(3), suggestions.size());

    CPPUNIT_ASSERT_MESSAGE("Expected label '5_max' missing", HasSuggestion(suggestions, "5_max"));
    CPPUNIT_ASSERT_MESSAGE("Expected label 'Unique' missing", HasSuggestion(suggestions, "Unique"));
    CPPUNIT_ASSERT_MESSAGE("Expected label 'Unlimited' missing", HasSuggestion(suggestions, "Unlimited"));
  }

  void TestIsNewInstanceAllowed()
  {
    m_Helper->ParseSuggestions(m_TestFile);

    // --- Unique: only 1 instance allowed ---
    auto uniqueLabel = mitk::Label::New();
    uniqueLabel->SetName("Unique");
    m_Segmentation->AddLabel(uniqueLabel, 0);
    CPPUNIT_ASSERT_MESSAGE("Unique should not allow new instances after 1",
      !m_Helper->IsNewInstanceAllowed(m_Segmentation, "Unique"));

    // --- Unlimited: always allowed ---
    CPPUNIT_ASSERT_MESSAGE("Unlimited should always allow new instances",
      m_Helper->IsNewInstanceAllowed(m_Segmentation, "Unlimited"));

    // --- 5_max: allow up to 5 instances ---
    for (int i = 0; i < 4; ++i)
    {
      auto l = mitk::Label::New();
      l->SetName("5_max");
      m_Segmentation->AddLabel(l, 0);
    }

    CPPUNIT_ASSERT_MESSAGE("5_max should allow adding until the 5th instance",
      m_Helper->IsNewInstanceAllowed(m_Segmentation, "5_max"));

    // add 5th
    auto l5 = mitk::Label::New();
    l5->SetName("5_max");
    m_Segmentation->AddLabel(l5, 0);

    CPPUNIT_ASSERT_MESSAGE("5_max should NOT allow a 6th instance",
      !m_Helper->IsNewInstanceAllowed(m_Segmentation, "5_max"));
  }

  void TestFilterAndRenameLogic()
  {
    m_Helper->ParseSuggestions(m_TestFile);

    auto uniqueLabel = mitk::Label::New();
    uniqueLabel->SetName("Unique");
    m_Segmentation->AddLabel(uniqueLabel, 0);

    // Valid suggestions for new labels → "Unique" should be excluded
    auto validNew = m_Helper->GetValidSuggestionsForNewLabels(m_Segmentation);
    CPPUNIT_ASSERT_MESSAGE("Valid new suggestions should NOT include 'Unique'",
      !HasSuggestion(validNew, "Unique"));

    // Valid rename suggestions → "Unique" should be included
    auto validRename = m_Helper->GetValidSuggestionsForRenamingLabels(m_Segmentation, "Unique");
    CPPUNIT_ASSERT_MESSAGE("Valid rename suggestions SHOULD include 'Unique'",
      HasSuggestion(validRename, "Unique"));
  }

  void TestAddAndClearSuggestions()
  {
    m_Helper->ParseSuggestions(m_TestFile);
    auto before = m_Helper->GetAllSuggestions();

    auto newLabel = mitk::Label::New();
    newLabel->SetName("Extra");
    m_Helper->AddSuggestion(newLabel);

    auto afterAdd = m_Helper->GetAllSuggestions();
    CPPUNIT_ASSERT_EQUAL(before.size() + 1, afterAdd.size());
    CPPUNIT_ASSERT_MESSAGE("Newly added label 'Extra' should exist in suggestions",
      HasSuggestion(afterAdd, "Extra"));

    m_Helper->ClearSuggestions();
    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(0), m_Helper->GetAllSuggestions().size());
  }


  void TestSuggestionOncePreferenceChangesBehavior()
  {
    // Ensure suggestions parsed
    m_Helper->ParseSuggestions(m_TestFile);

    for (int i = 0; i < 2; ++i)
    {
      auto l = mitk::Label::New();
      l->SetName("Unlimited");
      m_Segmentation->AddLabel(l, 0);
    }

    // First, ensure current pref is true (set in setUp)
    auto* node = GetSegmentationPreferencesNode();
    node->PutBool("suggest once", true);
    CPPUNIT_ASSERT_MESSAGE("'suggest once' should be true for this part of the test",
      node->GetBool("suggest once", false) == true);

    bool allowedWhenOnce = m_Helper->IsNewInstanceAllowed(m_Segmentation, "Unlimited");
    CPPUNIT_ASSERT_MESSAGE("When suggestionOnce == true, 'Unlimited' should NOT allow another instance", !allowedWhenOnce);

    // Now set suggest once = false and check behavior changes (Unlimited -> unlimited)
    node->PutBool("suggest once", false);
    CPPUNIT_ASSERT_MESSAGE("'suggest once' should now be false", node->GetBool("suggest once", true) == false);

    bool allowedWhenNotOnce = m_Helper->IsNewInstanceAllowed(m_Segmentation, "Unlimited");
    CPPUNIT_ASSERT_MESSAGE("When suggestionOnce == false, 'Unlimited' should allow another instance", allowedWhenNotOnce);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkLabelSuggestionHelper)
