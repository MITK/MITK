/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkFakeRestServer.h>

#include <QmitkPulseTimers.h>

#include <mitkDataStorage.h>
#include <mitkImage.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarLine.h>
#include <mitkPlaneGeometry.h>
#include <mitkPointSet.h>
#include <mitkRestFigure.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>
#include <mitkRestRoute.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <QCoreApplication>

#include <chrono>
#include <functional>
#include <memory>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace
{
  using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

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

  mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  mitk::Image::Pointer CreateImage()
  {
    unsigned int dimensions[] = { 20, 20, 3 };
    auto image = mitk::Image::New();
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);
    return image;
  }

  mitk::DataNode::Pointer CreateNode(const std::string& name, mitk::BaseData* data)
  {
    auto node = mitk::DataNode::New();
    node->SetName(name);
    node->SetData(data);
    return node;
  }

  void EnsureApplication()
  {
    // QTimer needs an event dispatcher, which the application of the thread provides.
    static std::unique_ptr<QCoreApplication> application;

    if (QCoreApplication::instance() == nullptr)
    {
      static int argc = 1;
      static char name[] = "mitkRestFigureTest";
      static char* argv[] = { name, nullptr };
      application = std::make_unique<QCoreApplication>(argc, argv);
    }
  }
}

class mitkRestFigureTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRestFigureTestSuite);
  MITK_TEST(TestPredicateSelectsTheFiguresOfTheMeasurementView);
  MITK_TEST(TestCreateFigureKnowsThreeTypes);
  MITK_TEST(TestNamesOfNewFigures);
  MITK_TEST(TestDefaultLook);
  MITK_TEST(TestLookRoundTrip);
  MITK_TEST(TestLookWithoutPropertiesKeepsTheGivenLook);
  MITK_TEST(TestLineWidthIsRoundedInJson);
  MITK_TEST(TestEveryLabelVisibilityRoundTrips);
  MITK_TEST(TestParseTakesFieldsAndBounds);
  MITK_TEST(TestParseIgnoresOtherFields);
  MITK_TEST(TestParseRefusesWrongTypesAndValues);
  MITK_TEST(TestLookFieldNames);
  MITK_TEST(TestApplyChangesOnlyWhatIsGiven);
  MITK_TEST(TestPulseEndsWithItsTimers);
  MITK_TEST(TestCanceledPulseKeepsGoing);
  MITK_TEST(TestDescribeFigureWithoutPlaneGeometry);
  MITK_TEST(TestDescribeFigure);
  MITK_TEST(TestDescribeFigureWithoutImageParent);
  CPPUNIT_TEST_SUITE_END();

  static std::string Parse(const nlohmann::json& body)
  {
    const auto failure = GetFailureOf([&]() { mitk::ParseRestFigureLookChange(body, "/api/v1/planar-figures"); });
    return failure.Status == 400 ? failure.Message : (failure.Status == 0 ? "" : "status " + std::to_string(failure.Status));
  }

public:
  void TestPredicateSelectsTheFiguresOfTheMeasurementView()
  {
    const auto predicate = mitk::GetRestFigurePredicate();

    auto figure = CreateNode("figure", mitk::PlanarLine::New());
    auto helper = CreateNode("helper", mitk::PlanarLine::New());
    helper->SetBoolProperty("helper object", true);
    auto marker = CreateNode("marker", mitk::PlanarLine::New());
    marker->SetBoolProperty("isContourMarker", true);
    auto notHelper = CreateNode("not helper", mitk::PlanarLine::New());
    notHelper->SetBoolProperty("helper object", false);
    auto image = CreateNode("image", CreateImage());

    CPPUNIT_ASSERT(predicate->CheckNode(figure));
    CPPUNIT_ASSERT(predicate->CheckNode(notHelper));
    CPPUNIT_ASSERT(!predicate->CheckNode(helper));
    CPPUNIT_ASSERT(!predicate->CheckNode(marker));
    CPPUNIT_ASSERT(!predicate->CheckNode(image));
  }

  void TestCreateFigureKnowsThreeTypes()
  {
    for (const std::string type : { "line", "circle", "rectangle" })
    {
      const auto figure = mitk::CreateRestFigure(type);
      CPPUNIT_ASSERT_MESSAGE(type, figure.IsNotNull());
      CPPUNIT_ASSERT_EQUAL(type, figure->GetTypeId());
    }

    // Types that PlanarFigure knows but the API does not draw, and spellings that are no type.
    for (const std::string type : { "ellipse", "polygon", "angle", "Line", " line", "" })
      CPPUNIT_ASSERT_MESSAGE("\"" + type + "\"", mitk::CreateRestFigure(type).IsNull());
  }

  void TestNamesOfNewFigures()
  {
    auto storage = mitk::StandaloneDataStorage::New();

    CPPUNIT_ASSERT_EQUAL(std::string("Circle"), mitk::CreateRestFigureName(storage, "circle"));
    CPPUNIT_ASSERT_EQUAL(std::string("Line"), mitk::CreateRestFigureName(storage, "line"));

    storage->Add(CreateNode("Circle", mitk::PlanarCircle::New()));
    CPPUNIT_ASSERT_EQUAL(std::string("Circle 2"), mitk::CreateRestFigureName(storage, "circle"));

    storage->Add(CreateNode("Circle 2", mitk::PlanarCircle::New()));
    CPPUNIT_ASSERT_EQUAL(std::string("Circle 3"), mitk::CreateRestFigureName(storage, "circle"));

    // A name of the user's own that only starts like the base name does not count.
    storage->Add(CreateNode("Rectangle copy", mitk::PlanarCircle::New()));
    CPPUNIT_ASSERT_EQUAL(std::string("Rectangle"), mitk::CreateRestFigureName(storage, "rectangle"));
  }

  void TestDefaultLook()
  {
    const auto json = mitk::ToRestJson(mitk::GetDefaultRestFigureLook());

    CPPUNIT_ASSERT(std::regex_match(json["color"].get<std::string>(), std::regex("#[0-9a-f]{6}")));
    CPPUNIT_ASSERT_EQUAL(2.0, json["line_width"].get<double>());
    CPPUNIT_ASSERT_EQUAL(std::string("on_hover"), json["label"]["show"].get<std::string>());
    CPPUNIT_ASSERT(json["label"]["name"].get<bool>());
    CPPUNIT_ASSERT(json["label"]["measurements"].get<bool>());
    CPPUNIT_ASSERT(!json["show_in_3d"].get<bool>());
    CPPUNIT_ASSERT(!json["pulse"].get<bool>());

    // A node without any of the properties looks like the defaults.
    const auto bare = mitk::DataNode::New();
    CPPUNIT_ASSERT_EQUAL(json.dump(), mitk::ToRestJson(mitk::ReadRestFigureLook(bare, mitk::GetDefaultRestFigureLook())).dump());
  }

  void TestLookRoundTrip()
  {
    const auto node = mitk::DataNode::New();
    node->SetColor(1.0f, 0.5f, 0.0f);
    node->SetFloatProperty("line width", 3.0f);
    node->SetProperty("planarfigure.label", mitk::PlanarFigureVisibilityProperty::New(Visibility::WhenSelected));
    node->SetBoolProperty("planarfigure.drawname", false);
    node->SetBoolProperty("planarfigure.drawquantities", true);
    node->SetBoolProperty("planarfigure.3drendering", true);
    node->SetBoolProperty("animated.pulse", true);

    const auto json = mitk::ToRestJson(mitk::ReadRestFigureLook(node, mitk::GetDefaultRestFigureLook()));

    CPPUNIT_ASSERT_EQUAL(std::string("#ff8000"), json["color"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(3.0, json["line_width"].get<double>());
    CPPUNIT_ASSERT_EQUAL(std::string("when_selected"), json["label"]["show"].get<std::string>());
    CPPUNIT_ASSERT(!json["label"]["name"].get<bool>());
    CPPUNIT_ASSERT(json["label"]["measurements"].get<bool>());
    CPPUNIT_ASSERT(json["show_in_3d"].get<bool>());
    CPPUNIT_ASSERT(json["pulse"].get<bool>());

    // What the API shows, a client can send back as it is, and it changes nothing.
    mitk::RestFigureLookChange change;
    CPPUNIT_ASSERT_NO_THROW(change = mitk::ParseRestFigureLookChange(json, "/api/v1/planar-figures"));

    const auto other = mitk::DataNode::New();
    mitk::ApplyRestFigureLookChange(other, change, nullptr);

    CPPUNIT_ASSERT_EQUAL(json.dump(), mitk::ToRestJson(mitk::ReadRestFigureLook(other, mitk::GetDefaultRestFigureLook())).dump());
  }

  void TestLookWithoutPropertiesKeepsTheGivenLook()
  {
    mitk::RestFigureLook given;
    given.Color.Set(0.0f, 0.0f, 1.0f);
    given.LineWidth = 7.0f;
    given.Label = Visibility::Always;
    given.ShowName = true;
    given.ShowIn3D = true;

    const auto look = mitk::ReadRestFigureLook(mitk::DataNode::New(), given);
    CPPUNIT_ASSERT_EQUAL(mitk::ToRestJson(given).dump(), mitk::ToRestJson(look).dump());

    // A node's own properties win, the others stay.
    const auto node = mitk::DataNode::New();
    node->SetFloatProperty("line width", 4.0f);
    const auto json = mitk::ToRestJson(mitk::ReadRestFigureLook(node, given));
    CPPUNIT_ASSERT_EQUAL(4.0, json["line_width"].get<double>());
    CPPUNIT_ASSERT_EQUAL(std::string("#0000ff"), json["color"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("always"), json["label"]["show"].get<std::string>());
  }

  void TestLineWidthIsRoundedInJson()
  {
    mitk::RestFigureLook look;
    look.LineWidth = 1.2345678f;
    CPPUNIT_ASSERT_EQUAL(1.235, mitk::ToRestJson(look)["line_width"].get<double>());
  }

  void TestEveryLabelVisibilityRoundTrips()
  {
    const std::vector<std::pair<std::string, Visibility>> visibilities = {
      { "always", Visibility::Always },
      { "on_hover", Visibility::OnHover },
      { "when_selected", Visibility::WhenSelected },
      { "on_hover_or_when_selected", Visibility::OnHoverOrWhenSelected },
      { "on_hover_while_selected", Visibility::OnHoverWhileSelected },
      { "never", Visibility::Never }
    };

    for (const auto& [name, visibility] : visibilities)
    {
      const auto change = mitk::ParseRestFigureLookChange({ { "label", { { "show", name } } } }, "/p");
      CPPUNIT_ASSERT_MESSAGE(name, change.Label.has_value() && *change.Label == visibility);

      mitk::RestFigureLook look;
      look.Label = visibility;
      CPPUNIT_ASSERT_EQUAL(name, mitk::ToRestJson(look)["label"]["show"].get<std::string>());
    }
  }

  void TestParseTakesFieldsAndBounds()
  {
    const auto empty = mitk::ParseRestFigureLookChange(nlohmann::json::object(), "/p");
    CPPUNIT_ASSERT(!empty.Color && !empty.LineWidth && !empty.Label && !empty.ShowName && !empty.ShowMeasurements &&
                   !empty.ShowIn3D && !empty.Pulse && !empty.PulseDuration);

    const auto change = mitk::ParseRestFigureLookChange({
      { "color", "#FF8800" },
      { "line_width", 10 },
      { "label", { { "name", false }, { "measurements", true } } },
      { "show_in_3d", true },
      { "pulse", true },
      { "pulse_seconds", 1.5 } }, "/p");

    CPPUNIT_ASSERT_EQUAL(std::string("#ff8800"), mitk::ToRestColor(*change.Color));
    CPPUNIT_ASSERT_EQUAL(10.0f, *change.LineWidth);
    CPPUNIT_ASSERT(!change.Label.has_value());
    CPPUNIT_ASSERT(change.ShowName.has_value() && !*change.ShowName);
    CPPUNIT_ASSERT(change.ShowMeasurements.has_value() && *change.ShowMeasurements);
    CPPUNIT_ASSERT(*change.ShowIn3D);
    CPPUNIT_ASSERT(*change.Pulse);
    CPPUNIT_ASSERT(std::chrono::milliseconds(1500) == *change.PulseDuration);

    // The bounds belong to the range, the line width as an integer or fraction alike.
    CPPUNIT_ASSERT_EQUAL(1.0f, *mitk::ParseRestFigureLookChange({ { "line_width", 1 } }, "/p").LineWidth);
    CPPUNIT_ASSERT_EQUAL(2.5f, *mitk::ParseRestFigureLookChange({ { "line_width", 2.5 } }, "/p").LineWidth);
    CPPUNIT_ASSERT(std::chrono::milliseconds(600000) ==
                   *mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 600 } }, "/p").PulseDuration);
    CPPUNIT_ASSERT(std::chrono::milliseconds(1) ==
                   *mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 0.001 } }, "/p").PulseDuration);
  }

  void TestParseIgnoresOtherFields()
  {
    // The route refuses them with IsRestFigureLookField(); the parser reads the look fields only.
    const auto change = mitk::ParseRestFigureLookChange({ { "name", "x" }, { "type", "line" }, { "pulse", false } }, "/p");
    CPPUNIT_ASSERT(change.Pulse.has_value() && !*change.Pulse);
    CPPUNIT_ASSERT(!change.Color && !change.LineWidth && !change.ShowIn3D);
  }

  void TestParseRefusesWrongTypesAndValues()
  {
    const std::vector<std::pair<nlohmann::json, std::string>> cases = {
      { { { "color", 5 } }, "\"color\"" },
      { { { "color", nullptr } }, "\"color\"" },
      { { { "color", "red" } }, "\"color\"" },
      { { { "color", "#ff88" } }, "\"color\"" },
      { { { "color", "ff8800" } }, "\"color\"" },
      { { { "line_width", "3" } }, "\"line_width\"" },
      { { { "line_width", nullptr } }, "\"line_width\"" },
      { { { "line_width", true } }, "\"line_width\"" },
      { { { "line_width", 0.99 } }, "\"line_width\"" },
      { { { "line_width", 10.01 } }, "\"line_width\"" },
      { { { "label", "always" } }, "\"label\" must be an object" },
      { { { "label", nlohmann::json::array() } }, "\"label\" must be an object" },
      { { { "label", { { "colour", "x" } } } }, "not \"colour\"" },
      { { { "label", { { "show", "sometimes" } } } }, "\"label.show\"" },
      { { { "label", { { "show", 1 } } } }, "\"label.show\"" },
      { { { "label", { { "show", "Always" } } } }, "\"label.show\"" },
      { { { "label", { { "name", "yes" } } } }, "\"label.name\"" },
      { { { "label", { { "measurements", 1 } } } }, "\"label.measurements\"" },
      { { { "show_in_3d", "true" } }, "\"show_in_3d\"" },
      { { { "pulse", 1 } }, "\"pulse\" must be true or false" },
      { { { "pulse_seconds", 5 } }, "\"pulse_seconds\" goes with" },
      { { { "pulse", false }, { "pulse_seconds", 5 } }, "\"pulse_seconds\" goes with" },
      { { { "pulse", true }, { "pulse_seconds", 0 } }, "\"pulse_seconds\" must be" },
      { { { "pulse", true }, { "pulse_seconds", 600.5 } }, "\"pulse_seconds\" must be" },
      { { { "pulse", true }, { "pulse_seconds", "5" } }, "\"pulse_seconds\" must be" },
      { { { "pulse", true }, { "pulse_seconds", nullptr } }, "\"pulse_seconds\" must be" }
    };

    for (const auto& [body, expected] : cases)
    {
      const auto message = Parse(body);
      CPPUNIT_ASSERT_MESSAGE(body.dump() + " gave \"" + message + "\"", message.find(expected) != std::string::npos);
    }

    // The valid fields of a request do not make up for an invalid one.
    CPPUNIT_ASSERT(!Parse({ { "color", "#ff8800" }, { "line_width", 99 } }).empty());
  }

  void TestLookFieldNames()
  {
    for (const std::string key : { "color", "line_width", "label", "show_in_3d", "pulse", "pulse_seconds" })
      CPPUNIT_ASSERT_MESSAGE(key, mitk::IsRestFigureLookField(key));

    for (const std::string key : { "name", "type", "Color", "lineWidth", "label.show", "" })
      CPPUNIT_ASSERT_MESSAGE(key, !mitk::IsRestFigureLookField(key));
  }

  void TestApplyChangesOnlyWhatIsGiven()
  {
    const auto node = mitk::DataNode::New();
    node->SetColor(0.0f, 1.0f, 0.0f);
    node->SetFloatProperty("line width", 2.0f);
    node->SetBoolProperty("planarfigure.drawname", true);

    const auto before = mitk::ToRestJson(mitk::ReadRestFigureLook(node, mitk::GetDefaultRestFigureLook()));
    mitk::ApplyRestFigureLookChange(node, mitk::RestFigureLookChange(), nullptr);
    CPPUNIT_ASSERT_EQUAL(before.dump(), mitk::ToRestJson(mitk::ReadRestFigureLook(node, mitk::GetDefaultRestFigureLook())).dump());

    mitk::ApplyRestFigureLookChange(node, mitk::ParseRestFigureLookChange({ { "line_width", 5 }, { "label", { { "show", "never" } } } }, "/p"), nullptr);
    const auto after = mitk::ToRestJson(mitk::ReadRestFigureLook(node, mitk::GetDefaultRestFigureLook()));

    CPPUNIT_ASSERT_EQUAL(5.0, after["line_width"].get<double>());
    CPPUNIT_ASSERT_EQUAL(std::string("never"), after["label"]["show"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(before["color"].get<std::string>(), after["color"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(true, after["label"]["name"].get<bool>());

    // Without timers a pulse has no end, with or without a duration.
    mitk::ApplyRestFigureLookChange(node, mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 1 } }, "/p"), nullptr);
    bool pulse = false;
    node->GetBoolProperty("animated.pulse", pulse);
    CPPUNIT_ASSERT(pulse);
  }

  void TestPulseEndsWithItsTimers()
  {
    EnsureApplication();

    const auto timed = mitk::DataNode::New();
    const auto untimed = mitk::DataNode::New();
    const auto timedChange = mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 600 } }, "/p");
    const auto untimedChange = mitk::ParseRestFigureLookChange({ { "pulse", true } }, "/p");

    auto timers = std::make_unique<QmitkPulseTimers>(nullptr);
    mitk::ApplyRestFigureLookChange(timed, timedChange, timers.get());
    mitk::ApplyRestFigureLookChange(untimed, untimedChange, timers.get());

    CPPUNIT_ASSERT(IsPulsing(timed));
    CPPUNIT_ASSERT(IsPulsing(untimed));

    // No timer would end the timed pulse anymore, so it ends at once; the other has none to end.
    timers.reset();
    CPPUNIT_ASSERT(!IsPulsing(timed));
    CPPUNIT_ASSERT(IsPulsing(untimed));
  }

  void TestCanceledPulseKeepsGoing()
  {
    EnsureApplication();

    const auto node = mitk::DataNode::New();
    auto timers = std::make_unique<QmitkPulseTimers>(nullptr);

    mitk::ApplyRestFigureLookChange(node, mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 600 } }, "/p"), timers.get());
    mitk::ApplyRestFigureLookChange(node, mitk::ParseRestFigureLookChange({ { "pulse", true } }, "/p"), timers.get());
    timers.reset();

    CPPUNIT_ASSERT(IsPulsing(node));

    // A pulse of a figure that was deleted meanwhile ends without touching anything else.
    auto gone = mitk::DataNode::New();
    timers = std::make_unique<QmitkPulseTimers>(nullptr);
    mitk::ApplyRestFigureLookChange(gone, mitk::ParseRestFigureLookChange({ { "pulse", true }, { "pulse_seconds", 600 } }, "/p"), timers.get());
    gone = nullptr;
    CPPUNIT_ASSERT_NO_THROW(timers.reset());
  }

  void TestDescribeFigureWithoutPlaneGeometry()
  {
    mitk::FakeRestServer server;
    server.Storage = mitk::StandaloneDataStorage::New();

    const auto node = CreateNode("Line", mitk::PlanarLine::New());
    server.Storage->Add(node);

    const auto description = mitk::DescribeRestFigure(server, server.Storage, node);
    const auto json = mitk::ToRestJson(server, description);

    CPPUNIT_ASSERT_EQUAL(std::string("Line"), json["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("line"), json["type"].get<std::string>());
    CPPUNIT_ASSERT(!json["closed"].get<bool>());
    CPPUNIT_ASSERT(json["points"].is_array() && json["points"].empty());
    CPPUNIT_ASSERT(json["measurements"].is_array() && json["measurements"].empty());
    CPPUNIT_ASSERT(json["image"].is_null());
    CPPUNIT_ASSERT_EQUAL(server.GetNodeUid(node), json["uid"].get<std::string>());

    // The look is that of the defaults.
    CPPUNIT_ASSERT_EQUAL(2.0, json["line_width"].get<double>());
    CPPUNIT_ASSERT(json.contains("label") && json.contains("show_in_3d") && json.contains("pulse"));
  }

  void TestDescribeFigure()
  {
    mitk::FakeRestServer server;
    server.Storage = mitk::StandaloneDataStorage::New();

    const auto image = CreateImage();
    const auto imageNode = CreateNode("Image", image);
    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image->GetGeometry(), mitk::AnatomicalPlane::Axial, 1);

    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(plane);
    line->SetFromSpan(P(2, 2), P(12, 2));

    const auto node = CreateNode("Line", line);
    node->SetColor(1.0f, 0.0f, 0.0f);
    server.Storage->Add(imageNode);
    server.Storage->Add(node, imageNode);

    const auto description = mitk::DescribeRestFigure(server, server.Storage, node);
    const auto json = mitk::ToRestJson(server, description);

    CPPUNIT_ASSERT_EQUAL(imageNode.GetPointer(), description.Image.get());
    CPPUNIT_ASSERT_EQUAL(node.GetPointer(), description.Node.get());
    CPPUNIT_ASSERT_EQUAL(server.GetNodeUid(imageNode), json["image"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(server.GetNodeUid(node), json["uid"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("#ff0000"), json["color"].get<std::string>());

    CPPUNIT_ASSERT_EQUAL(std::size_t(2), json["points"].size());
    CPPUNIT_ASSERT_EQUAL(mitk::ToRestJson(line->GetWorldControlPoint(0)).dump(), json["points"][0].dump());
    CPPUNIT_ASSERT_EQUAL(mitk::ToRestJson(line->GetWorldControlPoint(1)).dump(), json["points"][1].dump());

    CPPUNIT_ASSERT_EQUAL(std::size_t(1), json["measurements"].size());
    CPPUNIT_ASSERT_EQUAL(std::string("Length"), json["measurements"][0]["name"].get<std::string>());
    CPPUNIT_ASSERT_EQUAL(std::string("mm"), json["measurements"][0]["unit"].get<std::string>());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, json["measurements"][0]["value"].get<double>(), 1e-6);

    // A closed figure of another type.
    auto circle = mitk::PlanarCircle::New();
    circle->SetPlaneGeometry(plane);
    circle->SetFromSpan(P(10, 10), P(15, 10));
    const auto circleNode = CreateNode("Circle", circle);
    server.Storage->Add(circleNode, imageNode);

    const auto circleJson = mitk::ToRestJson(server, mitk::DescribeRestFigure(server, server.Storage, circleNode));
    CPPUNIT_ASSERT(circleJson["closed"].get<bool>());
    CPPUNIT_ASSERT_EQUAL(std::string("circle"), circleJson["type"].get<std::string>());
    CPPUNIT_ASSERT(circleJson["measurements"].size() > 1);
  }

  void TestDescribeFigureWithoutImageParent()
  {
    mitk::FakeRestServer server;
    server.Storage = mitk::StandaloneDataStorage::New();

    // The parent of a figure is an image or nothing: other sources are not the image that it marks.
    const auto points = CreateNode("Points", mitk::PointSet::New());
    const auto node = CreateNode("Line", mitk::PlanarLine::New());
    server.Storage->Add(points);
    server.Storage->Add(node, points);

    const auto description = mitk::DescribeRestFigure(server, server.Storage, node);
    CPPUNIT_ASSERT(description.Image == nullptr);
    CPPUNIT_ASSERT(mitk::ToRestJson(server, description)["image"].is_null());
  }

private:
  static bool IsPulsing(const mitk::DataNode* node)
  {
    bool pulse = false;
    node->GetBoolProperty("animated.pulse", pulse);
    return pulse;
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRestFigure)
