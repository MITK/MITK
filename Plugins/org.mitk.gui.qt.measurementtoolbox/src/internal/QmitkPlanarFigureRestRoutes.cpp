/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkPlanarFigureRestRoutes.h"

#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkIRestServerService.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlanarFigureVtkMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>
#include <mitkRenderingManager.h>
#include <mitkRestGuiThread.h>
#include <mitkRestPoints.h>
#include <mitkWeakPointer.h>

#include <QColor>
#include <QPointer>
#include <QTimer>

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <map>
#include <optional>

using mitk::CallInGuiThread;
using mitk::ErrorResponse;
using mitk::RestException;
using mitk::RestRequest;
using mitk::RestResponse;
using mitk::RestRoute;

namespace
{
  // The properties that the planar figure mappers draw a figure by.
  constexpr const char* LineWidthProperty = "line width";
  constexpr const char* LabelProperty = "planarfigure.label";
  constexpr const char* ShowNameProperty = "planarfigure.drawname";
  constexpr const char* ShowMeasurementsProperty = "planarfigure.drawquantities";
  constexpr const char* ShowIn3DProperty = "planarfigure.3drendering";
  constexpr const char* PulseProperty = "animated.pulse";

  /** The line widths that the Measurement view offers, in pixels. */
  constexpr double MinimumLineWidth = 1.0;
  constexpr double MaximumLineWidth = 10.0;

  constexpr double MaximumPulseSeconds = 600.0;

  using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

  /** When the label of a figure shows, in the words of the API. */
  struct LabelVisibility
  {
    const char* Name;
    Visibility Value;
  };

  constexpr std::array<LabelVisibility, 6> LabelVisibilities = {{
    { "always", Visibility::Always },
    { "on_hover", Visibility::OnHover },
    { "when_selected", Visibility::WhenSelected },
    { "on_hover_or_when_selected", Visibility::OnHoverOrWhenSelected },
    { "on_hover_while_selected", Visibility::OnHoverWhileSelected },
    { "never", Visibility::Never }
  }};

  [[noreturn]] void Throw(const std::string& code, const std::string& title, const std::string& detail, int status, const std::string& path)
  {
    throw RestException(ErrorResponse::Create(code, title, detail, status, path));
  }

  [[noreturn]] void ThrowInvalid(const std::string& detail, const std::string& path)
  {
    throw RestException(ErrorResponse::InvalidRequest(detail, path));
  }

  /** The figures of the Measurement view: those of helper objects and contour markers belong to other components. */
  mitk::NodePredicateBase::Pointer GetFigurePredicate()
  {
    auto isHelperObject = mitk::NodePredicateProperty::New("helper object", mitk::BoolProperty::New(true));
    auto isContourMarker = mitk::NodePredicateProperty::New("isContourMarker", mitk::BoolProperty::New(true));

    return mitk::NodePredicateAnd::New(
      mitk::TNodePredicateDataType<mitk::PlanarFigure>::New(),
      mitk::NodePredicateNot::New(isHelperObject),
      mitk::NodePredicateNot::New(isContourMarker)).GetPointer();
  }

  /** A figure of the types the API draws, or null for any other type. */
  mitk::PlanarFigure::Pointer CreateFigure(const std::string& type)
  {
    if (type == "line" || type == "circle" || type == "rectangle")
      return mitk::CreatePlanarFigure(type);

    return nullptr;
  }

  /** A name like the Measurement view gives, "Circle", "Circle 2", ..., that no data has yet. */
  std::string CreateName(const mitk::DataStorage* dataStorage, const std::string& type)
  {
    auto baseName = type;
    baseName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(baseName[0])));

    return dataStorage->GetUniqueName(baseName);
  }

  /** The look of a figure, as the API shows and changes it. */
  struct Look
  {
    QColor Color;
    float LineWidth = 0.0f;
    Visibility Label = Visibility::Never;
    bool ShowName = false;
    bool ShowMeasurements = false;
    bool ShowIn3D = false;
    bool Pulse = false;
  };

  /** Reads the look of a node; properties that it lacks keep the values of the given look. */
  Look ReadLook(const mitk::DataNode* node, Look look)
  {
    float color[3];

    if (node->GetColor(color))
      look.Color = QColor::fromRgbF(color[0], color[1], color[2]);

    node->GetFloatProperty(LineWidthProperty, look.LineWidth);

    if (const auto* label = dynamic_cast<const mitk::PlanarFigureVisibilityProperty*>(node->GetProperty(LabelProperty)); label != nullptr)
      look.Label = label->GetVisibility();

    node->GetBoolProperty(ShowNameProperty, look.ShowName);
    node->GetBoolProperty(ShowMeasurementsProperty, look.ShowMeasurements);
    node->GetBoolProperty(ShowIn3DProperty, look.ShowIn3D);
    node->GetBoolProperty(PulseProperty, look.Pulse);

    return look;
  }

  /** The look that the mappers give a figure, for nodes that lack some of its properties. */
  const Look& GetDefaultLook()
  {
    static const auto look = []() {
      auto defaults = mitk::DataNode::New();
      mitk::PlanarFigureVtkMapper2D::SetDefaultProperties(defaults);
      mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(defaults);
      return ReadLook(defaults, Look());
    }();

    return look;
  }

  nlohmann::json ToJson(const Look& look)
  {
    const auto label = std::find_if(LabelVisibilities.begin(), LabelVisibilities.end(),
      [&look](const LabelVisibility& visibility) { return visibility.Value == look.Label; });

    return {
      { "color", look.Color.name().toStdString() },
      { "line_width", look.LineWidth },
      { "label", {
        { "show", label != LabelVisibilities.end() ? label->Name : "never" },
        { "name", look.ShowName },
        { "measurements", look.ShowMeasurements } } },
      { "show_in_3d", look.ShowIn3D },
      { "pulse", look.Pulse }
    };
  }

  /** What a request changes of the look of a figure. */
  struct LookChange
  {
    std::optional<QColor> Color;
    std::optional<float> LineWidth;
    std::optional<Visibility> Label;
    std::optional<bool> ShowName;
    std::optional<bool> ShowMeasurements;
    std::optional<bool> ShowIn3D;
    std::optional<bool> Pulse;

    /** How long a pulse that is switched on lasts; it lasts until switched off without. */
    std::optional<std::chrono::milliseconds> PulseDuration;
  };

  bool IsLookField(const std::string& key)
  {
    return key == "color" || key == "line_width" || key == "label" || key == "show_in_3d" || key == "pulse" ||
      key == "pulse_seconds";
  }

  bool GetBool(const nlohmann::json& object, const std::string& key, const std::string& field, const std::string& path)
  {
    if (!object[key].is_boolean())
      ThrowInvalid("\"" + field + "\" must be true or false.", path);

    return object[key].get<bool>();
  }

  /** Reads the look fields of a request body, before anything changes, so that an invalid request changes nothing. */
  LookChange ParseLookChange(const nlohmann::json& body, const std::string& path)
  {
    LookChange change;

    if (body.contains("color"))
    {
      const auto text = body["color"].is_string() ? body["color"].get<std::string>() : std::string();

      const bool isHex = text.size() == 7 && text[0] == '#' &&
        std::all_of(text.begin() + 1, text.end(), [](unsigned char c) { return std::isxdigit(c) != 0; });

      if (!isHex)
        ThrowInvalid("\"color\" must be written like \"#ff8800\".", path);

      change.Color = QColor(QString::fromStdString(text));
    }

    if (body.contains("line_width"))
    {
      const auto& width = body["line_width"];

      if (!width.is_number() || width.get<double>() < MinimumLineWidth || width.get<double>() > MaximumLineWidth)
        ThrowInvalid("\"line_width\" must be a number of pixels from 1 to 10.", path);

      change.LineWidth = width.get<float>();
    }

    if (body.contains("label"))
    {
      const auto& label = body["label"];

      if (!label.is_object())
        ThrowInvalid("\"label\" must be an object: {show, name, measurements}, all optional.", path);

      for (const auto& item : label.items())
      {
        if (item.key() != "show" && item.key() != "name" && item.key() != "measurements")
          ThrowInvalid("\"label\" takes show, name and measurements, not \"" + item.key() + "\".", path);
      }

      if (label.contains("show"))
      {
        const auto name = label["show"].is_string() ? label["show"].get<std::string>() : std::string();

        const auto visibility = std::find_if(LabelVisibilities.begin(), LabelVisibilities.end(),
          [&name](const LabelVisibility& entry) { return name == entry.Name; });

        if (visibility == LabelVisibilities.end())
        {
          ThrowInvalid("\"label.show\" must be always, on_hover, when_selected, on_hover_or_when_selected, "
            "on_hover_while_selected or never.", path);
        }

        change.Label = visibility->Value;
      }

      if (label.contains("name"))
        change.ShowName = GetBool(label, "name", "label.name", path);

      if (label.contains("measurements"))
        change.ShowMeasurements = GetBool(label, "measurements", "label.measurements", path);
    }

    if (body.contains("show_in_3d"))
      change.ShowIn3D = GetBool(body, "show_in_3d", "show_in_3d", path);

    if (body.contains("pulse"))
      change.Pulse = GetBool(body, "pulse", "pulse", path);

    if (body.contains("pulse_seconds"))
    {
      if (!change.Pulse.value_or(false))
        ThrowInvalid("\"pulse_seconds\" goes with \"pulse\": true.", path);

      const auto& seconds = body["pulse_seconds"];

      if (!seconds.is_number() || seconds.get<double>() <= 0.0 || seconds.get<double>() > MaximumPulseSeconds)
        ThrowInvalid("\"pulse_seconds\" must be a number of seconds above 0 and up to 600.", path);

      change.PulseDuration = std::chrono::milliseconds(std::lround(seconds.get<double>() * 1000.0));
    }

    return change;
  }

  /**
   * Ends pulses after their duration, with one timer per figure. Lives in the
   * GUI thread, as a child of the object that the routes were created with.
   */
  class PulseTimers : public QObject
  {
  public:
    explicit PulseTimers(QObject* parent)
      : QObject(parent)
    {
    }

    /** Ends the pulses that are still timed at once, as no timer would end them anymore. */
    ~PulseTimers() override
    {
      bool hasEnded = false;

      for (const auto& entry : m_Pulses)
      {
        if (auto lockedNode = entry.second.Node.Lock(); lockedNode.IsNotNull())
        {
          lockedNode->SetBoolProperty(PulseProperty, false);
          hasEnded = true;
        }
      }

      if (hasEnded)
        mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }

    /** Switches the pulse of a figure off after the duration, instead of when an earlier call said. */
    void EndPulseAfter(mitk::DataNode* node, std::chrono::milliseconds duration)
    {
      this->Cancel(node);

      auto* timer = new QTimer(this);
      timer->setSingleShot(true);
      m_Pulses[node] = { timer, node };

      // A later call for the figure deletes the timer, so the pulse found here is the one it times.
      connect(timer, &QTimer::timeout, this, [this, node]() {
        const auto it = m_Pulses.find(node);

        if (it == m_Pulses.end())
          return;

        const auto lockedNode = it->second.Node.Lock();
        it->second.Timer->deleteLater();
        m_Pulses.erase(it);

        if (lockedNode.IsNotNull())
        {
          lockedNode->SetBoolProperty(PulseProperty, false);
          mitk::RenderingManager::GetInstance()->RequestUpdateAll();
        }
      });

      timer->start(duration);
    }

    /** Keeps the pulse of a figure as it is, without an end that an earlier call set. */
    void Cancel(const mitk::DataNode* node)
    {
      if (const auto it = m_Pulses.find(node); it != m_Pulses.end())
      {
        delete it->second.Timer.data();
        m_Pulses.erase(it);
      }
    }

  private:
    struct TimedPulse
    {
      QPointer<QTimer> Timer;

      /** The figure, which may be deleted before its pulse ends; another node may get its address then. */
      mitk::WeakPointer<mitk::DataNode> Node;
    };

    std::map<const mitk::DataNode*, TimedPulse> m_Pulses;
  };

  /** Runs in the GUI thread. */
  void ApplyLookChange(mitk::DataNode* node, const LookChange& change, PulseTimers* pulseTimers)
  {
    if (change.Color.has_value())
      node->SetColor(change.Color->redF(), change.Color->greenF(), change.Color->blueF());

    if (change.LineWidth.has_value())
      node->SetFloatProperty(LineWidthProperty, *change.LineWidth);

    if (change.Label.has_value())
      node->SetProperty(LabelProperty, mitk::PlanarFigureVisibilityProperty::New(*change.Label));

    if (change.ShowName.has_value())
      node->SetBoolProperty(ShowNameProperty, *change.ShowName);

    if (change.ShowMeasurements.has_value())
      node->SetBoolProperty(ShowMeasurementsProperty, *change.ShowMeasurements);

    if (change.ShowIn3D.has_value())
      node->SetBoolProperty(ShowIn3DProperty, *change.ShowIn3D);

    if (change.Pulse.has_value())
    {
      node->SetBoolProperty(PulseProperty, *change.Pulse);

      if (pulseTimers != nullptr)
      {
        if (change.PulseDuration.has_value())
          pulseTimers->EndPulseAfter(node, *change.PulseDuration);
        else
          pulseTimers->Cancel(node);
      }
    }
  }

  /**
   * A figure as the API describes it. The uids are left to ToJson(), as only the
   * handler's thread may look them up.
   */
  struct FigureDescription
  {
    mitk::DataNode::Pointer Node;

    /** The image that the figure marks, its parent in the Data Manager. */
    mitk::DataNode::Pointer Image;

    nlohmann::json Json;
  };

  /** Runs in the GUI thread. */
  FigureDescription DescribeFigure(const mitk::DataStorage* dataStorage, mitk::DataNode* node)
  {
    const auto* figure = static_cast<const mitk::PlanarFigure*>(node->GetData());

    auto points = nlohmann::json::array();

    for (std::size_t i = 0; i < figure->GetNumberOfControlPoints(); ++i)
      points.push_back(mitk::ToRestJson(figure->GetWorldControlPoint(i)));

    auto measurements = nlohmann::json::array();

    for (const auto& measurement : figure->GetMeasurements())
    {
      measurements.push_back({
        { "name", mitk::PlanarFigure::GetDisplayName(measurement.Type) },
        { "value", measurement.Value },
        { "unit", mitk::PlanarFigure::GetUnitSymbol(measurement.Type) } });
    }

    const auto images = dataStorage->GetSources(node, mitk::TNodePredicateDataType<mitk::Image>::New(), true);

    nlohmann::json json = {
      { "name", node->GetName() },
      { "type", figure->GetTypeId() },
      { "closed", figure->IsClosed() },
      { "points", points },
      { "measurements", measurements }
    };

    json.update(ToJson(ReadLook(node, GetDefaultLook())));

    return { node, images->empty() ? mitk::DataNode::Pointer() : images->front(), json };
  }

  nlohmann::json ToJson(const mitk::IRestServerService& server, const FigureDescription& figure)
  {
    auto json = figure.Json;
    json["uid"] = server.GetNodeUid(figure.Node);
    json["image"] = figure.Image.IsNotNull() ? nlohmann::json(server.GetNodeUid(figure.Image)) : nlohmann::json(nullptr);
    return json;
  }
}

std::vector<RestRoute> CreatePlanarFigureRoutes(mitk::IRestServerService* server, QObject* owner)
{
  // Only used in the GUI thread, where it lives.
  const QPointer<PulseTimers> pulseTimers = new PulseTimers(owner);

  std::vector<RestRoute> routes;

  RestRoute list;
  list.Method = "GET";
  list.Path = "/planar-figures";
  list.Summary = "List the planar figures, the figures of the Measurement view";
  list.Description =
    "Including the figures the user drew. Returns {data: [{uid, name, type, image, closed, points, "
    "measurements, color, line_width, label, show_in_3d, pulse}]}: type like line, circle, rectangle, ellipse, "
    "polygon or angle; image, the uid of the image the figure marks, its parent in the Data Manager, or null; closed, "
    "whether it encloses a region; points, its control points in millimeters (of a circle its center and a point on "
    "it, of a rectangle two opposite corners); measurements: [{name, value, unit}] as the Measurement view shows "
    "them, like Length, Radius, Diameter, Circumference or Area, in mm and mm2; the look as PATCH "
    "/api/v1/planar-figures/{uid} changes it. GET /api/v1/statistics?mask=<uid> measures the grey values of the "
    "image in or along a figure.";
  list.Handler = [server](const RestRequest& request) {
    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figures = CallInGuiThread(*server, [&]() {
      std::vector<FigureDescription> descriptions;
      const auto nodes = dataStorage->GetSubset(GetFigurePredicate());

      for (const auto& node : *nodes)
      {
        if (static_cast<const mitk::PlanarFigure*>(node->GetData())->IsComplete())
          descriptions.push_back(DescribeFigure(dataStorage, node));
      }

      return descriptions;
    });

    auto json = nlohmann::json::array();

    for (const auto& figure : figures)
      json.push_back(ToJson(*server, figure));

    return RestResponse::FromData(json);
  };
  routes.push_back(list);

  RestRoute create;
  create.Method = "POST";
  create.Path = "/planar-figures";
  create.Summary = "Draw a line, circle or rectangle in a 2D window, as the Measurement view does";
  create.Description =
    "The figure lies in the plane of the slice that the window shows now, as if drawn there with the mouse. Body: "
    "{type: line, circle or rectangle; image: the uid of the image the figure marks, which becomes its parent in "
    "the Data Manager; window: axial, sagittal or coronal; two points, either pixels: [[x, y], [x, y]] of a "
    "screenshot of the window with image_size, as GET /api/v1/rendering/windows/{id}/screenshot describes it, "
    "or points: [[x, y, z], [x, y, z]] in millimeters in the slice the window shows; name (optional), Circle, "
    "Circle 2 and so on without it; and optionally the look fields of PATCH /api/v1/planar-figures/{uid}}. A line "
    "takes its two ends, a circle its center and a point on it, a rectangle two opposite corners. Returns the figure "
    "as GET /api/v1/planar-figures lists it, with status 201. Status 422 if image is no image or a point lies off "
    "the slice. DELETE /api/v1/datastorage/nodes/{uid} removes a figure.";
  create.Handler = [server, pulseTimers](const RestRequest& request) {
    const auto body = request.GetJsonBody();
    const auto type = body.value("type", std::string());

    if (!body.contains("image") || !body["image"].is_string())
      ThrowInvalid("\"image\", the uid of the image that the figure marks, is missing.", request.Path);

    if (!body.contains("window") || !body["window"].is_string())
      ThrowInvalid("\"window\", the 2D window in whose slice to draw, is missing.", request.Path);

    const auto lookChange = ParseLookChange(body, request.Path);
    const auto window = body["window"].get<std::string>();
    const auto imageNode = server->FindNode(body["image"].get<std::string>());

    if (imageNode.IsNull())
      throw RestException(ErrorResponse::NodeNotFound(body["image"].get<std::string>(), request.Path));

    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figure = CallInGuiThread(*server, [&]() {
      auto planarFigure = CreateFigure(type);

      if (planarFigure.IsNull())
        ThrowInvalid("\"type\" must be line, circle or rectangle.", request.Path);

      if (dynamic_cast<const mitk::Image*>(imageNode->GetData()) == nullptr)
        Throw("NOT_AN_IMAGE", "Unprocessable Content", "\"" + imageNode->GetName() + "\" is no image.", 422, request.Path);

      const auto* renderer = mitk::GetRestRenderer(window, request.Path);

      if (renderer->GetMapperID() != mitk::BaseRenderer::Standard2D)
        ThrowInvalid("Figures lie in the slice of a 2D window, not in the 3D window.", request.Path);

      const auto* plane = renderer->GetCurrentWorldPlaneGeometry();
      const auto points = mitk::GetRestWorldPoints(body, request.Path, 2);

      if (points.size() != 2)
      {
        ThrowInvalid("Give two points: the ends of a line, the center of a circle and a point on it, or two opposite "
          "corners of a rectangle.", request.Path);
      }

      // Points from elsewhere, like a pick, may lie anywhere within the slice, up to half its thickness off its plane.
      const auto halfThickness = plane->GetExtentInMM(2) / 2.0;
      std::array<mitk::Point2D, 2> planePoints;

      for (std::size_t i = 0; i < points.size(); ++i)
      {
        if (plane->Distance(points[i]) > halfThickness)
        {
          Throw("POINT_OFF_SLICE", "Unprocessable Content", "A point lies off the slice that the " + window +
            " window shows. Show the slice through the points first, for example with PUT "
            "/api/v1/rendering/selected-position, or give pixels of a screenshot.", 422, request.Path);
        }

        plane->Map(points[i], planePoints[i]);
      }

      const auto offset = planePoints[1] - planePoints[0];

      const bool isEmpty = type == "rectangle"
        ? (std::abs(offset[0]) < mitk::eps || std::abs(offset[1]) < mitk::eps)
        : offset.GetNorm() < mitk::eps;

      if (isEmpty)
        ThrowInvalid("The two points span no figure: they coincide or, for a rectangle, lie in one row or column.", request.Path);

      // As the planar figure interactor spans a figure drawn with the mouse,
      // from where the mouse is pressed to where it is released.
      planarFigure->SetPlaneGeometry(plane);
      planarFigure->SetFromSpan(planePoints[0], planePoints[1]);

      const auto name = body.value("name", std::string());

      auto node = mitk::DataNode::New();
      node->SetName(name.empty() ? CreateName(dataStorage, type) : name);
      node->SetData(planarFigure);
      ApplyLookChange(node, lookChange, pulseTimers);

      dataStorage->Add(node, imageNode);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return DescribeFigure(dataStorage, node);
    });

    return RestResponse::FromData(ToJson(*server, figure), 201);
  };
  routes.push_back(create);

  RestRoute change;
  change.Method = "PATCH";
  change.Path = "/planar-figures/:uid";
  change.Summary = "Change the look of a planar figure: color, line width, label, display in 3D and pulse";
  change.Description =
    "Path parameter: uid of a figure as GET /api/v1/planar-figures lists it. Body, all optional: {color: "
    "\"#rrggbb\"; line_width: pixels from 1 to 10; label: {show: when the label with the name and measurements shows, "
    "always, on_hover, when_selected, on_hover_or_when_selected, on_hover_while_selected or never; name and "
    "measurements: whether the label has them}; show_in_3d: whether the 3D window shows the figure too; pulse: "
    "whether the figure grows and shrinks to draw attention to it; pulse_seconds, with pulse true: the seconds after "
    "which the pulse stops by itself, up to 600; without it, the figure pulses until pulse is false}. Other fields "
    "are refused, and an invalid field changes nothing. The render windows show the change right away. Returns the "
    "figure as GET /api/v1/planar-figures lists it. Status 422 if uid is no planar figure of the Measurement view.";
  change.Handler = [server, pulseTimers](const RestRequest& request) {
    const auto body = request.GetJsonBody();

    if (!body.is_object())
      ThrowInvalid("The body must be an object with the fields to change.", request.Path);

    for (const auto& item : body.items())
    {
      if (!IsLookField(item.key()))
      {
        ThrowInvalid("\"" + item.key() + "\" is no field of the look. The fields are color, line_width, label, "
          "show_in_3d, pulse and pulse_seconds.", request.Path);
      }
    }

    const auto lookChange = ParseLookChange(body, request.Path);
    const auto& uid = request.PathParams.at("uid");
    const auto node = server->FindNode(uid);

    if (node.IsNull())
      throw RestException(ErrorResponse::NodeNotFound(uid, request.Path));

    const auto dataStorage = server->GetDataStorage();

    if (dataStorage.IsNull())
      throw RestException(ErrorResponse::DataStorageNotAvailable(request.Path));

    const auto figure = CallInGuiThread(*server, [&]() {
      const auto* planarFigure = dynamic_cast<const mitk::PlanarFigure*>(node->GetData());

      if (planarFigure == nullptr || !planarFigure->IsComplete() || !GetFigurePredicate()->CheckNode(node))
      {
        Throw("NOT_A_PLANAR_FIGURE", "Unprocessable Content", "\"" + node->GetName() + "\" is no planar figure of "
          "the Measurement view.", 422, request.Path);
      }

      ApplyLookChange(node, lookChange, pulseTimers);
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      return DescribeFigure(dataStorage, node);
    });

    return RestResponse::FromData(ToJson(*server, figure));
  };
  routes.push_back(change);

  return routes;
}
