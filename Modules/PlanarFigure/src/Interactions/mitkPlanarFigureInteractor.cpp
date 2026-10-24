/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureInteractor.h>

#include "mitkPlanarFigureViewHelpers.h"

#include <mitkAbstractTransformGeometry.h>
#include <mitkBaseRenderer.h>
#include <mitkDataStorage.h>
#include <mitkExclusiveInteraction.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionKeyReleaseEvent.h>
#include <mitkMouseMoveEvent.h>
#include <mitkMousePressEvent.h>
#include <mitkMouseReleaseEvent.h>
#include <mitkNodePredicateDataType.h>
#include <mitkPlanarFigure.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include <algorithm>
#include <limits>
#include <locale>
#include <sstream>

namespace mitk
{
  itkEventMacroDefinition(PlanarFigureEvent, itk::AnyEvent);
  itkEventMacroDefinition(EndPlacementPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDefinition(SelectPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDefinition(EndInteractionPlanarFigureEvent, PlanarFigureEvent);
  itkEventMacroDefinition(PointMovedPlanarFigureEvent, PlanarFigureEvent);
}

namespace
{
  // A point of a figure placed point by point that is closer to a placed one,
  // in logical pixels, belongs to a double click or would coincide with it.
  constexpr double MinimumPointDistance = 5.0;

  mitk::PlanarFigureInteractor *FeedbackOwner = nullptr;

  double GetDistanceParameter(const mitk::PropertyList *properties, const std::string &key, double fallback)
  {
    std::string value;

    if (!properties->GetStringProperty(key.c_str(), value))
      return fallback;

    std::istringstream stream(value);
    stream.imbue(std::locale::classic());

    double distance = 0.0;

    if (!(stream >> distance) || !(stream >> std::ws).eof() || !(distance >= 0.0))
    {
      MITK_WARN << "The planar figure interactor ignores " << key << " = \"" << value << "\", which is no distance.";
      return fallback;
    }

    return distance;
  }

  mitk::Point2D GetClosestPointOnSegment(const mitk::Point2D &point, const mitk::Point2D &start, const mitk::Point2D &end)
  {
    const auto segment = end - start;
    const auto squaredLength = segment.GetSquaredNorm();

    if (squaredLength <= 0.0)
      return start;

    const auto t = std::clamp(((point - start) * segment) / squaredLength, 0.0, 1.0);
    return start + segment * t;
  }

  double GetDistanceToSegment(const mitk::Point2D &point, const mitk::Point2D &start, const mitk::Point2D &end)
  {
    return point.EuclideanDistanceTo(GetClosestPointOnSegment(point, start, end));
  }

  bool IsControlHeld(const mitk::InteractionEvent *interactionEvent)
  {
    // The modifiers of the press and release of Ctrl itself do not tell whether it is held afterwards.
    if (const auto *keyEvent = dynamic_cast<const mitk::InteractionKeyEvent *>(interactionEvent); nullptr != keyEvent)
    {
      return mitk::InteractionEvent::KeyControl == keyEvent->GetKey()
        || 0 != (keyEvent->GetModifiers() & mitk::InteractionEvent::ControlKey);
    }

    if (const auto *keyEvent = dynamic_cast<const mitk::InteractionKeyReleaseEvent *>(interactionEvent); nullptr != keyEvent)
    {
      return mitk::InteractionEvent::KeyControl != keyEvent->GetKey()
        && 0 != (keyEvent->GetModifiers() & mitk::InteractionEvent::ControlKey);
    }

    if (const auto *moveEvent = dynamic_cast<const mitk::MouseMoveEvent *>(interactionEvent); nullptr != moveEvent)
      return 0 != (moveEvent->GetModifiers() & mitk::InteractionEvent::ControlKey);

    if (const auto *pressEvent = dynamic_cast<const mitk::MousePressEvent *>(interactionEvent); nullptr != pressEvent)
      return 0 != (pressEvent->GetModifiers() & mitk::InteractionEvent::ControlKey);

    if (const auto *releaseEvent = dynamic_cast<const mitk::MouseReleaseEvent *>(interactionEvent); nullptr != releaseEvent)
      return 0 != (releaseEvent->GetModifiers() & mitk::InteractionEvent::ControlKey);

    return false;
  }

  bool IsInside(const mitk::Point2D &point, const std::vector<mitk::Point2D> &polygon)
  {
    bool isInside = false;

    for (std::size_t i = 0, j = polygon.size() - 1; i < polygon.size(); j = i++)
    {
      const auto &a = polygon[i];
      const auto &b = polygon[j];

      if ((a[1] > point[1]) != (b[1] > point[1]) && point[0] < (b[0] - a[0]) * (point[1] - a[1]) / (b[1] - a[1]) + a[0])
        isInside = !isInside;
    }

    return isInside;
  }

  int GetCursor(const mitk::PlanarFigureInteractor::Feedback &feedback)
  {
    if (!feedback.IsHovered)
      return VTK_CURSOR_DEFAULT;

    if (feedback.InsertionPreview.has_value() || feedback.IsRemovalPreview)
      return VTK_CURSOR_CROSSHAIR;

    return feedback.HoveredHandle.has_value()
      ? VTK_CURSOR_HAND
      : VTK_CURSOR_SIZEALL;
  }

  // Holes are subtracted.
  double GetEnclosedArea(const std::vector<mitk::PlanarFigure::Contour> &contours)
  {
    auto area = 0.0;

    for (const auto &contour : contours)
    {
      const auto &points = contour.Points;

      if (!contour.Closed || points.size() < 3)
        continue;

      auto twiceArea = 0.0;

      for (std::size_t i = 0, j = points.size() - 1; i < points.size(); j = i++)
        twiceArea += points[j][0] * points[i][1] - points[i][0] * points[j][1];

      const auto contourArea = 0.5 * std::abs(twiceArea);
      area += mitk::PlanarFigure::ContourRole::Hole == contour.Role ? -contourArea : contourArea;
    }

    return area;
  }

  mitk::Point2D GetDisplayPosition(const mitk::InteractionEvent *interactionEvent)
  {
    if (const auto *positionEvent = dynamic_cast<const mitk::InteractionPositionEvent *>(interactionEvent); nullptr != positionEvent)
      return positionEvent->GetPointerPositionOnScreen();

    mitk::Point2D origin;
    origin.Fill(0.0);
    return origin;
  }

  vtkRenderWindow *GetRenderWindow(const mitk::InteractionEvent *interactionEvent)
  {
    return interactionEvent->GetSender()->GetRenderWindow();
  }

  mitk::PlanarFigureInteractor::Feedback GetHoverFeedback(const mitk::InteractionEvent *interactionEvent,
                                                          const std::optional<std::size_t> &hoveredHandle)
  {
    mitk::PlanarFigureInteractor::Feedback feedback;
    feedback.Renderer = interactionEvent->GetSender();
    feedback.IsHovered = true;
    feedback.HoveredHandle = hoveredHandle;
    return feedback;
  }

  // The slice the render window shows, with the spacing of the data it was made for.
  const mitk::PlaneGeometry *GetSlicePlane(mitk::BaseRenderer *renderer)
  {
    const auto *plane = renderer->GetSliceNavigationController()->GetCurrentPlaneGeometry();

    return nullptr != plane
      ? plane
      : renderer->GetCurrentWorldPlaneGeometry();
  }

  // The lower and upper corner of the bounds of a plane, in plane coordinates.
  std::pair<mitk::Point2D, mitk::Point2D> GetPlaneExtent(const mitk::PlaneGeometry *plane)
  {
    const auto bounds = plane->GetBounds();

    // Bounds of image geometries start at the center of the first pixel.
    const auto shift = plane->GetImageGeometry() ? 0.5 : 0.0;

    mitk::Point2D lowerIndex;
    lowerIndex[0] = bounds[0] - shift;
    lowerIndex[1] = bounds[2] - shift;

    mitk::Point2D upperIndex;
    upperIndex[0] = bounds[1] - shift;
    upperIndex[1] = bounds[3] - shift;

    mitk::Point2D lower;
    plane->IndexToWorld(lowerIndex, lower);

    mitk::Point2D upper;
    plane->IndexToWorld(upperIndex, upper);

    return { lower, upper };
  }

  mitk::Point2D ClampToPlane(const mitk::PlaneGeometry *plane, mitk::Point2D point)
  {
    const auto [lower, upper] = GetPlaneExtent(plane);

    for (int i = 0; i < 2; ++i)
      point[i] = std::clamp(point[i], lower[i], upper[i]);

    return point;
  }

  // Shortens an offset so that it moves none of the points out of the plane, or further out if one already is.
  mitk::Vector2D ClampOffsetToPlane(const mitk::PlaneGeometry *plane, const std::vector<mitk::Point2D> &points, mitk::Vector2D offset)
  {
    const auto [lower, upper] = GetPlaneExtent(plane);

    for (int i = 0; i < 2; ++i)
    {
      auto minimum = -std::numeric_limits<double>::infinity();
      auto maximum = std::numeric_limits<double>::infinity();

      for (const auto &point : points)
      {
        minimum = std::max(minimum, lower[i] - point[i]);
        maximum = std::min(maximum, upper[i] - point[i]);
      }

      offset[i] = std::clamp(offset[i], std::min(minimum, 0.0), std::max(maximum, 0.0));
    }

    return offset;
  }

  // The extent of a round figure may leave the plane, its center may not, like the vertices of other figures.
  bool IsBoundToPlane(mitk::PlanarFigure::HandleRole role)
  {
    return mitk::PlanarFigure::HandleRole::Extent != role;
  }

  std::vector<mitk::Point2D> GetPositionsBoundToPlane(const mitk::PlanarFigure &figure)
  {
    std::vector<mitk::Point2D> positions;

    for (const auto &handle : figure.GetHandles())
    {
      if (IsBoundToPlane(handle.Role))
        positions.push_back(handle.Position);
    }

    return positions;
  }

  // Round figures span from their center to an extent.
  bool IsSpanEndBoundToPlane(const mitk::PlanarFigure &figure)
  {
    const auto handles = figure.GetHandles();

    return std::none_of(handles.begin(), handles.end(), [](const mitk::PlanarFigure::Handle &handle) {
      return mitk::PlanarFigure::HandleRole::Center == handle.Role;
    });
  }

  // A stored event keeps a raw pointer to its renderer, which dies with its render window,
  // so the event is only used while the renderer is still registered.
  const mitk::InteractionPositionEvent *GetIfRendererExists(const mitk::InteractionPositionEvent *positionEvent)
  {
    if (nullptr == positionEvent)
      return nullptr;

    const auto &renderers = mitk::BaseRenderer::baseRendererMap;
    const auto *renderer = positionEvent->GetSender();

    return std::any_of(renderers.begin(), renderers.end(), [renderer](const auto &entry) { return entry.second == renderer; })
      ? positionEvent
      : nullptr;
  }

  // Selecting a figure that is selected already would collapse a selection of several nodes.
  void SelectUnlessSelected(mitk::DataNode *node, mitk::PlanarFigure *figure)
  {
    if (!node->IsSelected())
      figure->InvokeEvent(mitk::SelectPlanarFigureEvent());
  }
}

mitk::PlanarFigureInteractor::PlanarFigureInteractor()
{
  m_PressPosition.Fill(0.0);
  m_GrabOffset.Fill(0.0);
  m_SpanStart.Fill(0.0);
}

mitk::PlanarFigureInteractor::~PlanarFigureInteractor()
{
  if (this == FeedbackOwner)
    FeedbackOwner = nullptr;

  if (nullptr != m_FeedbackWindow)
  {
    m_FeedbackWindow->SetCurrentCursor(VTK_CURSOR_DEFAULT);

    // The handles shown for this interactor would stay until the next repaint.
    if (m_Feedback != Feedback{})
      RenderingManager::GetInstance()->RequestUpdate(m_FeedbackWindow);
  }
}

void mitk::PlanarFigureInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("is_empty", IsEmpty);
  CONNECT_CONDITION("is_on_flat_plane", IsOnFlatPlane);
  CONNECT_CONDITION("is_placed_by_span", IsPlacedBySpan);
  CONNECT_CONDITION("is_stamp", IsStamp);
  CONNECT_CONDITION("is_complete", IsComplete);
  CONNECT_CONDITION("is_hit", IsHit);
  CONNECT_CONDITION("is_beyond_drag_threshold", IsBeyondDragThreshold);
  CONNECT_CONDITION("is_valid_point", IsValidPoint);
  CONNECT_CONDITION("reaches_maximum", ReachesMaximum);
  CONNECT_CONDITION("can_finish", CanFinish);
  CONNECT_CONDITION("is_removable", IsRemovable);
  CONNECT_CONDITION("is_insertable", IsInsertable);

  CONNECT_FUNCTION("update_hover", UpdateHover);
  CONNECT_FUNCTION("end_hover", EndHover);
  CONNECT_FUNCTION("press", Press);
  CONNECT_FUNCTION("drag", Drag);
  CONNECT_FUNCTION("end_drag", EndDrag);
  CONNECT_FUNCTION("remove_point", RemovePoint);
  CONNECT_FUNCTION("insert_point", InsertPoint);
  CONNECT_FUNCTION("begin_placement", BeginPlacement);
  CONNECT_FUNCTION("move_floating_point", MoveFloatingPoint);
  CONNECT_FUNCTION("append_point", AppendPoint);
  CONNECT_FUNCTION("remove_floating_point", RemoveFloatingPoint);
  CONNECT_FUNCTION("remove_last_point", RemoveLastPoint);
  CONNECT_FUNCTION("end_placement", EndPlacement);
  CONNECT_FUNCTION("move_stamp", MoveStamp);
}

void mitk::PlanarFigureInteractor::ConfigurationChanged()
{
  const auto properties = this->GetAttributes();

  if (properties.IsNull())
    return;

  m_HitTolerance = GetDistanceParameter(properties, "hitTolerance", DefaultHitTolerance);
  m_DragThreshold = GetDistanceParameter(properties, "dragThreshold", DefaultDragThreshold);
}

void mitk::PlanarFigureInteractor::DataNodeChanged()
{
  this->ResetToStartState();
  m_Grab = {};
  m_LastMove = nullptr;
  this->SetFeedback({}, nullptr);

  Superclass::DataNodeChanged();
}

bool mitk::PlanarFigureInteractor::FilterEvents(InteractionEvent *interactionEvent, DataNode *dataNode)
{
  const auto *renderer = interactionEvent->GetSender();

  return nullptr != renderer
    && BaseRenderer::Standard2D == renderer->GetMapperID()
    && Superclass::FilterEvents(interactionEvent, dataNode);
}

mitk::PlanarFigureInteractor::Feedback mitk::PlanarFigureInteractor::GetFeedback(const BaseRenderer *renderer) const
{
  return renderer == m_Feedback.Renderer ? m_Feedback : Feedback{};
}

void mitk::PlanarFigureInteractor::SetFeedback(const Feedback &feedback, vtkRenderWindow *renderWindow)
{
  if (feedback == m_Feedback)
    return;

  if (nullptr != feedback.Renderer && this != FeedbackOwner)
  {
    // The previous owner may not see the move that ends its hover, so it must not restore the feedback.
    if (nullptr != FeedbackOwner)
    {
      FeedbackOwner->m_LastMove = nullptr;
      FeedbackOwner->SetFeedback({}, nullptr);
    }

    FeedbackOwner = this;
  }
  else if (nullptr == feedback.Renderer && this == FeedbackOwner)
  {
    FeedbackOwner = nullptr;
  }

  auto *renderingManager = RenderingManager::GetInstance();

  if (nullptr != m_FeedbackWindow)
  {
    if (m_FeedbackWindow != renderWindow)
      m_FeedbackWindow->SetCurrentCursor(VTK_CURSOR_DEFAULT);

    renderingManager->RequestUpdate(m_FeedbackWindow);
  }

  m_Feedback = feedback;
  m_FeedbackWindow = renderWindow;

  if (nullptr != renderWindow)
  {
    renderWindow->SetCurrentCursor(GetCursor(feedback));
    renderingManager->RequestUpdate(renderWindow);
  }
}

mitk::PlanarFigure *mitk::PlanarFigureInteractor::GetFigure() const
{
  const auto *node = this->GetDataNode();
  return nullptr != node ? dynamic_cast<PlanarFigure *>(node->GetData()) : nullptr;
}

std::optional<mitk::Point2D> mitk::PlanarFigureInteractor::GetPlanePosition(const InteractionEvent *interactionEvent) const
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  const auto *figure = this->GetFigure();

  if (nullptr == positionEvent || nullptr == figure)
    return std::nullopt;

  const auto *figurePlane = figure->GetPlaneGeometry();
  const auto *rendererPlane = interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry();

  if (nullptr == figurePlane || nullptr == rendererPlane || !PlanarFigureViewHelpers::IsOnPlane(figurePlane, rendererPlane))
    return std::nullopt;

  Point2D position;
  figurePlane->Map(positionEvent->GetPositionInWorld(), position);
  return position;
}

double mitk::PlanarFigureInteractor::GetMillimetersPerPixel(const BaseRenderer *renderer) const
{
  return renderer->GetScaleFactorMMPerDisplayUnit() * PlanarFigureViewHelpers::GetPixelScale(renderer);
}

double mitk::PlanarFigureInteractor::GetDisplayDistance(const InteractionEvent *interactionEvent, const Point2D &displayPosition) const
{
  return GetDisplayPosition(interactionEvent).EuclideanDistanceTo(displayPosition) / PlanarFigureViewHelpers::GetPixelScale(interactionEvent->GetSender());
}

mitk::PlanarFigureInteractor::Hit mitk::PlanarFigureInteractor::HitTest(const InteractionPositionEvent *positionEvent, bool isControlHeld) const
{
  const auto *figure = this->GetFigure();

  if (nullptr == figure || !figure->IsComplete())
    return {};

  const auto position = this->GetPlanePosition(positionEvent);

  if (!position.has_value())
    return {};

  const auto *renderer = positionEvent->GetSender();
  const auto millimetersPerPixel = this->GetMillimetersPerPixel(renderer);
  const auto tolerance = m_HitTolerance * millimetersPerPixel;

  // Ctrl inserts and removes vertices of figures whose contour runs through their control points.
  const auto canEditPoints = isControlHeld && figure->CanInsertControlPoints() && !figure->IsShapeLocked();

  const auto handles = figure->GetHandles();
  Hit hit;
  auto handleDistance = (PlanarFigureViewHelpers::HandleRadius * millimetersPerPixel) + tolerance;

  for (std::size_t i = 0; i < handles.size(); ++i)
  {
    if (figure->IsShapeLocked() && PlanarFigure::HandleRole::Center != handles[i].Role)
      continue;

    const auto distance = handles[i].Position.EuclideanDistanceTo(*position);

    if (distance <= handleDistance)
    {
      hit.Kind = HitKind::Handle;
      hit.Handle = i;
      handleDistance = distance;
    }
  }

  if (HitKind::Handle == hit.Kind)
  {
    const auto isRemovable = canEditPoints && PlanarFigure::HandleRole::Vertex == handles[*hit.Handle].Role &&
                             figure->GetNumberOfControlPoints() > figure->GetMinimumNumberOfControlPoints();

    if (isRemovable)
      hit.Kind = HitKind::Removal;

    return hit;
  }

  float lineWidth = 2.0f;
  this->GetDataNode()->GetFloatProperty("line width", lineWidth, renderer);

  const auto strokeDistance = 0.5 * lineWidth * millimetersPerPixel + tolerance;

  if (canEditPoints && figure->GetNumberOfControlPoints() < figure->GetMaximumNumberOfControlPoints())
  {
    const auto &points = figure->GetControlPoints();
    const auto numberOfSegments = figure->IsClosed() ? points.size() : points.size() - 1;
    auto insertionDistance = strokeDistance;

    for (std::size_t i = 0; i < numberOfSegments; ++i)
    {
      const auto closestPoint = GetClosestPointOnSegment(*position, points[i], points[(i + 1) % points.size()]);
      const auto distance = position->EuclideanDistanceTo(closestPoint);

      if (distance <= insertionDistance)
      {
        hit.Kind = HitKind::Insertion;
        hit.InsertionIndex = i + 1;
        hit.InsertionPosition = closestPoint;
        insertionDistance = distance;
      }
    }

    if (HitKind::Insertion == hit.Kind)
      return hit;
  }

  const auto contours = figure->GetContours();

  for (const auto &contour : contours)
  {
    const auto &points = contour.Points;
    const auto numberOfSegments = contour.Closed ? points.size() : points.size() - 1;

    for (std::size_t i = 0; i < numberOfSegments && points.size() > 1; ++i)
    {
      if (GetDistanceToSegment(*position, points[i], points[(i + 1) % points.size()]) <= strokeDistance)
      {
        hit.Kind = HitKind::Stroke;
        return hit;
      }
    }
  }

  // Holes and outlines alternate, so an odd number of closed contours around the position means inside.
  bool isInside = false;

  for (const auto &contour : contours)
  {
    if (contour.Closed && contour.Points.size() > 2 && IsInside(*position, contour.Points))
      isInside = !isInside;
  }

  if (isInside)
    hit.Kind = HitKind::Interior;

  return hit;
}

mitk::PlanarFigureInteractor::Hit mitk::PlanarFigureInteractor::GetHit(const InteractionEvent *interactionEvent) const
{
  if (ExclusiveInteraction::IsClaimed())
    return {};

  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);

  if (nullptr == positionEvent)
    positionEvent = GetIfRendererExists(m_LastMove);

  if (nullptr == positionEvent)
    return {};

  const auto isControlHeld = IsControlHeld(interactionEvent);
  const auto hit = this->HitTest(positionEvent, isControlHeld);
  const auto *renderer = positionEvent->GetSender();
  const auto dataStorage = renderer->GetDataStorage();

  // The wide band along a stroke, or along an edge where Ctrl would insert a point, yields
  // to the small handles of other figures.
  const auto isEdgeHit = HitKind::Stroke == hit.Kind || HitKind::Insertion == hit.Kind;

  if ((HitKind::Interior != hit.Kind && !isEdgeHit) || dataStorage.IsNull())
    return hit;

  const auto area = isEdgeHit ? 0.0 : GetEnclosedArea(this->GetFigure()->GetContours());
  const auto nodes = dataStorage->GetSubset(TNodePredicateDataType<PlanarFigure>::New());

  for (const auto &node : *nodes)
  {
    if (node == this->GetDataNode())
      continue;

    const auto *other = dynamic_cast<const PlanarFigureInteractor *>(node->GetDataInteractor().GetPointer());

    if (nullptr == other || !other->IsInteractionEnabled() || !node->IsVisible(renderer))
      continue;

    const auto otherHit = other->HitTest(positionEvent, isControlHeld);

    if (isEdgeHit)
    {
      if (HitKind::Handle == otherHit.Kind || HitKind::Removal == otherHit.Kind)
        return {};
    }
    else if (HitKind::Interior == otherHit.Kind)
    {
      if (GetEnclosedArea(other->GetFigure()->GetContours()) < area)
        return {};
    }
    else if (HitKind::None != otherHit.Kind)
    {
      return {};
    }
  }

  return hit;
}

bool mitk::PlanarFigureInteractor::IsEmpty(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && 0 == figure->GetNumberOfControlPoints();
}

bool mitk::PlanarFigureInteractor::IsOnFlatPlane(const InteractionEvent *interactionEvent)
{
  const auto *plane = interactionEvent->GetSender()->GetCurrentWorldPlaneGeometry();
  return nullptr != plane && nullptr == dynamic_cast<const AbstractTransformGeometry *>(plane);
}

bool mitk::PlanarFigureInteractor::IsPlacedBySpan(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && PlanarFigure::PlacementKind::Span == figure->GetPlacementKind();
}

bool mitk::PlanarFigureInteractor::IsStamp(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && figure->IsBeingPlaced() && figure->IsShapeLocked() && figure->GetNumberOfControlPoints() > 0;
}

bool mitk::PlanarFigureInteractor::IsComplete(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && figure->IsComplete();
}

bool mitk::PlanarFigureInteractor::IsHit(const InteractionEvent *interactionEvent)
{
  return HitKind::None != this->GetHit(interactionEvent).Kind;
}

bool mitk::PlanarFigureInteractor::IsBeyondDragThreshold(const InteractionEvent *interactionEvent)
{
  // Events of windows that do not show the figure's plane do not count, e.g. a press that would end its placement.
  return this->GetPlanePosition(interactionEvent).has_value() &&
         this->GetDisplayDistance(interactionEvent, m_PressPosition) > m_DragThreshold;
}

bool mitk::PlanarFigureInteractor::IsValidPoint(const InteractionEvent *interactionEvent)
{
  const auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);

  if (nullptr == figure || !position.has_value())
    return false;

  const auto numberOfControlPoints = figure->GetNumberOfControlPoints();

  if (numberOfControlPoints < 2)
    return true;

  // The last point follows the mouse; the others are placed.
  const auto *plane = figure->GetPlaneGeometry();
  const auto *renderer = interactionEvent->GetSender();
  const auto newPoint = PlanarFigureViewHelpers::ToDisplay(ClampToPlane(plane, *position), plane, renderer);
  const auto scale = PlanarFigureViewHelpers::GetPixelScale(renderer);

  for (unsigned int i = 0; i + 1 < numberOfControlPoints; ++i)
  {
    const auto placedPoint = PlanarFigureViewHelpers::ToDisplay(figure->GetControlPoint(i), plane, renderer);

    if (newPoint.EuclideanDistanceTo(placedPoint) / scale < MinimumPointDistance)
      return false;
  }

  return true;
}

bool mitk::PlanarFigureInteractor::ReachesMaximum(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && figure->GetNumberOfControlPoints() >= figure->GetMaximumNumberOfControlPoints();
}

bool mitk::PlanarFigureInteractor::CanFinish(const InteractionEvent *)
{
  const auto *figure = this->GetFigure();
  return nullptr != figure && figure->GetNumberOfControlPoints() > figure->GetMinimumNumberOfControlPoints();
}

bool mitk::PlanarFigureInteractor::IsRemovable(const InteractionEvent *interactionEvent)
{
  return HitKind::Removal == this->GetHit(interactionEvent).Kind;
}

bool mitk::PlanarFigureInteractor::IsInsertable(const InteractionEvent *interactionEvent)
{
  return HitKind::Insertion == this->GetHit(interactionEvent).Kind;
}

void mitk::PlanarFigureInteractor::UpdateHover(StateMachineAction *, InteractionEvent *interactionEvent)
{
  if (const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent); nullptr != positionEvent)
    m_LastMove = positionEvent;

  const auto hit = this->GetHit(interactionEvent);

  if (HitKind::None == hit.Kind)
  {
    this->SetFeedback({}, nullptr);
    return;
  }

  // Key events also reach the window with the keyboard focus, so the feedback goes where the mouse is.
  const auto *lastMove = m_LastMove.GetPointer();

  auto feedback = GetHoverFeedback(lastMove, hit.Handle);
  feedback.IsRemovalPreview = HitKind::Removal == hit.Kind;

  if (HitKind::Insertion == hit.Kind)
    feedback.InsertionPreview = hit.InsertionPosition;

  this->SetFeedback(feedback, GetRenderWindow(lastMove));
}

void mitk::PlanarFigureInteractor::EndHover(StateMachineAction *, InteractionEvent *)
{
  m_LastMove = nullptr;
  this->SetFeedback({}, nullptr);
}

void mitk::PlanarFigureInteractor::Press(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);

  if (nullptr == figure || !position.has_value())
    return;

  m_PressPosition = GetDisplayPosition(interactionEvent);
  m_Grab = this->GetHit(interactionEvent);

  if (m_Grab.Handle.has_value())
  {
    m_GrabOffset = figure->GetHandles()[*m_Grab.Handle].Position - *position;
  }
  else if (HitKind::None != m_Grab.Kind)
  {
    m_GrabOffset = figure->GetControlPoint(0) - *position;
  }

  SelectUnlessSelected(this->GetDataNode(), figure);
}

void mitk::PlanarFigureInteractor::Drag(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);

  if (nullptr == figure || !position.has_value() || HitKind::None == m_Grab.Kind)
    return;

  const auto *plane = figure->GetPlaneGeometry();
  const auto target = *position + m_GrabOffset;

  if (m_Grab.Handle.has_value())
  {
    const auto handles = figure->GetHandles();

    // The figure may have been changed elsewhere since the press.
    if (*m_Grab.Handle >= handles.size())
      return;

    figure->MoveHandle(*m_Grab.Handle, IsBoundToPlane(handles[*m_Grab.Handle].Role) ? ClampToPlane(plane, target) : target);
  }
  else
  {
    figure->Translate(ClampOffsetToPlane(plane, GetPositionsBoundToPlane(*figure), target - figure->GetControlPoint(0)));
  }

  this->SetFeedback(GetHoverFeedback(interactionEvent, m_Grab.Handle), GetRenderWindow(interactionEvent));

  RenderingManager::GetInstance()->RequestUpdateAll();
  figure->InvokeEvent(PointMovedPlanarFigureEvent());
}

void mitk::PlanarFigureInteractor::EndDrag(StateMachineAction *, InteractionEvent *)
{
  auto *figure = this->GetFigure();

  if (nullptr != figure && HitKind::None != m_Grab.Kind)
    figure->InvokeEvent(EndInteractionPlanarFigureEvent());

  m_Grab = {};
}

void mitk::PlanarFigureInteractor::RemovePoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto hit = this->GetHit(interactionEvent);

  if (nullptr == figure || HitKind::Removal != hit.Kind)
    return;

  SelectUnlessSelected(this->GetDataNode(), figure);
  figure->RemoveControlPoint(static_cast<unsigned int>(*hit.Handle));

  this->SetFeedback(GetHoverFeedback(interactionEvent, std::nullopt), GetRenderWindow(interactionEvent));
  RenderingManager::GetInstance()->RequestUpdateAll();

  figure->InvokeEvent(EndInteractionPlanarFigureEvent());
}

void mitk::PlanarFigureInteractor::InsertPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);
  const auto hit = this->GetHit(interactionEvent);

  if (nullptr == figure || !position.has_value() || HitKind::Insertion != hit.Kind)
    return;

  SelectUnlessSelected(this->GetDataNode(), figure);
  figure->InsertControlPoint(hit.InsertionIndex, hit.InsertionPosition);

  // The new point is dragged until the button is released.
  m_PressPosition = GetDisplayPosition(interactionEvent);
  m_Grab = {};
  m_Grab.Kind = HitKind::Handle;
  m_Grab.Handle = hit.InsertionIndex;
  m_GrabOffset = hit.InsertionPosition - *position;

  this->SetFeedback(GetHoverFeedback(interactionEvent, m_Grab.Handle), GetRenderWindow(interactionEvent));
  RenderingManager::GetInstance()->RequestUpdateAll();

  figure->InvokeEvent(PointMovedPlanarFigureEvent());
}

void mitk::PlanarFigureInteractor::BeginPlacement(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  auto *renderer = interactionEvent->GetSender();

  if (nullptr == figure)
    return;

  figure->SetPlaneGeometry(GetSlicePlane(renderer));

  const auto position = this->GetPlanePosition(interactionEvent);

  if (!position.has_value())
    return;

  m_PressPosition = GetDisplayPosition(interactionEvent);
  m_SpanStart = ClampToPlane(figure->GetPlaneGeometry(), *position);

  figure->BeginPlacement();

  if (PlanarFigure::PlacementKind::Span == figure->GetPlacementKind())
    figure->SetFromSpan(m_SpanStart, m_SpanStart);
  else
    figure->SetControlPoints({ m_SpanStart, m_SpanStart });

  // Applications use it to show a figure in the window it was drawn in.
  this->GetDataNode()->SetBoolProperty("PlanarFigureInitializedWindow", true, renderer);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PlanarFigureInteractor::MoveFloatingPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);

  if (nullptr == figure || !position.has_value() || 0 == figure->GetNumberOfControlPoints())
    return;

  const auto *plane = figure->GetPlaneGeometry();

  if (PlanarFigure::PlacementKind::Span == figure->GetPlacementKind())
  {
    figure->SetFromSpan(m_SpanStart, IsSpanEndBoundToPlane(*figure) ? ClampToPlane(plane, *position) : *position);
  }
  else
  {
    auto points = figure->GetControlPoints();
    points.back() = ClampToPlane(plane, *position);
    figure->SetControlPoints(points);
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PlanarFigureInteractor::AppendPoint(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  const auto position = this->GetPlanePosition(interactionEvent);

  if (nullptr == figure || !position.has_value() || 0 == figure->GetNumberOfControlPoints())
    return;

  const auto point = ClampToPlane(figure->GetPlaneGeometry(), *position);

  auto points = figure->GetControlPoints();
  points.back() = point;
  points.push_back(point);
  figure->SetControlPoints(points);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PlanarFigureInteractor::RemoveFloatingPoint(StateMachineAction *, InteractionEvent *)
{
  auto *figure = this->GetFigure();

  if (nullptr == figure || figure->GetNumberOfControlPoints() < 2)
    return;

  auto points = figure->GetControlPoints();
  points.pop_back();
  figure->SetControlPoints(points);
}

void mitk::PlanarFigureInteractor::RemoveLastPoint(StateMachineAction *, InteractionEvent *)
{
  auto *figure = this->GetFigure();

  // The first point stays; the placement is canceled elsewhere.
  if (nullptr == figure || figure->GetNumberOfControlPoints() < 3)
    return;

  auto points = figure->GetControlPoints();
  points.erase(points.end() - 2);
  figure->SetControlPoints(points);

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::PlanarFigureInteractor::EndPlacement(StateMachineAction *, InteractionEvent *)
{
  auto *figure = this->GetFigure();

  if (nullptr == figure)
    return;

  figure->EndPlacement();
  RenderingManager::GetInstance()->RequestUpdateAll();

  // Observers of the end of the placement may hand this interactor to the next figure.
  figure->InvokeEvent(EndInteractionPlanarFigureEvent());
  figure->InvokeEvent(EndPlacementPlanarFigureEvent());
}

void mitk::PlanarFigureInteractor::MoveStamp(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *figure = this->GetFigure();
  auto *renderer = interactionEvent->GetSender();

  if (nullptr == figure || !this->IsOnFlatPlane(interactionEvent))
    return;

  // The shape is kept in plane coordinates, so it moves to the slice under the mouse unchanged.
  figure->SetPlaneGeometry(GetSlicePlane(renderer));

  const auto position = this->GetPlanePosition(interactionEvent);

  if (!position.has_value())
    return;

  figure->Translate(ClampOffsetToPlane(figure->GetPlaneGeometry(), GetPositionsBoundToPlane(*figure), *position - figure->GetControlPoint(0)));
  this->GetDataNode()->SetBoolProperty("PlanarFigureInitializedWindow", true, renderer);

  RenderingManager::GetInstance()->RequestUpdate(renderer->GetRenderWindow());
}
