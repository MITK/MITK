/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRestFigure.h>

#include <QmitkPulseTimers.h>

#include <mitkDataStorage.h>
#include <mitkErrorResponse.h>
#include <mitkImage.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>
#include <mitkPlanarFigureVtkMapper2D.h>
#include <mitkPlanarFigureVtkMapper3D.h>
#include <mitkRestHelpers.h>
#include <mitkRestPoints.h>

#include <QColor>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>

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

  [[noreturn]] void ThrowInvalid(const std::string& detail, const std::string& path)
  {
    throw mitk::RestException(mitk::ErrorResponse::InvalidRequest(detail, path));
  }

  bool GetBool(const nlohmann::json& object, const std::string& key, const std::string& field, const std::string& path)
  {
    if (!object[key].is_boolean())
      ThrowInvalid("\"" + field + "\" must be true or false.", path);

    return object[key].get<bool>();
  }
}

mitk::NodePredicateBase::Pointer mitk::GetRestFigurePredicate()
{
  auto isHelperObject = NodePredicateProperty::New("helper object", BoolProperty::New(true));
  auto isContourMarker = NodePredicateProperty::New("isContourMarker", BoolProperty::New(true));

  return NodePredicateAnd::New(
    TNodePredicateDataType<PlanarFigure>::New(),
    NodePredicateNot::New(isHelperObject),
    NodePredicateNot::New(isContourMarker)).GetPointer();
}

mitk::PlanarFigure::Pointer mitk::CreateRestFigure(const std::string& type)
{
  if (type == "line" || type == "circle" || type == "rectangle")
    return CreatePlanarFigure(type);

  return nullptr;
}

std::string mitk::CreateRestFigureName(const DataStorage* dataStorage, const std::string& type)
{
  auto baseName = type;
  baseName[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(baseName[0])));

  return dataStorage->GetUniqueName(baseName);
}

mitk::RestFigureLook mitk::ReadRestFigureLook(const DataNode* node, RestFigureLook look)
{
  float color[3];

  if (node->GetColor(color))
    look.Color.Set(color[0], color[1], color[2]);

  node->GetFloatProperty(LineWidthProperty, look.LineWidth);

  if (const auto* label = dynamic_cast<const PlanarFigureVisibilityProperty*>(node->GetProperty(LabelProperty)); label != nullptr)
    look.Label = label->GetVisibility();

  node->GetBoolProperty(ShowNameProperty, look.ShowName);
  node->GetBoolProperty(ShowMeasurementsProperty, look.ShowMeasurements);
  node->GetBoolProperty(ShowIn3DProperty, look.ShowIn3D);
  node->GetBoolProperty(PulseProperty, look.Pulse);

  return look;
}

const mitk::RestFigureLook& mitk::GetDefaultRestFigureLook()
{
  static const auto look = []() {
    auto defaults = DataNode::New();
    PlanarFigureVtkMapper2D::SetDefaultProperties(defaults);
    PlanarFigureVtkMapper3D::SetDefaultProperties(defaults);
    return ReadRestFigureLook(defaults, RestFigureLook());
  }();

  return look;
}

nlohmann::json mitk::ToRestJson(const RestFigureLook& look)
{
  const auto label = std::find_if(LabelVisibilities.begin(), LabelVisibilities.end(),
    [&look](const LabelVisibility& visibility) { return visibility.Value == look.Label; });

  const auto color = QColor::fromRgbF(look.Color[0], look.Color[1], look.Color[2]);

  return {
    { "color", color.name().toStdString() },
    { "line_width", std::round(look.LineWidth * 1000.0) / 1000.0 },
    { "label", {
      { "show", label != LabelVisibilities.end() ? label->Name : "never" },
      { "name", look.ShowName },
      { "measurements", look.ShowMeasurements } } },
    { "show_in_3d", look.ShowIn3D },
    { "pulse", look.Pulse }
  };
}

bool mitk::IsRestFigureLookField(const std::string& key)
{
  return key == "color" || key == "line_width" || key == "label" || key == "show_in_3d" || key == "pulse" ||
    key == "pulse_seconds";
}

mitk::RestFigureLookChange mitk::ParseRestFigureLookChange(const nlohmann::json& body, const std::string& path)
{
  RestFigureLookChange change;

  if (body.contains("color"))
  {
    change.Color = ParseRestColor(body["color"].is_string() ? body["color"].get<std::string>() : std::string());

    if (!change.Color.has_value())
      ThrowInvalid("\"color\" must be written like \"#ff8800\".", path);
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

void mitk::ApplyRestFigureLookChange(DataNode* node, const RestFigureLookChange& change, QmitkPulseTimers* pulseTimers)
{
  if (change.Color.has_value())
    node->SetColor(*change.Color);

  if (change.LineWidth.has_value())
    node->SetFloatProperty(LineWidthProperty, *change.LineWidth);

  if (change.Label.has_value())
    node->SetProperty(LabelProperty, PlanarFigureVisibilityProperty::New(*change.Label));

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

mitk::RestFigureDescription mitk::DescribeRestFigure(const IRestServerService& server, const DataStorage* dataStorage, DataNode* node)
{
  const auto* figure = static_cast<const PlanarFigure*>(node->GetData());

  auto points = nlohmann::json::array();

  for (std::size_t i = 0; i < figure->GetNumberOfControlPoints(); ++i)
    points.push_back(ToRestJson(figure->GetWorldControlPoint(i)));

  auto measurements = nlohmann::json::array();

  for (const auto& measurement : figure->GetMeasurements())
  {
    measurements.push_back({
      { "name", PlanarFigure::GetDisplayName(measurement.Type) },
      { "value", measurement.Value },
      { "unit", PlanarFigure::GetUnitSymbol(measurement.Type) } });
  }

  const auto images = dataStorage->GetSources(node, TNodePredicateDataType<Image>::New(), true);

  nlohmann::json json = {
    { "name", node->GetName() },
    { "type", figure->GetTypeId() },
    { "closed", figure->IsClosed() },
    { "points", points },
    { "measurements", measurements }
  };

  json.update(ToRestJson(ReadRestFigureLook(node, GetDefaultRestFigureLook())));

  return { server.HoldNode(node), server.HoldNode(images->empty() ? nullptr : images->front().GetPointer()), json };
}

nlohmann::json mitk::ToRestJson(const IRestServerService& server, const RestFigureDescription& figure)
{
  auto json = figure.Json;
  json["uid"] = server.GetNodeUid(figure.Node.get());
  json["image"] = figure.Image != nullptr ? nlohmann::json(server.GetNodeUid(figure.Image.get())) : nlohmann::json(nullptr);
  return json;
}
