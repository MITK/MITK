/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkMultiLabelIOHelper.h>
#include <mitkLabel.h>
#include <mitkImage.h>
#include <mitkNodePredicateGeometry.h>

#include <filesystem>

namespace CppUnit
{
  namespace StringHelper
  {
    template<> inline std::string toString(const mitk::MultiLabelSegmentation::LabelValueVectorType& lvs)
    {
      std::ostringstream stream;
      stream << "[";
      for (mitk::MultiLabelSegmentation::LabelValueVectorType::const_iterator iter = lvs.begin(); iter != lvs.end(); ++iter)
      {
        stream << *iter;
        if (iter + 1 != lvs.end()) stream << ", ";

      }
      stream << "]";
      return stream.str();
    }
  }
}

class mitkMultiLabelIOHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelIOHelperTestSuite);

  MITK_TEST(TestSaveAndLoadJSONPreset);
  MITK_TEST(TestLoadLegacyXMLPreset);
  MITK_TEST(TestLoadJSONPreset);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TestJSONFile;
  std::string m_TestLegacyFile;
  std::string m_TempPresetFile;
  mitk::MultiLabelSegmentation::Pointer m_Segmentation;
  mitk::MultiLabelSegmentation::Pointer m_EmptySegmentation;

public:
  void setUp() override
  {
    m_TestJSONFile = GetTestDataFilePath("Multilabel/Preset.json");
    m_TestLegacyFile = GetTestDataFilePath("Multilabel/Preset_legacy.lpset");

    // Initialize segmentation
    m_EmptySegmentation = mitk::MultiLabelSegmentation::New();
    m_Segmentation = mitk::MultiLabelSegmentation::New();
    mitk::Image::Pointer regularImage = mitk::Image::New();
    unsigned int dimensions[3] = { 32, 32, 16 };
    regularImage->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);
    m_EmptySegmentation->Initialize(regularImage);
    m_Segmentation->Initialize(regularImage);

    // Create one dummy label + group
    m_Segmentation->AddLabel("TestLabel", mitk::MakeColor(1, 0, 0), 0);
    m_Segmentation->AddLabel("TestLabel2", mitk::MakeColor(0, 1, 0), 0);
    m_Segmentation->AddGroup();
    m_Segmentation->SetGroupName(1, "group2");
    m_Segmentation->AddLabel("TestLabel3", mitk::MakeColor(0, 0, 1), 1);
    m_Segmentation->AddLabel(mitk::Label::New(8, "TestLabel8"), 0);

    // Temporary filename for save/load
    m_TempPresetFile = std::filesystem::temp_directory_path().string() + "/MITK_TestPreset.mitklabel.json";
  }

  void tearDown() override
  {
    if (std::filesystem::exists(m_TempPresetFile))
      std::filesystem::remove(m_TempPresetFile);
    m_Segmentation = nullptr;
  }

  void TestSaveAndLoadJSONPreset()
  {

    bool saveOK = mitk::MultiLabelIOHelper::SaveMultiLabelSegmentationPreset(m_TempPresetFile, m_Segmentation);
    CPPUNIT_ASSERT_MESSAGE("Saving JSON preset should succeed", saveOK);

    CPPUNIT_ASSERT(std::filesystem::exists(m_TempPresetFile));

    // Create a new segmentation to load into
    mitk::MultiLabelSegmentation::Pointer loadedSegmentation = mitk::MultiLabelSegmentation::New();
    loadedSegmentation->Initialize(m_Segmentation->GetTimeGeometry(), true, false);

    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(m_TempPresetFile, loadedSegmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading JSON preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Loaded segmentation should contain at least one label", mitk::Equal(*loadedSegmentation, *m_Segmentation, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,true));
  }

  void TestLoadLegacyXMLPreset()
  {
    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(m_TestLegacyFile, m_EmptySegmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading legacy XML preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 2 groups",
      m_EmptySegmentation->GetNumberOfGroups() == 2);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 3 labels in group 0", m_EmptySegmentation->GetLabelValuesByGroup(0).size() == 3);

    auto labels = m_EmptySegmentation->GetLabelValuesByName(0, "Label 1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 1\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);
    labels = m_EmptySegmentation->GetLabelValuesByName(0, "Label 2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 2\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 2 }), labels);
    labels = m_EmptySegmentation->GetLabelValuesByName(0, "Label 3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 3\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 3 }), labels);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 1 labels in group 1", m_EmptySegmentation->GetLabelValuesByGroup(1).size() == 1);

    labels = m_EmptySegmentation->GetLabelValuesByName(1, "Label 4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 4\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 4 }), labels);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // test loading into segmentation with existing labels
    loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(m_TestLegacyFile, m_Segmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading legacy XML preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 2 groups",
      m_Segmentation->GetNumberOfGroups() == 2);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 4 labels in group 0", m_Segmentation->GetLabelValuesByGroup(0).size() == 4);

    labels = m_Segmentation->GetLabelValuesByName(0, "Label 1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 1\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "Label 2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 2\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 2 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "Label 3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 3\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 3 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "TestLabel8");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 8\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 8 }), labels);

    CPPUNIT_ASSERT_MESSAGE("Legacy preset should contain 1 labels in group 1", m_Segmentation->GetLabelValuesByGroup(1).size() == 1);

    labels = m_Segmentation->GetLabelValuesByName(1, "Label 4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 4\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 4 }), labels);
  }

  void TestLoadJSONPreset()
  {
    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(m_TestJSONFile, m_EmptySegmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading JSON preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 2 groups",
      m_EmptySegmentation->GetNumberOfGroups() == 2);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 4 labels in group 0", m_EmptySegmentation->GetLabelValuesByGroup(0).size() == 4);

    auto labels = m_EmptySegmentation->GetLabelValuesByName(0, "Label 1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 1\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);
    labels = m_EmptySegmentation->GetLabelValuesByName(0, "Label 3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 3\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 3 }), labels);
    labels = m_EmptySegmentation->GetLabelValuesByName(0, "Unique");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Unique\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 11 }), labels);
    labels = m_EmptySegmentation->GetLabelValuesByName(0, "Unlimited");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Unlimited\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 12 }), labels);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 1 labels in group 1", m_EmptySegmentation->GetLabelValuesByGroup(1).size() == 1);

    labels = m_EmptySegmentation->GetLabelValuesByName(1, "Label 4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 4\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 13 }), labels);

    /////////////////////////////////////////////////////////////////////////////////////////////////////
    // test loading into segmentation with existing labels
    loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(m_TestJSONFile, m_Segmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading JSON preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 2 groups",
      m_Segmentation->GetNumberOfGroups() == 2);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 6 labels in group 0", m_Segmentation->GetLabelValuesByGroup(0).size() == 6);

    labels = m_Segmentation->GetLabelValuesByName(0, "Label 1");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 1\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 1 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "TestLabel2");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"TestLabel2\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 2 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "Label 3");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 3\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 3 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "Unique");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Unique\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 11 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "Unlimited");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Unlimited\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 12 }), labels);
    labels = m_Segmentation->GetLabelValuesByName(0, "TestLabel8");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 8\" in group 0",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 8 }), labels);

    CPPUNIT_ASSERT_MESSAGE("Preset should contain 1 labels in group 1", m_Segmentation->GetLabelValuesByGroup(1).size() == 1);

    labels = m_Segmentation->GetLabelValuesByName(1, "Label 4");
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong label values retrieved for \"Label 4\" in group 1",
      mitk::MultiLabelSegmentation::LabelValueVectorType({ 13 }), labels);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelIOHelper)
