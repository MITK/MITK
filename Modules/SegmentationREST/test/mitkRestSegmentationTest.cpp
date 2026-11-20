/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkIPreferences.h>
#include <mitkIPreferencesService.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkRestRoute.h>
#include <mitkRestSegmentation.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <filesystem>
#include <functional>
#include <string>
#include <utility>
#include <vector>

namespace
{
  struct Failure
  {
    int Status = 0;
    std::string Message;
  };

  Failure GetFailureOf(const std::function<void()>& call)
  {
    try
    {
      call();
    }
    catch (const mitk::RestException& e)
    {
      const auto& error = e.GetError()["error"];
      return { error["status"].get<int>(), error["message"].get<std::string>() };
    }

    return {};
  }

  mitk::RestRequest CreateRequest(const std::string& param, const std::string& value)
  {
    mitk::RestRequest request;
    request.Path = "/api/v1/segmentation/test";
    request.PathParams[param] = value;
    return request;
  }

  mitk::Color CreateColor(float r, float g, float b)
  {
    mitk::Color color;
    color.Set(r, g, b);
    return color;
  }

  /** The labels are in the order of their groups and values: 1, 2, 3 here. */
  const nlohmann::json& FindLabel(const nlohmann::json& labels, int value)
  {
    return labels.at(value - 1);
  }
}

class mitkRestSegmentationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestSegmentationTestSuite);
  MITK_TEST(TestColorsMustBeHexStrings);
  MITK_TEST(TestMeasureFlag);
  MITK_TEST(TestUnknownFieldsAreRefused);
  MITK_TEST(TestDescribeLabelsAndGroups);
  MITK_TEST(TestDescribeMeasuredLabels);
  MITK_TEST(TestLabelValueInPath);
  MITK_TEST(TestGroupIndexInPath);
  MITK_TEST(TestRestrictionsAndSuggestions);
  CPPUNIT_TEST_SUITE_END();

  mitk::MultiLabelSegmentation::Pointer m_Segmentation;

public:
  /**
   * Group 0 (unnamed) holds Liver (1, three voxels, hidden, locked) and Spleen (2, one voxel),
   * group 1 "Right" holds Kidney (3, no voxel), on 4 x 2 x 2 voxels of 1 x 1 x 2 mm.
   */
  void setUp() override
  {
    unsigned int dimensions[] = { 4, 2, 2 };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);
    mitk::Vector3D spacing;
    spacing[0] = 1.0;
    spacing[1] = 1.0;
    spacing[2] = 2.0;
    image->GetGeometry()->SetSpacing(spacing);

    m_Segmentation = mitk::MultiLabelSegmentation::New();
    m_Segmentation->Initialize(image);

    auto* liver = m_Segmentation->AddLabel("Liver", CreateColor(1, 0.5f, 0), 0);
    liver->SetVisible(false);
    liver->SetLocked(true);
    m_Segmentation->AddLabel("Spleen", CreateColor(0, 1, 0), 0);
    const auto group = m_Segmentation->AddGroup();
    m_Segmentation->SetGroupName(group, "Right");
    m_Segmentation->AddLabel("Kidney", CreateColor(0, 0, 1), group);

    mitk::ImageWriteAccessor accessor(m_Segmentation->GetGroupImage(0));
    auto* pixels = static_cast<mitk::MultiLabelSegmentation::LabelValueType*>(accessor.GetData());
    pixels[0] = pixels[1] = pixels[2] = 1;
    pixels[3] = 2;
  }

  void tearDown() override { m_Segmentation = nullptr; }

  void TestColorsMustBeHexStrings()
  {
    const auto color = mitk::ParseRestColorOrThrow("#ff8800", "/p");
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, color[0], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.533, color[1], 1e-3);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, color[2], 1e-6);

    for (const auto& junk : { nlohmann::json("red"), nlohmann::json("#ff88"), nlohmann::json(255), nlohmann::json(), nlohmann::json::array() })
    {
      const auto failure = GetFailureOf([&]() { mitk::ParseRestColorOrThrow(junk, "/p"); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE(junk.dump(), 400, failure.Status);
      CPPUNIT_ASSERT_EQUAL(std::string("A color must be written like \"#ff8800\"."), failure.Message);
    }
  }

  void TestMeasureFlag()
  {
    mitk::RestRequest request;
    request.Path = "/p";
    CPPUNIT_ASSERT(!mitk::IsRestMeasureRequested(request));

    request.QueryParams.emplace("measure", "true");
    CPPUNIT_ASSERT(mitk::IsRestMeasureRequested(request));

    request.QueryParams.clear();
    request.QueryParams.emplace("measure", "1");
    CPPUNIT_ASSERT_EQUAL(400, GetFailureOf([&]() { mitk::IsRestMeasureRequested(request); }).Status);
  }

  void TestUnknownFieldsAreRefused()
  {
    const std::vector<std::string> fields = { "name", "color", "locked" };
    CPPUNIT_ASSERT_NO_THROW(mitk::ThrowIfUnknownRestField({ { "color", "#fff" } }, fields, "a label", "/p"));

    auto failure = GetFailureOf([&]() { mitk::ThrowIfUnknownRestField({ { "colour", "#fff" } }, fields, "a label", "/p"); });
    CPPUNIT_ASSERT_EQUAL(400, failure.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("\"colour\" is no field of a label. The fields are name, color and locked."), failure.Message);

    failure = GetFailureOf([&]() { mitk::ThrowIfUnknownRestField({ { "x", 1 } }, { "name" }, "a group", "/p"); });
    CPPUNIT_ASSERT_EQUAL(std::string("\"x\" is no field of a group. The field is name."), failure.Message);

    failure = GetFailureOf([&]() { mitk::ThrowIfUnknownRestField(nlohmann::json::array(), fields, "a label", "/p"); });
    CPPUNIT_ASSERT_EQUAL(400, failure.Status);
    CPPUNIT_ASSERT_EQUAL(std::string("The body must be an object with the fields to change."), failure.Message);
  }

  void TestDescribeLabelsAndGroups()
  {
    const auto labels = mitk::DescribeRestLabels(m_Segmentation);
    CPPUNIT_ASSERT_EQUAL(std::size_t(3), labels.size());

    const auto& liver = FindLabel(labels, 1);
    CPPUNIT_ASSERT_EQUAL(std::string("Liver"), liver["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("#ff8000"), liver["color"].get<std::string>());
    CPPUNIT_ASSERT(!liver["visible"].get<bool>());
    CPPUNIT_ASSERT(liver["locked"].get<bool>());
    CPPUNIT_ASSERT(!liver.contains("voxels"));
    CPPUNIT_ASSERT_EQUAL(1, FindLabel(labels, 3)["group"].get<int>());

    const auto groups = mitk::DescribeRestGroups(m_Segmentation);
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), groups.size());
    CPPUNIT_ASSERT_EQUAL(std::string("Group 1"), groups[0]["display_name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::size_t(2), groups[0]["labels"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("Right"), groups[1]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("Right"), groups[1]["display_name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(groups[1], mitk::DescribeRestGroup(m_Segmentation, 1));
  }

  void TestDescribeMeasuredLabels()
  {
    mitk::RestTimeSelection selection;
    auto labels = mitk::DescribeRestLabels(m_Segmentation, selection);

    const auto& liver = FindLabel(labels, 1);
    CPPUNIT_ASSERT_EQUAL(3, liver["voxels"].get<int>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.006, liver["volume_ml"].get<double>(), 1e-9);
    CPPUNIT_ASSERT_EQUAL(nlohmann::json::array({ 1.0, 0.0, 0.0 }), liver["center"]);

    const auto& kidney = FindLabel(labels, 3);
    CPPUNIT_ASSERT_EQUAL(0, kidney["voxels"].get<int>());
    CPPUNIT_ASSERT(kidney["center"].is_null());

    selection.All = true;
    labels = mitk::DescribeRestLabels(m_Segmentation, selection);
    const auto& steps = FindLabel(labels, 2)["time_steps"];
    CPPUNIT_ASSERT_EQUAL(std::size_t(1), steps.size());
    CPPUNIT_ASSERT_EQUAL(1, steps[0]["voxels"].get<int>());
  }

  void TestLabelValueInPath()
  {
    CPPUNIT_ASSERT_EQUAL(std::string("Spleen"), mitk::GetRestLabel(m_Segmentation, CreateRequest("value", "2"))->GetName());

    const std::vector<std::pair<std::string, int>> cases = {
      { "2abc", 400 }, { "+2", 400 }, { "", 400 }, { "65536", 400 }, { "65535", 404 }, { "99", 404 }
    };

    for (const auto& [value, status] : cases)
    {
      const auto failure = GetFailureOf([&]() { mitk::GetRestLabel(m_Segmentation, CreateRequest("value", value)); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + value + "\"", status, failure.Status);
    }

  }

  void TestGroupIndexInPath()
  {
    CPPUNIT_ASSERT_EQUAL(mitk::MultiLabelSegmentation::GroupIndexType(1), mitk::GetRestGroupIndex(m_Segmentation, CreateRequest("index", "1")));

    for (const auto& [index, status] : std::vector<std::pair<std::string, int>>{ { "x", 400 }, { "+1", 400 }, { "2", 404 } })
    {
      const auto failure = GetFailureOf([&]() { mitk::GetRestGroupIndex(m_Segmentation, CreateRequest("index", index)); });
      CPPUNIT_ASSERT_EQUAL_MESSAGE("\"" + index + "\"", status, failure.Status);
    }

  }

  void TestRestrictionsAndSuggestions()
  {
    const auto storage = std::filesystem::temp_directory_path() / "mitkRestSegmentationTest.xml";
    auto* service = mitk::CoreServices::GetPreferencesService();
    service->InitializeStorage(storage);
    auto* node = service->GetSystemPreferences()->Node("org.mitk.views.segmentation");

    node->Put("label set preset", "C:/presets/mine.lsetp");
    auto restrictions = mitk::DescribeRestRestrictions(*node);
    CPPUNIT_ASSERT_EQUAL(std::string("mine.lsetp"), restrictions["label_set_preset"].get<std::string>());
    CPPUNIT_ASSERT(restrictions["label_suggestions_enforced"].is_boolean());

    node->Put("label set preset", "");
    CPPUNIT_ASSERT(mitk::DescribeRestRestrictions(*node)["label_set_preset"].is_null());

    const auto suggestions = mitk::DescribeRestLabelSuggestions();
    CPPUNIT_ASSERT(suggestions["names"].is_array());
    CPPUNIT_ASSERT_EQUAL(restrictions["label_suggestions_enforced"], suggestions["enforced"]);

    service->UninitializeStorage();
    std::filesystem::remove(storage);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestSegmentation)
