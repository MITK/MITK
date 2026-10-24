/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureVtkMapper2D.h>

#include "mitkPlanarFigureTessellator.h"
#include "mitkPlanarFigureViewHelpers.h"

#include <mitkBaseRenderer.h>
#include <mitkColorProperty.h>
#include <mitkPlanarBezierCurve.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureInteractor.h>
#include <mitkPlanarFigureVisibilityProperty.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include <vtkActor2D.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkPropAssembly.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkTextActor.h>
#include <vtkTextProperty.h>

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numbers>
#include <sstream>

namespace
{
  using RGBA = mitk::PlanarFigureTessellator::Color;
  using mitk::PlanarFigureViewHelpers::ToDisplay;

  /* The look of planar figures. Lengths are in logical pixels. */
  namespace Style
  {
    constexpr double HaloExtraWidth = 2.0;
    constexpr double HaloFeather = 6.0;
    constexpr double HoveredHaloAlpha = 0.30;
    constexpr double SelectedHaloAlpha = 0.45;
    constexpr double HoveredAndSelectedHaloAlpha = 0.60;
    constexpr double OutlineExtraWidth = 2.0;
    constexpr double OutlineAlpha = 0.4;
    constexpr double HelperWidth = 1.0;
    constexpr double HelperAlpha = 0.85;
    constexpr double HelperOutlineWidth = HelperWidth + OutlineExtraWidth;
    constexpr double HelperOutlineAlpha = 0.45;
    constexpr double DashLength = 4.0;
    constexpr double GapLength = 3.0;
    constexpr double AngleArcRadius = 24.0;
    constexpr double ArrowHeadLength = 12.0;
    constexpr double ArrowHeadHalfAngle = std::numbers::pi / 6.0;
    constexpr double HandleRadius = mitk::PlanarFigureViewHelpers::HandleRadius;
    constexpr double HoveredHandleRadius = 5.5;
    constexpr double HandleOutlineWidth = 1.5;
    constexpr double HandleOutlineAlpha = 0.8;
    constexpr double CenterHandleWidth = 2.0;
    constexpr double InsertionPreviewAlpha = 0.7;
    constexpr double LabelOffset = 8.0;
    constexpr double LabelHorizontalPadding = 6.0;
    constexpr double LabelTopPadding = 6.0;
    constexpr double LabelBottomPadding = 3.0; // Below the baseline, the descent of the font adds to it.
    constexpr double LabelCornerRadius = 4.0;
    constexpr double LabelPlateAlpha = 0.55;
    constexpr double LabelLineSpacing = 1.35; // Relative to the height of a line; VTK uses 1.1.
  }

  using mitk::PlanarFigureViewHelpers::DefaultColor;
  using mitk::PlanarFigureViewHelpers::DefaultPulseFrequency;
  using mitk::PlanarFigureViewHelpers::PulseAmplitude;
  using mitk::PlanarFigureViewHelpers::PulseFrequencyProperty;
  using mitk::PlanarFigureViewHelpers::PulseProperty;

  struct DisplayPolyline
  {
    std::vector<mitk::Point2D> Points;
    bool Closed;
  };

  RGBA WithAlpha(const RGBA &color, double alpha)
  {
    return { color[0], color[1], color[2], alpha };
  }

  // Black on light colors, white on dark ones, so strokes stay visible on any image.
  double GetContrastValue(const float rgb[3])
  {
    const auto luminance = 0.2126 * rgb[0] + 0.7152 * rgb[1] + 0.0722 * rgb[2];
    return luminance >= 0.35 ? 0.0 : 1.0;
  }

  // The shorter sweep between both arms, so mirrored display mappings draw the same arc.
  std::vector<mitk::Point2D> GetArc(const mitk::Point2D &vertex, const mitk::Point2D &end1, const mitk::Point2D &end2, double maximumRadius)
  {
    const auto arm1 = end1 - vertex;
    const auto arm2 = end2 - vertex;
    const auto length1 = arm1.GetNorm();
    const auto length2 = arm2.GetNorm();

    if (length1 <= 0.0 || length2 <= 0.0)
      return {};

    const auto radius = std::min(maximumRadius, 0.8 * std::min(length1, length2));
    const auto start = std::atan2(arm1[1], arm1[0]);
    auto sweep = std::atan2(arm2[1], arm2[0]) - start;

    while (sweep > std::numbers::pi)
      sweep -= 2.0 * std::numbers::pi;

    while (sweep < -std::numbers::pi)
      sweep += 2.0 * std::numbers::pi;

    const auto numberOfSegments = std::max(8, static_cast<int>(std::ceil(std::abs(sweep) * radius / 3.0)));
    std::vector<mitk::Point2D> arc;

    for (int i = 0; i <= numberOfSegments; ++i)
    {
      const auto angle = start + sweep * i / numberOfSegments;
      mitk::Vector2D offset;
      offset[0] = std::cos(angle) * radius;
      offset[1] = std::sin(angle) * radius;
      arc.push_back(vertex + offset);
    }

    return arc;
  }

  // Both sides of an arrow head as one polyline through the tip.
  std::vector<mitk::Point2D> GetArrowHead(const mitk::Point2D &tip, const mitk::Point2D &tail, double length)
  {
    auto direction = tail - tip;
    const auto norm = direction.GetNorm();

    if (norm <= 0.0)
      return {};

    direction /= norm;
    length = std::min(length, 0.5 * norm);

    const auto rotate = [&](double angle) {
      mitk::Vector2D rotated;
      rotated[0] = direction[0] * std::cos(angle) - direction[1] * std::sin(angle);
      rotated[1] = direction[0] * std::sin(angle) + direction[1] * std::cos(angle);
      return tip + rotated * length;
    };

    return { rotate(Style::ArrowHeadHalfAngle), tip, rotate(-Style::ArrowHeadHalfAngle) };
  }

  mitk::PlanarFigureInteractor::Feedback GetFeedback(const mitk::DataNode *node, const mitk::BaseRenderer *renderer)
  {
    const auto *interactor = dynamic_cast<const mitk::PlanarFigureInteractor *>(node->GetDataInteractor().GetPointer());

    return nullptr != interactor
      ? interactor->GetFeedback(renderer)
      : mitk::PlanarFigureInteractor::Feedback{};
  }

  mitk::PlanarFigureVisibilityProperty::Visibility GetVisibility(const mitk::DataNode *node, const std::string &key,
    const mitk::BaseRenderer *renderer, mitk::PlanarFigureVisibilityProperty::Visibility fallback)
  {
    const auto *property = dynamic_cast<const mitk::PlanarFigureVisibilityProperty *>(node->GetProperty(key.c_str(), renderer));
    return nullptr != property ? property->GetVisibility() : fallback;
  }

  // The control polygon of a Bezier curve helps to edit it, so it comes and goes with the handles,
  // while the arc of an angle shows what is measured.
  mitk::PlanarFigureVisibilityProperty::Visibility GetDefaultHelperLines(const mitk::BaseData *data)
  {
    return nullptr != dynamic_cast<const mitk::PlanarBezierCurve *>(data)
      ? mitk::PlanarFigureVisibilityProperty::Visibility::OnHover
      : mitk::PlanarFigureVisibilityProperty::Visibility::Always;
  }

  bool IsShown(mitk::PlanarFigureVisibilityProperty::Visibility visibility, bool isHovered, bool isSelected, bool isBeingPlaced)
  {
    using Visibility = mitk::PlanarFigureVisibilityProperty::Visibility;

    switch (visibility)
    {
      case Visibility::Always:
        return true;
      case Visibility::OnHover:
        return isHovered || isBeingPlaced;
      case Visibility::WhenSelected:
        return isSelected || isBeingPlaced;
      case Visibility::OnHoverOrWhenSelected:
        return isHovered || isSelected || isBeingPlaced;
      case Visibility::OnHoverWhileSelected:
        return (isHovered && isSelected) || isBeingPlaced;
      case Visibility::Never:
        return false;
    }

    return true;
  }

  // The unit symbols are plain ASCII, the label typesets them in UTF-8.
  std::vector<std::string> GetMeasurementLines(const mitk::PlanarFigure *figure)
  {
    std::vector<std::string> lines;

    for (const auto &measurement : figure->GetMeasurements())
    {
      const auto unit = mitk::PlanarFigure::GetUnitSymbol(measurement.Type);

      std::ostringstream line;
      line.imbue(std::locale::classic());
      line << std::fixed << std::setprecision(2) << mitk::PlanarFigure::GetDisplayName(measurement.Type) << ": " << measurement.Value;

      if ("mm2" == unit)
        line << " mm\xC2\xB2";
      else if ("deg" == unit)
        line << "\xC2\xB0";
      else
        line << ' ' << unit;

      lines.push_back(line.str());
    }

    return lines;
  }
}

mitk::PlanarFigureVtkMapper2D::LocalStorage::LocalStorage()
  : m_PropAssembly(vtkSmartPointer<vtkPropAssembly>::New()),
    m_Actor(vtkSmartPointer<vtkActor2D>::New()),
    m_Mapper(vtkSmartPointer<vtkPolyDataMapper2D>::New()),
    m_TextActor(vtkSmartPointer<vtkTextActor>::New())
{
  m_Mapper->ScalarVisibilityOn();
  m_Mapper->SetScalarModeToUsePointData();
  m_Mapper->SetColorModeToDirectScalars();
  m_Actor->SetMapper(m_Mapper);

  m_PropAssembly->AddPart(m_Actor);
  m_PropAssembly->AddPart(m_TextActor);
  m_PropAssembly->SetVisibility(false);
}

mitk::PlanarFigureVtkMapper2D::LocalStorage::~LocalStorage()
{
}

mitk::PlanarFigureVtkMapper2D::PlanarFigureVtkMapper2D()
{
}

mitk::PlanarFigureVtkMapper2D::~PlanarFigureVtkMapper2D()
{
}

void mitk::PlanarFigureVtkMapper2D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  node->AddProperty("color", ColorProperty::New(DefaultColor), renderer, overwrite);
  node->AddProperty("opacity", FloatProperty::New(1.0f), renderer, overwrite);
  node->AddProperty("line width", FloatProperty::New(2.0f), renderer, overwrite);
  node->AddProperty("planarfigure.drawname", BoolProperty::New(true), renderer, overwrite);
  node->AddProperty("planarfigure.drawquantities", BoolProperty::New(true), renderer, overwrite);
  node->AddProperty("planarfigure.label", PlanarFigureVisibilityProperty::New(PlanarFigureVisibilityProperty::Visibility::OnHover), renderer, overwrite);
  node->AddProperty("planarfigure.drawcontrastoutline", BoolProperty::New(true), renderer, overwrite);

  node->AddProperty("planarfigure.helperlines", PlanarFigureVisibilityProperty::New(GetDefaultHelperLines(node->GetData())), renderer, overwrite);
  node->AddProperty("font.size", IntProperty::New(13), renderer, overwrite);
  node->AddProperty(PulseProperty, BoolProperty::New(false), renderer, overwrite);
  node->AddProperty(PulseFrequencyProperty, FloatProperty::New(DefaultPulseFrequency), renderer, overwrite);
}

vtkProp *mitk::PlanarFigureVtkMapper2D::GetVtkProp(BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_PropAssembly;
}

void mitk::PlanarFigureVtkMapper2D::ResetMapper(BaseRenderer *renderer)
{
  auto *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  localStorage->m_PropAssembly->SetVisibility(false);
  localStorage->m_BuildKey.reset();
}

mitk::PlanarFigureVtkMapper2D::BuildKey mitk::PlanarFigureVtkMapper2D::GetBuildKey(BaseRenderer *renderer) const
{
  const auto *node = this->GetDataNode();
  const auto *rendererPlane = renderer->GetCurrentWorldPlaneGeometry();
  const auto *viewportSize = renderer->GetViewportSize();

  const auto *figure = dynamic_cast<const PlanarFigure *>(node->GetData());

  // The modification time of the figure leaves out its plane.
  const auto *figurePlane = figure != nullptr ? figure->GetPlaneGeometry() : nullptr;

  BuildKey key;
  key.FigureTime = figure != nullptr ? figure->GetMTime() : 0;
  key.FigurePlane = figurePlane;
  key.FigurePlaneTime = figurePlane != nullptr ? figurePlane->GetMTime() : 0;
  key.Feedback = GetFeedback(node, renderer);
  key.NodeTime = node->GetMTime();
  key.PropertiesTime = node->GetPropertyList()->GetMTime();
  key.RendererPropertiesTime = node->GetPropertyList(renderer)->GetMTime();
  key.RendererPlane = rendererPlane;
  key.RendererPlaneTime = rendererPlane != nullptr ? rendererPlane->GetMTime() : 0;
  auto *camera = renderer->GetVtkRenderer()->GetActiveCamera();
  std::copy_n(camera->GetPosition(), 3, key.CameraPosition.begin());
  std::copy_n(camera->GetFocalPoint(), 3, key.CameraFocalPoint.begin());
  std::copy_n(camera->GetViewUp(), 3, key.CameraViewUp.begin());
  key.CameraParallelScale = camera->GetParallelScale();
  key.ViewportWidth = viewportSize[0];
  key.ViewportHeight = viewportSize[1];
  key.DPI = renderer->GetRenderWindow()->GetDPI();

  bool pulse = false;
  node->GetBoolProperty(PulseProperty, pulse, renderer);

  if (pulse)
    key.AnimationTime = RenderingManager::GetInstance()->GetAnimationTime();

  return key;
}

void mitk::PlanarFigureVtkMapper2D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  auto *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  const auto key = this->GetBuildKey(renderer);

  if (localStorage->m_BuildKey == key)
    return;

  localStorage->m_BuildKey = key;
  this->Draw(renderer, localStorage);
}

void mitk::PlanarFigureVtkMapper2D::Draw(BaseRenderer *renderer, LocalStorage *localStorage) const
{
  const auto *node = this->GetDataNode();
  const auto *figure = dynamic_cast<const PlanarFigure *>(node->GetData());
  const auto *figurePlane = figure != nullptr ? figure->GetPlaneGeometry() : nullptr;
  const auto *rendererPlane = renderer->GetCurrentWorldPlaneGeometry();

  // A figure of locked shape that is being placed is shown only in the window that moves it.
  bool isPlacedHere = false;
  node->GetBoolProperty("PlanarFigureInitializedWindow", isPlacedHere, renderer);
  const auto isStampElsewhere = figure != nullptr && figure->IsBeingPlaced() && figure->IsShapeLocked() && !isPlacedHere;

  if (figurePlane == nullptr || rendererPlane == nullptr || 0 == figure->GetNumberOfControlPoints() || isStampElsewhere ||
      !node->IsVisible(renderer) || !PlanarFigureViewHelpers::IsOnPlane(figurePlane, rendererPlane))
  {
    localStorage->m_PropAssembly->SetVisibility(false);
    return;
  }

  float rgb[3] = { DefaultColor[0], DefaultColor[1], DefaultColor[2] };
  node->GetColor(rgb, renderer);

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer);

  float logicalLineWidth = 2.0f;
  node->GetFloatProperty("line width", logicalLineWidth, renderer);

  const auto scale = PlanarFigureViewHelpers::GetPixelScale(renderer);
  const auto lineWidth = logicalLineWidth * scale;

  const auto feedback = GetFeedback(node, renderer);
  const auto isHovered = feedback.IsHovered;

  bool isSelected = false;
  node->GetBoolProperty("selected", isSelected, renderer);

  std::vector<DisplayPolyline> contours;
  std::vector<DisplayPolyline> outlines;

  for (const auto &contour : figure->GetContours())
  {
    contours.push_back({ ToDisplay(contour.Points, figurePlane, renderer), contour.Closed });

    if (PlanarFigure::ContourRole::Outline == contour.Role)
      outlines.push_back(contours.back());
  }

  std::vector<DisplayPolyline> helpers;
  std::vector<DisplayPolyline> dashedHelpers;

  // Arrow heads are part of the figure, the other decorations are helper lines.
  const auto helperLines = GetVisibility(node, "planarfigure.helperlines", renderer, GetDefaultHelperLines(figure));
  const auto showHelperLines = IsShown(helperLines, isHovered, isSelected, figure->IsBeingPlaced());

  for (const auto &decoration : figure->GetDecorations())
  {
    if (const auto *controlPolygon = std::get_if<PlanarFigure::HelperPolyline>(&decoration); controlPolygon != nullptr)
    {
      if (showHelperLines)
        dashedHelpers.push_back({ ToDisplay(controlPolygon->Points, figurePlane, renderer), false });
    }
    else if (const auto *arc = std::get_if<PlanarFigure::AngleArc>(&decoration); arc != nullptr)
    {
      if (!showHelperLines)
        continue;

      const auto points = GetArc(ToDisplay(arc->Vertex, figurePlane, renderer), ToDisplay(arc->End1, figurePlane, renderer),
        ToDisplay(arc->End2, figurePlane, renderer), Style::AngleArcRadius * scale);

      if (!points.empty())
        helpers.push_back({ points, false });
    }
    else if (const auto *arrowHead = std::get_if<PlanarFigure::ArrowHead>(&decoration); arrowHead != nullptr)
    {
      const auto points = GetArrowHead(ToDisplay(arrowHead->Tip, figurePlane, renderer),
        ToDisplay(arrowHead->Tail, figurePlane, renderer), Style::ArrowHeadLength * scale);

      if (!points.empty())
        contours.push_back({ points, false });
    }
  }

  // The pulse scales positions around the center of the outline, but neither widths nor sizes.
  bool pulse = false;
  node->GetBoolProperty(PulseProperty, pulse, renderer);

  auto pulseScale = 1.0;
  Point2D pulseCenter;
  pulseCenter.Fill(0.0);

  if (pulse)
  {
    RenderingManager::GetInstance()->RequestAnimationFrame(renderer->GetRenderWindow());

    float frequency = DefaultPulseFrequency;
    node->GetFloatProperty(PulseFrequencyProperty, frequency, renderer);

    pulseScale = PlanarFigureViewHelpers::GetPulseScale(localStorage->m_BuildKey->AnimationTime, frequency);

    std::size_t numberOfPoints = 0;

    for (const auto &polyline : outlines.empty() ? contours : outlines)
    {
      for (const auto &point : polyline.Points)
      {
        pulseCenter[0] += point[0];
        pulseCenter[1] += point[1];
        ++numberOfPoints;
      }
    }

    if (numberOfPoints > 0)
    {
      pulseCenter[0] /= numberOfPoints;
      pulseCenter[1] /= numberOfPoints;
    }

    for (auto *polylines : { &contours, &helpers, &dashedHelpers })
    {
      for (auto &polyline : *polylines)
      {
        for (auto &point : polyline.Points)
          point = pulseCenter + (point - pulseCenter) * pulseScale;
      }
    }

    // The label stays where the outline reaches at most, so it neither moves nor gets covered.
    for (auto &outline : outlines)
    {
      for (auto &point : outline.Points)
        point = pulseCenter + (point - pulseCenter) * (1.0 + PulseAmplitude);
    }
  }

  const auto contrastValue = GetContrastValue(rgb);
  const RGBA contrast = { contrastValue, contrastValue, contrastValue, 1.0 };
  const RGBA base = { rgb[0], rgb[1], rgb[2], opacity };

  PlanarFigureTessellator tessellator;

  if (isHovered || isSelected)
  {
    const auto alpha = isHovered && isSelected ? Style::HoveredAndSelectedHaloAlpha
                     : isSelected ? Style::SelectedHaloAlpha
                     : Style::HoveredHaloAlpha;

    for (const auto &contour : contours)
    {
      tessellator.AddStroke(contour.Points, contour.Closed, lineWidth + Style::HaloExtraWidth * scale,
        WithAlpha(base, alpha * opacity), Style::HaloFeather * scale);
    }
  }

  Point2D viewportLowerLeft;
  viewportLowerLeft.Fill(0.0);

  Point2D viewportUpperRight;
  viewportUpperRight[0] = renderer->GetViewportSize()[0];
  viewportUpperRight[1] = renderer->GetViewportSize()[1];

  bool drawContrastOutline = true;
  node->GetBoolProperty("planarfigure.drawcontrastoutline", drawContrastOutline, renderer);

  if (drawContrastOutline)
  {
    const auto helperOutlineColor = WithAlpha(contrast, Style::HelperOutlineAlpha * opacity);

    for (const auto &helper : helpers)
      tessellator.AddStroke(helper.Points, false, Style::HelperOutlineWidth * scale, helperOutlineColor);

    // A solid outline would fill the gaps, so every dash gets its own.
    for (const auto &helper : dashedHelpers)
    {
      tessellator.AddDashedStroke(helper.Points, false, Style::HelperOutlineWidth * scale, helperOutlineColor,
        Style::DashLength * scale, Style::GapLength * scale, viewportLowerLeft, viewportUpperRight);
    }

    for (const auto &contour : contours)
    {
      tessellator.AddStroke(contour.Points, contour.Closed, lineWidth + Style::OutlineExtraWidth * scale,
        WithAlpha(contrast, Style::OutlineAlpha * opacity));
    }
  }

  for (const auto &helper : helpers)
    tessellator.AddStroke(helper.Points, false, Style::HelperWidth * scale, WithAlpha(base, Style::HelperAlpha * opacity));

  for (const auto &helper : dashedHelpers)
  {
    tessellator.AddDashedStroke(helper.Points, false, Style::HelperWidth * scale, WithAlpha(base, Style::HelperAlpha * opacity),
      Style::DashLength * scale, Style::GapLength * scale, viewportLowerLeft, viewportUpperRight);
  }

  for (const auto &contour : contours)
    tessellator.AddStroke(contour.Points, contour.Closed, lineWidth, base);

  // While a figure is placed point by point, its last point follows the mouse.
  const auto isPlacedPointByPoint = figure->IsBeingPlaced() && PlanarFigure::PlacementKind::Sequence == figure->GetPlacementKind();

  if (isHovered || isPlacedPointByPoint)
  {
    auto handles = figure->GetHandles();

    if (isPlacedPointByPoint && !handles.empty())
      handles.pop_back();

    const auto outlineWidth = Style::HandleOutlineWidth * scale;

    const auto toView = [&](const Point2D &point) {
      return pulseCenter + (ToDisplay(point, figurePlane, renderer) - pulseCenter) * pulseScale;
    };

    // Hollow handles mark the center, and where a Ctrl+click would insert or remove a point.
    const auto addHollowHandle = [&](const Point2D &position, double radius, double alpha) {
      const auto innerRadius = radius - Style::CenterHandleWidth * scale;
      tessellator.AddRing(position, innerRadius - outlineWidth, radius + outlineWidth, WithAlpha(contrast, Style::HandleOutlineAlpha * alpha * opacity));
      tessellator.AddRing(position, innerRadius, radius, WithAlpha(base, alpha * opacity));
    };

    for (std::size_t i = 0; i < handles.size(); ++i)
    {
      const auto isCenter = PlanarFigure::HandleRole::Center == handles[i].Role;

      if (figure->IsShapeLocked() && !isCenter)
        continue;

      const auto position = toView(handles[i].Position);
      const auto isHoveredHandle = feedback.HoveredHandle == i;
      const auto radius = (isHoveredHandle ? Style::HoveredHandleRadius : Style::HandleRadius) * scale;

      if (isHoveredHandle && feedback.IsRemovalPreview)
      {
        addHollowHandle(position, radius, 1.0);

        Vector2D arm;
        arm.Fill(radius * std::numbers::sqrt2 / 2.0);
        Vector2D otherArm = arm;
        otherArm[1] = -arm[1];

        const auto crossColor = WithAlpha(contrast, Style::HandleOutlineAlpha * opacity);
        tessellator.AddStroke({ position - arm, position + arm }, false, outlineWidth, crossColor);
        tessellator.AddStroke({ position - otherArm, position + otherArm }, false, outlineWidth, crossColor);
      }
      else if (isCenter)
      {
        addHollowHandle(position, radius, 1.0);
      }
      else
      {
        tessellator.AddDisc(position, radius + outlineWidth, WithAlpha(contrast, Style::HandleOutlineAlpha * opacity));
        tessellator.AddDisc(position, radius, base);
      }
    }

    if (feedback.InsertionPreview.has_value())
      addHollowHandle(toView(*feedback.InsertionPreview), Style::HandleRadius * scale, Style::InsertionPreviewAlpha);
  }

  // Helper and hidden objects serve other tools, like the box prompts of nnInteractive, and carry no label.
  bool isHelperObject = false;
  node->GetBoolProperty("helper object", isHelperObject, renderer);

  bool isHiddenObject = false;
  node->GetBoolProperty("hidden object", isHiddenObject, renderer);

  const auto label = GetVisibility(node, "planarfigure.label", renderer, PlanarFigureVisibilityProperty::Visibility::OnHover);
  const auto showLabel = !isHelperObject && !isHiddenObject && IsShown(label, isHovered, isSelected, figure->IsBeingPlaced());

  bool drawName = true;
  node->GetBoolProperty("planarfigure.drawname", drawName, renderer);

  bool drawQuantities = true;
  node->GetBoolProperty("planarfigure.drawquantities", drawQuantities, renderer);

  auto text = showLabel && drawName ? node->GetName() : std::string();

  if (showLabel && drawQuantities)
  {
    // Below a name, the measurements are indented.
    const std::string indentation = text.empty() ? "" : "  ";

    for (const auto &line : GetMeasurementLines(figure))
      text += (text.empty() ? "" : "\n") + indentation + line;
  }

  auto *textActor = localStorage->m_TextActor.GetPointer();
  textActor->SetVisibility(false);

  if (!text.empty())
  {
    const Point2D *rightmost = nullptr;
    const Point2D *leftmost = nullptr;

    for (const auto &outline : outlines)
    {
      for (const auto &point : outline.Points)
      {
        if (rightmost == nullptr || point[0] > (*rightmost)[0])
          rightmost = &point;

        if (leftmost == nullptr || point[0] < (*leftmost)[0])
          leftmost = &point;
      }
    }

    if (rightmost != nullptr)
    {
      int fontSize = 13;
      node->GetIntProperty("font.size", fontSize, renderer);

      auto *textProperty = textActor->GetTextProperty();
      textProperty->SetFontSize(fontSize);
      textProperty->SetLineSpacing(Style::LabelLineSpacing);
      textProperty->SetColor(1.0, 1.0, 1.0);
      textProperty->SetOpacity(opacity);
      textProperty->SetBackgroundOpacity(0.0);
      textProperty->SetVerticalJustificationToCentered();
      textProperty->SetJustificationToLeft();
      textProperty->ShadowOff();

      textActor->SetInput(text.c_str());

      // Relative to the position of the text, with the last pixel included.
      double boundingBox[4] = { 0.0, 0.0, 0.0, 0.0 };
      textActor->GetBoundingBox(renderer->GetVtkRenderer(), boundingBox);

      const auto horizontalPadding = Style::LabelHorizontalPadding * scale;
      const auto plateWidth = boundingBox[1] + 1.0 - boundingBox[0] + 2.0 * horizontalPadding;

      // Right of the figure, else left of it, else as far right as the viewport allows.
      const auto offset = Style::LabelOffset * scale;
      const auto viewportWidth = static_cast<double>(renderer->GetViewportSize()[0]);
      const auto right = (*rightmost)[0] + offset;
      const auto left = (*leftmost)[0] - offset;

      auto plateLeft = right;
      auto anchorHeight = (*rightmost)[1];

      if (right + plateWidth > viewportWidth)
      {
        if (left - plateWidth >= 0.0)
        {
          plateLeft = left - plateWidth;
          anchorHeight = (*leftmost)[1];
        }
        else
        {
          plateLeft = std::max(0.0, viewportWidth - plateWidth);
        }
      }

      // VTK draws text at whole pixels, so the plate is placed around the rounded position.
      const auto x = std::round(plateLeft + horizontalPadding - boundingBox[0]);
      const auto y = std::round(anchorHeight);
      textActor->SetPosition(x, y);
      textActor->SetVisibility(true);

      Point2D lowerLeft;
      lowerLeft[0] = x + boundingBox[0] - horizontalPadding;
      lowerLeft[1] = y + boundingBox[2] - Style::LabelBottomPadding * scale;

      Point2D upperRight;
      upperRight[0] = x + boundingBox[1] + 1.0 + horizontalPadding;
      upperRight[1] = y + boundingBox[3] + 1.0 + Style::LabelTopPadding * scale;

      // The text actor is drawn after the figure, so the plate is added last to lie right beneath the text.
      tessellator.AddRoundedRectangle(lowerLeft, upperRight, Style::LabelCornerRadius * scale, { 0.0, 0.0, 0.0, Style::LabelPlateAlpha * opacity });
    }
  }

  localStorage->m_Mapper->SetInputData(tessellator.GetPolyData());
  localStorage->m_PropAssembly->SetVisibility(true);
}
