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
#include <mitkIOUtil.h>
#include <mitkSegIOTestFixtures.h>
#include <mitkTemporoSpatialStringProperty.h>

#include <tinyxml2.h>

#include <fstream>
#include <sstream>

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
  MITK_TEST(TestLoadLegacyPresetWithUnreadableLabelReturnsFalse);
  MITK_TEST(TestLoadJSONPreset);
  MITK_TEST(TestLabelJSONTrackingRoundTrip);
  MITK_TEST(TestDeserializeLabelColorAsColorProperty);
  MITK_TEST(TestDeserializeLabelColorAsIntegers);
  MITK_TEST(TestDeserializeLabelDiscardsOutdatedKeys);
  MITK_TEST(TestDeserializeGroupsReportsLabelsWithOutdatedKeys);
  MITK_TEST(TestLoadLabelFromXMLDiscardsOutdatedKeys);
  MITK_TEST(TestLoadLabelFromXMLDiscardsLegacyLayer);
  MITK_TEST(TestSerializeLabelSkipsOutdatedKeys);
  MITK_TEST(TestDeserializeLabelIndexlessCodeNames);

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
    m_TempPresetFile = mitk::IOUtil::GetTempPath() + mitk::IOUtil::GetDirectorySeparator() + "MITK_TestPreset.mitklabel.json";
  }

  void tearDown() override
  {
    if (fs::exists(m_TempPresetFile))
      fs::remove(m_TempPresetFile);
    m_Segmentation = nullptr;
  }

  void TestSaveAndLoadJSONPreset()
  {

    bool saveOK = mitk::MultiLabelIOHelper::SaveMultiLabelSegmentationPreset(m_TempPresetFile, m_Segmentation);
    CPPUNIT_ASSERT_MESSAGE("Saving JSON preset should succeed", saveOK);

    CPPUNIT_ASSERT(fs::exists(m_TempPresetFile));

    // Create a new segmentation to load into
    mitk::MultiLabelSegmentation::Pointer loadedSegmentation = mitk::MultiLabelSegmentation::New();
    loadedSegmentation->Initialize(m_Segmentation->GetTimeGeometry(), true, false);

    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TempPresetFile, loadedSegmentation);
    CPPUNIT_ASSERT_MESSAGE("Loading JSON preset should succeed", loadOK);

    CPPUNIT_ASSERT_MESSAGE("Loaded segmentation should contain at least one label", mitk::Equal(*loadedSegmentation, *m_Segmentation, mitk::NODE_PREDICATE_GEOMETRY_DEFAULT_CHECK_COORDINATE_PRECISION,true));
  }

  void TestLoadLegacyXMLPreset()
  {
    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TestLegacyFile, m_EmptySegmentation);
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
    loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TestLegacyFile, m_Segmentation);
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

  // The value of the last label lacks its number, so the loader fails after it applied the groups and earlier labels.
  void TestLoadLegacyPresetWithUnreadableLabelReturnsFalse()
  {
    std::string content;
    {
      std::ifstream input(m_TestLegacyFile, std::ios::binary);
      content.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    }
    const std::string valueOfLabel4 = R"(<unsigned value="4"/>)";
    const auto position = content.find(valueOfLabel4);
    CPPUNIT_ASSERT_MESSAGE("Fixture lacks the value of label 4", position != std::string::npos);
    content.replace(position, valueOfLabel4.size(), "<unsigned/>");
    {
      std::ofstream output(m_TempPresetFile, std::ios::binary);
      output << content;
    }

    bool loadOK = true;
    CPPUNIT_ASSERT_NO_THROW(loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TempPresetFile, m_EmptySegmentation));
    CPPUNIT_ASSERT_MESSAGE("Loading a legacy preset with an unreadable label should fail", !loadOK);
  }

  void TestLoadJSONPreset()
  {
    bool loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TestJSONFile, m_EmptySegmentation);
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
    loadOK = mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(m_TestJSONFile, m_Segmentation);
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

  // tracking_id / tracking_uid round-trip: serialise omits the keys when
  // the property is absent OR explicitly empty; deserialise sets the
  // property only for non-empty values. Has*() therefore signals "a real
  // tracking value was carried" uniformly across native JSON and DICOM SEG.
  void TestLabelJSONTrackingRoundTrip()
  {
    auto absentLabel = mitk::Label::New();
    absentLabel->SetName("absent");
    auto absentJson = mitk::MultiLabelIOHelper::SerializeLabelToJSON(absentLabel);
    CPPUNIT_ASSERT_MESSAGE("Absent tracking_id must not be emitted",
                           !absentJson.contains("tracking_id"));
    CPPUNIT_ASSERT_MESSAGE("Absent tracking_uid must not be emitted",
                           !absentJson.contains("tracking_uid"));

    auto emptyStampedLabel = mitk::Label::New();
    emptyStampedLabel->SetName("legacy-empty");
    emptyStampedLabel->SetTrackingID("");
    emptyStampedLabel->SetTrackingUID("");
    auto emptyJson = mitk::MultiLabelIOHelper::SerializeLabelToJSON(emptyStampedLabel);
    CPPUNIT_ASSERT_MESSAGE("Empty-string tracking_id must not be emitted",
                           !emptyJson.contains("tracking_id"));
    CPPUNIT_ASSERT_MESSAGE("Empty-string tracking_uid must not be emitted",
                           !emptyJson.contains("tracking_uid"));

    nlohmann::json legacyJson;
    legacyJson["name"] = "from-legacy";
    legacyJson["value"] = 1;
    legacyJson["tracking_id"] = "";
    legacyJson["tracking_uid"] = "";
    auto legacyLoaded = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(legacyJson);
    CPPUNIT_ASSERT_MESSAGE("Empty-string tracking_id in legacy JSON must not set the property",
                           !legacyLoaded->HasTrackingID());
    CPPUNIT_ASSERT_MESSAGE("Empty-string tracking_uid in legacy JSON must not set the property",
                           !legacyLoaded->HasTrackingUID());

    nlohmann::json realJson;
    realJson["name"] = "with-tracking";
    realJson["value"] = 1;
    realJson["tracking_id"] = "real-id";
    realJson["tracking_uid"] = "real-uid";
    auto realLoaded = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(realJson);
    CPPUNIT_ASSERT_MESSAGE("Non-empty tracking_id must round-trip into HasTrackingID()",
                           realLoaded->HasTrackingID());
    CPPUNIT_ASSERT_EQUAL(std::string("real-id"), realLoaded->GetTrackingID());
    CPPUNIT_ASSERT_MESSAGE("Non-empty tracking_uid must round-trip into HasTrackingUID()",
                           realLoaded->HasTrackingUID());
    CPPUNIT_ASSERT_EQUAL(std::string("real-uid"), realLoaded->GetTrackingUID());
  }

  // Version 1 of the native format wrote colours as self-contained ColorProperty JSON.
  void TestDeserializeLabelColorAsColorProperty()
  {
    const auto label = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(nlohmann::json::parse(
      R"({"value": 3, "name": "A", "color": {"type": "ColorProperty", "value": [0.25, 0.5, 0.75]}})"));

    CPPUNIT_ASSERT_EQUAL(0.25f, label->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(0.5f, label->GetColor().GetGreen());
    CPPUNIT_ASSERT_EQUAL(0.75f, label->GetColor().GetBlue());
  }

  // Hand-written label definitions may give colours as integers in 0..255.
  void TestDeserializeLabelColorAsIntegers()
  {
    const auto label = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(nlohmann::json::parse(
      R"({"value": 3, "name": "A", "color": [51, 102, 255]})"));

    CPPUNIT_ASSERT_EQUAL(51 / 255.f, label->GetColor().GetRed());
    CPPUNIT_ASSERT_EQUAL(102 / 255.f, label->GetColor().GetGreen());
    CPPUNIT_ASSERT_EQUAL(1.f, label->GetColor().GetBlue());
  }

  /**
   * One key of each family that MITK versions before 2025.12 stamped on every label: segment
   * number, label and algorithm type, and the region, category, type and type-modifier codes.
   * They are spelled out because the constants that produced them no longer exist.
   */
  static const std::vector<std::string>& OutdatedKeys()
  {
    static const std::vector<std::string> keys = {
      "DICOM.0062.0002.0062.0004",
      "DICOM.0062.0002.0062.0005",
      "DICOM.0062.0002.0062.0008",
      "DICOM.0062.0002.0008.2218.0008.0100",
      "DICOM.0062.0002.0062.0003.0008.0102",
      "DICOM.0062.0002.0062.000F.0008.0104",
      "DICOM.0062.0002.0062.000F.0062.0011.0008.0100"
    };
    return keys;
  }

  /** The value as those versions wrote it. */
  static nlohmann::json OutdatedValue(const std::string& value)
  {
    return nlohmann::json::parse(
      R"({"type": "TemporoSpatialStringProperty", "value": {"values": [{"t": 0, "value": ")" + value + R"(", "z": 0}]}})");
  }

  static void AssertNoOutdatedKeys(const mitk::Label* label)
  {
    for (const auto& key : label->GetPropertyKeys())
      CPPUNIT_ASSERT_MESSAGE("Outdated key survived: " + key, key.rfind("DICOM.0062.0002.", 0) != 0);
  }

  void TestDeserializeLabelDiscardsOutdatedKeys()
  {
    nlohmann::json json = { { "value", 3 }, { "name", "Liver" } };
    for (const auto& key : OutdatedKeys())
      json[key] = OutdatedValue("SEMIAUTOMATIC");

    bool hadOutdatedProperties = false;
    const auto label = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(json, &hadOutdatedProperties);

    AssertNoOutdatedKeys(label);
    CPPUNIT_ASSERT(hadOutdatedProperties);
    CPPUNIT_ASSERT_EQUAL(mitk::Label::PixelType(3), label->GetValue());
    CPPUNIT_ASSERT_EQUAL(std::string("Liver"), label->GetName());
    CPPUNIT_ASSERT(mitk::Label::AlgorithmType::Undefined == label->GetAlgorithmType());

    hadOutdatedProperties = true;
    mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(nlohmann::json{ { "value", 4 }, { "name", "B" } },
                                                       &hadOutdatedProperties);
    CPPUNIT_ASSERT(!hadOutdatedProperties);
  }

  void TestDeserializeGroupsReportsLabelsWithOutdatedKeys()
  {
    auto outdatedLabel = [](int value) {
      nlohmann::json label = { { "value", value }, { "name", "L" + std::to_string(value) } };
      label[OutdatedKeys().front()] = OutdatedValue(std::to_string(value));
      return label;
    };
    nlohmann::json groups = nlohmann::json::array();
    groups.push_back({ { "labels", { { { "value", 1 }, { "name", "L1" } }, outdatedLabel(2) } } });
    groups.push_back({ { "labels", { outdatedLabel(5) } } });

    std::vector<mitk::Label::PixelType> labelsWithOutdatedProperties = { 99 };
    const auto result = mitk::MultiLabelIOHelper::DeserializeMultiLabelGroupsFromJSON(groups,
                                                                                      &labelsWithOutdatedProperties);

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), result.size());
    for (const auto& group : result)
      for (const auto& label : group.labels)
        AssertNoOutdatedKeys(label);
    CPPUNIT_ASSERT(std::vector<mitk::Label::PixelType>({ 2, 5 }) == labelsWithOutdatedProperties);
  }

  void TestLoadLabelFromXMLDiscardsOutdatedKeys()
  {
    tinyxml2::XMLDocument document;
    document.Parse(R"(<Label>)"
                   R"(<property key="value" type="UShortProperty"><unsigned value="3" /></property>)"
                   R"(<property key="DICOM.0062.0002.0062.0008" type="TemporoSpatialStringProperty">)"
                   R"(<temporo_spatial_string>{"values":[{"z":0, "t":0, "value":"SEMIAUTOMATIC"}]}</temporo_spatial_string>)"
                   R"(</property></Label>)");

    bool hadOutdatedProperties = false;
    const auto label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(document.FirstChildElement("Label"),
                                                                          &hadOutdatedProperties);

    AssertNoOutdatedKeys(label);
    CPPUNIT_ASSERT(hadOutdatedProperties);
    CPPUNIT_ASSERT_EQUAL(mitk::Label::PixelType(3), label->GetValue());
  }

  // The layer of the XML format is the group of the label now. Kept as a property, every later
  // save would write it back.
  void TestLoadLabelFromXMLDiscardsLegacyLayer()
  {
    tinyxml2::XMLDocument document;
    document.Parse(R"(<Label>)"
                   R"(<property key="value" type="UShortProperty"><unsigned value="3" /></property>)"
                   R"(<property key="layer" type="UIntProperty"><unsigned value="1" /></property>)"
                   R"(</Label>)");

    bool hadOutdatedProperties = false;
    const auto label = mitk::MultiLabelIOHelper::LoadLabelFromXMLDocument(document.FirstChildElement("Label"),
                                                                          &hadOutdatedProperties);

    CPPUNIT_ASSERT_MESSAGE("The legacy layer index is dropped", label->GetConstProperty("layer").IsNull());
    CPPUNIT_ASSERT_MESSAGE("The layer index is no outdated DICOM property", !hadOutdatedProperties);
    CPPUNIT_ASSERT_EQUAL(mitk::Label::PixelType(3), label->GetValue());
  }

  // Readers ignore label properties under DICOM.0062.0002., so the writer leaves them out, and says
  // so, since such a property was set deliberately.
  void TestSerializeLabelSkipsOutdatedKeys()
  {
    auto label = mitk::Label::New(3, "Liver");
    label->SetProperty("DICOM.0062.0002.0062.0008", mitk::TemporoSpatialStringProperty::New("SEMIAUTOMATIC"));
    label->SetProperty("DICOM.0062.0002.0062.000F.0008.0104", mitk::TemporoSpatialStringProperty::New("Liver"));
    label->SetProperty("my.key", mitk::StringProperty::New("kept"));

    mitk::test::WarningRecorder warnings("DICOM.0062.0002.");
    const auto json = mitk::MultiLabelIOHelper::SerializeLabelToJSON(label);

    const std::string prefix = "DICOM.0062.0002.";
    for (const auto& [key, value] : json.items())
      CPPUNIT_ASSERT_MESSAGE("An outdated key was written: " + key, key.compare(0, prefix.size(), prefix) != 0);
    CPPUNIT_ASSERT_MESSAGE("Other custom properties are written", json.contains("my.key"));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("One warning per property left out", std::size_t(2), warnings.GetWarnings().size());
  }

  // Hand-written label definitions may omit the index when there is only one region or structure.
  void TestDeserializeLabelIndexlessCodeNames()
  {
    const auto stringValue = [](const std::string& value) {
      return nlohmann::json{ { "type", "StringProperty" }, { "value", value } };
    };

    nlohmann::json json = { { "value", 3 }, { "name", "A" } };
    json["anatomic_region.value"] = stringValue("R1");
    json["anatomic_region.scheme"] = stringValue("SCT");
    json["anatomic_region.meaning"] = stringValue("Region");
    json["primary_anatomic_structure.value"] = stringValue("P1");
    json["primary_anatomic_structure.scheme"] = stringValue("SCT");
    json["primary_anatomic_structure.meaning"] = stringValue("Structure");

    const auto label = mitk::MultiLabelIOHelper::DeserializeLabelFromJSON(json);

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), label->GetAnatomicRegionCount());
    CPPUNIT_ASSERT(mitk::DICOMCodeSequenceWithModifiers("R1", "SCT", "Region") == label->GetAnatomicRegion(0));
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), label->GetPrimaryAnatomicStructureCount());
    CPPUNIT_ASSERT(mitk::DICOMCodeSequenceWithModifiers("P1", "SCT", "Structure") == label->GetPrimaryAnatomicStructure(0));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelIOHelper)
