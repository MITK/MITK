/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkDisplayInteractor.h"

#include "mitkBaseRenderer.h"
#include "mitkCameraController.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkPropertyList.h"
#include <mitkAbstractTransformGeometry.h>
#include <mitkRotationOperation.h>
#include <cstring>
// level window
#include "mitkLevelWindow.h"
#include "mitkLevelWindowProperty.h"
#include "mitkLine.h"
#include "mitkNodePredicateDataType.h"
#include "mitkStandaloneDataStorage.h"
#include "vtkRenderWindowInteractor.h"

// Rotation
#include "mitkInteractionConst.h"
#include "rotate_cursor.xpm"
#include <mitkImagePixelReadAccessor.h>
#include <mitkRotationOperation.h>

#include "mitkImage.h"
#include "mitkImagePixelReadAccessor.h"
#include "mitkPixelTypeMultiplex.h"
#include "mitkStatusBar.h"

#include <mitkCompositePixelValueToString.h>

void mitk::DisplayInteractor::Notify(InteractionEvent *interactionEvent, bool isHandled)
{
  // to use the state machine pattern,
  // the event is passed to the state machine interface to be handled
  if (!isHandled || m_AlwaysReact)
  {
    HandleEvent(interactionEvent, nullptr);
  }
}

mitk::DisplayInteractor::DisplayInteractor()
  : m_IndexToSliceModifier(4)
  , m_AutoRepeat(false)
  , m_InvertScrollDirection(false)
  , m_InvertZoomDirection(false)
  , m_InvertMoveDirection(false)
  , m_InvertLevelWindowDirection(false)
  , m_AlwaysReact(false)
  , m_ZoomFactor(2)
  , m_LinkPlanes(true)
{
  m_StartCoordinateInMM.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_LastCoordinateInMM.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);
}

mitk::DisplayInteractor::~DisplayInteractor()
{
  // nothing here
}

void mitk::DisplayInteractor::ConnectActionsAndFunctions()
{
  CONNECT_CONDITION("check_position_event", CheckPositionEvent);
  CONNECT_CONDITION("check_can_rotate", CheckRotationPossible);
  CONNECT_CONDITION("check_can_swivel", CheckSwivelPossible);

  CONNECT_FUNCTION("init", Init);
  CONNECT_FUNCTION("move", Move);
  CONNECT_FUNCTION("zoom", Zoom);
  CONNECT_FUNCTION("scroll", Scroll);
  CONNECT_FUNCTION("ScrollOneDown", ScrollOneDown);
  CONNECT_FUNCTION("ScrollOneUp", ScrollOneUp);
  CONNECT_FUNCTION("levelWindow", AdjustLevelWindow);
  CONNECT_FUNCTION("setCrosshair", SetCrosshair);

  CONNECT_FUNCTION("updateStatusbar", UpdateStatusbar)

  CONNECT_FUNCTION("startRotation", StartRotation);
  CONNECT_FUNCTION("endRotation", EndRotation);
  CONNECT_FUNCTION("rotate", Rotate);

  CONNECT_FUNCTION("swivel", Swivel);

  CONNECT_FUNCTION("IncreaseTimeStep", IncreaseTimeStep);
  CONNECT_FUNCTION("DecreaseTimeStep", DecreaseTimeStep);
}

bool mitk::DisplayInteractor::CheckPositionEvent(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
  {
    return false;
  }

  return true;
}

bool mitk::DisplayInteractor::CheckRotationPossible(const mitk::InteractionEvent *interactionEvent)
{
  // Decide between moving and rotation slices.
  /*
  Detailed logic:

  1. Find the SliceNavigationController that has sent the event: this one defines our rendering plane and will NOT be
  rotated. Needs not even be counted or checked.
  2. Inspect every other SliceNavigationController
  - calculate the line intersection of this SliceNavigationController's plane with our rendering plane
  - if there is NO interesection, ignore and continue
  - IF there is an intersection
  - check the mouse cursor's distance from that line.
  0. if the line is NOT near the cursor, remember the plane as "one of the other planes" (which can be rotated in
  "locked" mode)
  1. on first line near the cursor,  just remember this intersection line as THE other plane that we want to rotate
  2. on every consecutive line near the cursor, check if the line is geometrically identical to the line that we want to
  rotate
  - if yes, we just push this line to the "other" lines and rotate it along
  - if no, then we have a situation where the mouse is near two other lines (e.g. crossing point) and don't want to
  rotate
  */
  const auto *posEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (posEvent == nullptr)
    return false;

  BaseRenderer *clickedRenderer = posEvent->GetSender();
  const PlaneGeometry *ourViewportGeometry = (clickedRenderer->GetCurrentWorldPlaneGeometry());

  if (!ourViewportGeometry)
    return false;

  Point3D cursorPosition = posEvent->GetPositionInWorld();
  const auto spacing = ourViewportGeometry->GetSpacing();
  const PlaneGeometry *geometryToBeRotated = nullptr; // this one is under the mouse cursor
  const PlaneGeometry *anyOtherGeometry = nullptr;    // this is also visible (for calculation of intersection ONLY)
  Line3D intersectionLineWithGeometryToBeRotated;

  bool hitMultipleLines(false);
  m_SNCsToBeRotated.clear();

  const double threshholdDistancePixels = 12.0;

  auto renWindows = interactionEvent->GetSender()->GetRenderingManager()->GetAllRegisteredRenderWindows();

  for (auto renWin : renWindows)
  {
    SliceNavigationController *snc = BaseRenderer::GetInstance(renWin)->GetSliceNavigationController();

    // If the mouse cursor is in 3D Renderwindow, do not check for intersecting planes.
    if (BaseRenderer::GetInstance(renWin)->GetMapperID() == BaseRenderer::Standard3D)
      continue;

    const PlaneGeometry *otherRenderersRenderPlane = snc->GetCurrentPlaneGeometry();
    if (otherRenderersRenderPlane == nullptr)
      continue; // ignore, we don't see a plane

    // check if there is an intersection
    Line3D intersectionLine; // between rendered/clicked geometry and the one being analyzed
    if (!ourViewportGeometry->IntersectionLine(otherRenderersRenderPlane, intersectionLine))
    {
      continue; // we ignore this plane, it's parallel to our plane
    }

    // check distance from intersection line
    const double distanceFromIntersectionLine =
      intersectionLine.Distance(cursorPosition) / spacing[snc->GetDefaultViewDirection()];

    // far away line, only remember for linked rotation if necessary
    if (distanceFromIntersectionLine > threshholdDistancePixels)
    {
      anyOtherGeometry = otherRenderersRenderPlane; // we just take the last one, so overwrite each iteration (we just
                                                    // need some crossing point)
      // TODO what about multiple crossings? NOW we have undefined behavior / random crossing point is used
      if (m_LinkPlanes)
      {
        m_SNCsToBeRotated.push_back(snc);
      }
    }
    else // close to cursor
    {
      if (geometryToBeRotated == nullptr) // first one close to the cursor
      {
        geometryToBeRotated = otherRenderersRenderPlane;
        intersectionLineWithGeometryToBeRotated = intersectionLine;
        m_SNCsToBeRotated.push_back(snc);
      }
      else
      {
        // compare to the line defined by geometryToBeRotated: if identical, just rotate this otherRenderersRenderPlane
        // together with the primary one
        //                                                     if different, DON'T rotate
        if (intersectionLine.IsParallel(intersectionLineWithGeometryToBeRotated) &&
            intersectionLine.Distance(intersectionLineWithGeometryToBeRotated.GetPoint1()) < mitk::eps)
        {
          m_SNCsToBeRotated.push_back(snc);
        }
        else
        {
          hitMultipleLines = true;
        }
      }
    }
  }

  bool moveSlices(true);

  if (geometryToBeRotated && anyOtherGeometry && ourViewportGeometry && !hitMultipleLines)
  {
    // assure all three are valid, so calculation of center of rotation can be done
    moveSlices = false;
  }
  // question in state machine is: "rotate?"
  if (moveSlices) // i.e. NOT rotate
  {
    return false;
  }
  else
  { // we DO have enough information for rotation
    m_LastCursorPosition = intersectionLineWithGeometryToBeRotated.Project(
      cursorPosition); // remember where the last cursor position ON THE LINE has been observed

    if (anyOtherGeometry->IntersectionPoint(
          intersectionLineWithGeometryToBeRotated,
          m_CenterOfRotation)) // find center of rotation by intersection with any of the OTHER lines
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  return false;
}

bool mitk::DisplayInteractor::CheckSwivelPossible(const mitk::InteractionEvent *interactionEvent)
{
  const ScalarType ThresholdDistancePixels = 6.0;

  // Decide between moving and rotation: if we're close to the crossing
  // point of the planes, moving mode is entered, otherwise
  // rotation/swivel mode
  const auto *posEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);

  BaseRenderer *renderer = interactionEvent->GetSender();

  if (!posEvent || !renderer)
    return false;

  const Point3D &cursor = posEvent->GetPositionInWorld();

  m_SNCsToBeRotated.clear();

  const PlaneGeometry *clickedGeometry(nullptr);
  const PlaneGeometry *otherGeometry1(nullptr);
  const PlaneGeometry *otherGeometry2(nullptr);

  auto renWindows = interactionEvent->GetSender()->GetRenderingManager()->GetAllRegisteredRenderWindows();

  for (auto renWin : renWindows)
  {
    SliceNavigationController *snc = BaseRenderer::GetInstance(renWin)->GetSliceNavigationController();

    // If the mouse cursor is in 3D Renderwindow, do not check for intersecting planes.
    if (BaseRenderer::GetInstance(renWin)->GetMapperID() == BaseRenderer::Standard3D)
      continue;

    // unsigned int slice = (*iter)->GetSlice()->GetPos();
    // unsigned int time  = (*iter)->GetTime()->GetPos();

    const PlaneGeometry *planeGeometry = snc->GetCurrentPlaneGeometry();
    if (!planeGeometry)
      continue;

    if (snc == renderer->GetSliceNavigationController())
    {
      clickedGeometry = planeGeometry;
      m_SNCsToBeRotated.push_back(snc);
    }
    else
    {
      if (otherGeometry1 == nullptr)
      {
        otherGeometry1 = planeGeometry;
      }
      else
      {
        otherGeometry2 = planeGeometry;
      }
      if (m_LinkPlanes)
      {
        // If planes are linked, apply rotation to all planes
        m_SNCsToBeRotated.push_back(snc);
      }
    }
  }

  mitk::Line3D line;
  mitk::Point3D point;
  if ((clickedGeometry != nullptr) && (otherGeometry1 != nullptr) && (otherGeometry2 != nullptr) &&
      clickedGeometry->IntersectionLine(otherGeometry1, line) && otherGeometry2->IntersectionPoint(line, point))
  {
    m_CenterOfRotation = point;
    if (m_CenterOfRotation.EuclideanDistanceTo(cursor) < ThresholdDistancePixels)
    {
      return false;
    }
    else
    {
      m_ReferenceCursor = posEvent->GetPointerPositionOnScreen();

      // Get main axes of rotation plane and store it for rotation step
      m_RotationPlaneNormal = clickedGeometry->GetNormal();

      ScalarType xVector[] = {1.0, 0.0, 0.0};
      ScalarType yVector[] = {0.0, 1.0, 0.0};
      clickedGeometry->BaseGeometry::IndexToWorld(Vector3D(xVector), m_RotationPlaneXVector);
      clickedGeometry->BaseGeometry::IndexToWorld(Vector3D(yVector), m_RotationPlaneYVector);

      m_RotationPlaneNormal.Normalize();
      m_RotationPlaneXVector.Normalize();
      m_RotationPlaneYVector.Normalize();

      m_PreviousRotationAxis.Fill(0.0);
      m_PreviousRotationAxis[2] = 1.0;
      m_PreviousRotationAngle = 0.0;

      return true;
    }
  }
  else
  {
    return false;
  }
  return false;
}

void mitk::DisplayInteractor::Init(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = static_cast<InteractionPositionEvent *>(interactionEvent);

  m_LastDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  m_CurrentDisplayCoordinate = m_LastDisplayCoordinate;
  positionEvent->GetSender()->DisplayToPlane(m_LastDisplayCoordinate, m_StartCoordinateInMM);
  m_LastCoordinateInMM = m_StartCoordinateInMM;
}

void mitk::DisplayInteractor::Move(StateMachineAction *, InteractionEvent *interactionEvent)
{
  BaseRenderer *sender = interactionEvent->GetSender();
  auto *positionEvent = static_cast<InteractionPositionEvent *>(interactionEvent);

  float invertModifier = -1.0;
  if (m_InvertMoveDirection)
  {
    invertModifier = 1.0;
  }
  // perform translation
  Vector2D moveVector = (positionEvent->GetPointerPositionOnScreen() - m_LastDisplayCoordinate) * invertModifier;
  moveVector *= sender->GetScaleFactorMMPerDisplayUnit();

  sender->GetCameraController()->MoveBy(moveVector);
  sender->GetRenderingManager()->RequestUpdate(sender->GetRenderWindow());
  m_LastDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
}

void mitk::DisplayInteractor::SetCrosshair(mitk::StateMachineAction *, mitk::InteractionEvent *interactionEvent)
{
  auto* positionEvent = static_cast<InteractionPositionEvent*>(interactionEvent);
  Point3D pos = positionEvent->GetPositionInWorld();

  const BaseRenderer::Pointer sender = interactionEvent->GetSender();
  auto renWindows = sender->GetRenderingManager()->GetAllRegisteredRenderWindows();
  for (auto renWin : renWindows)
  {
    if (BaseRenderer::GetInstance(renWin)->GetMapperID() == BaseRenderer::Standard2D && renWin != sender->GetRenderWindow())
    {
      BaseRenderer::GetInstance(renWin)->GetSliceNavigationController()->SelectSliceByPoint(pos);
    }
  }
}

void mitk::DisplayInteractor::IncreaseTimeStep(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto sliceNaviController = interactionEvent->GetSender()->GetRenderingManager()->GetTimeNavigationController();
  auto stepper = sliceNaviController->GetTime();
  stepper->SetAutoRepeat(true);
  stepper->Next();
}

void mitk::DisplayInteractor::DecreaseTimeStep(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto sliceNaviController = interactionEvent->GetSender()->GetRenderingManager()->GetTimeNavigationController();
  auto stepper = sliceNaviController->GetTime();
  stepper->SetAutoRepeat(true);
  stepper->Previous();
}

void mitk::DisplayInteractor::Zoom(StateMachineAction *, InteractionEvent *interactionEvent)
{
  float factor = 1.0;
  float distance = 0;

  if (m_ZoomDirection == "updown")
  {
    distance = m_CurrentDisplayCoordinate[1] - m_LastDisplayCoordinate[1];
  }
  else
  {
    distance = m_CurrentDisplayCoordinate[0] - m_LastDisplayCoordinate[0];
  }

  if (m_InvertZoomDirection)
  {
    distance *= -1.0;
  }

  // set zooming speed
  if (distance < 0.0)
  {
    factor = 1.0 / m_ZoomFactor;
  }
  else if (distance > 0.0)
  {
    factor = 1.0 * m_ZoomFactor;
  }

  auto* positionEvent = static_cast<InteractionPositionEvent*>(interactionEvent);
  m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();

  if (factor != 1.0)
  {
    const BaseRenderer::Pointer sender = interactionEvent->GetSender();
    sender->GetCameraController()->Zoom(factor, m_StartCoordinateInMM);
    sender->GetRenderingManager()->RequestUpdate(sender->GetRenderWindow());
  }
}

void mitk::DisplayInteractor::Scroll(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto* positionEvent = static_cast<InteractionPositionEvent *>(interactionEvent);

  mitk::SliceNavigationController::Pointer sliceNaviController = interactionEvent->GetSender()->GetSliceNavigationController();
  if (sliceNaviController)
  {
    int delta = 0;
    // Scrolling direction
    if (m_ScrollDirection == "updown")
    {
      delta = static_cast<int>(m_LastDisplayCoordinate[1] - positionEvent->GetPointerPositionOnScreen()[1]);
    }
    else
    {
      delta = static_cast<int>(m_LastDisplayCoordinate[0] - positionEvent->GetPointerPositionOnScreen()[0]);
    }

    if (m_InvertScrollDirection)
    {
      delta *= -1;
    }

    // Set how many pixels the mouse has to be moved to scroll one slice
    // if we moved less than 'm_IndexToSliceModifier' pixels slice ONE slice only
    if (delta > 0 && delta < m_IndexToSliceModifier)
    {
      delta = m_IndexToSliceModifier;
    }
    else if (delta < 0 && delta > -m_IndexToSliceModifier)
    {
      delta = -m_IndexToSliceModifier;
    }
    delta /= m_IndexToSliceModifier;

    int newPos = sliceNaviController->GetSlice()->GetPos() + delta;

    // if auto repeat is on, start at first slice if you reach the last slice and vice versa
    int maxSlices = sliceNaviController->GetSlice()->GetSteps();
    if (m_AutoRepeat)
    {
      while (newPos < 0)
      {
        newPos += maxSlices;
      }

      while (newPos >= maxSlices)
      {
        newPos -= maxSlices;
      }
    }
    else
    {
      // if the new slice is below 0 we still show slice 0
      // due to the stepper using unsigned int we have to do this ourselves
      if (newPos < 1)
      {
        newPos = 0;
      }
    }

    m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
    m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();

    // set the new position
    sliceNaviController->GetSlice()->SetPos(newPos);
  }
}

void mitk::DisplayInteractor::ScrollOneDown(StateMachineAction *, InteractionEvent *interactionEvent)
{
  mitk::SliceNavigationController::Pointer sliceNaviController =
    interactionEvent->GetSender()->GetSliceNavigationController();
  if (!sliceNaviController->GetSliceLocked())
  {
    mitk::Stepper *stepper = sliceNaviController->GetSlice();
    if (stepper->GetSteps() <= 1)
    {
      stepper = sliceNaviController->GetTime();
    }
    stepper->Next();
  }
}

void mitk::DisplayInteractor::ScrollOneUp(StateMachineAction *, InteractionEvent *interactionEvent)
{
  mitk::SliceNavigationController::Pointer sliceNaviController =
    interactionEvent->GetSender()->GetSliceNavigationController();
  if (!sliceNaviController->GetSliceLocked())
  {
    mitk::Stepper *stepper = sliceNaviController->GetSlice();
    if (stepper->GetSteps() <= 1)
    {
      stepper = sliceNaviController->GetTime();
    }
    stepper->Previous();
  }
}

void mitk::DisplayInteractor::AdjustLevelWindow(StateMachineAction *, InteractionEvent *interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  auto *positionEvent = static_cast<InteractionPositionEvent *>(interactionEvent);

  m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();
  // search for active image
  mitk::DataStorage::Pointer storage = sender->GetDataStorage();
  mitk::DataNode::Pointer node = nullptr;
  mitk::DataStorage::SetOfObjects::ConstPointer allImageNodes = storage->GetSubset(mitk::NodePredicateDataType::New("Image"));
  for (unsigned int i = 0; i < allImageNodes->size(); ++i)
  {
    bool isActiveImage = false;
    bool propFound = allImageNodes->at(i)->GetBoolProperty("imageForLevelWindow", isActiveImage);

    if (propFound && isActiveImage)
    {
      node = allImageNodes->at(i);
      continue;
    }
  }
  if (node.IsNull())
  {
    node = storage->GetNode(mitk::NodePredicateDataType::New("Image"));
  }
  if (node.IsNull())
  {
    return;
  }

  mitk::LevelWindow lv = mitk::LevelWindow();
  node->GetLevelWindow(lv);
  ScalarType level = lv.GetLevel();
  ScalarType window = lv.GetWindow();

  int levelIndex = 0;
  int windowIndex = 1;

  if (m_LevelDirection != "leftright")
  {
    levelIndex = 1;
    windowIndex = 0;
  }

  int directionModifier = 1;
  if (m_InvertLevelWindowDirection)
  {
    directionModifier = -1;
  }

  // calculate adjustments from mouse movements
  level += (m_CurrentDisplayCoordinate[levelIndex] - m_LastDisplayCoordinate[levelIndex]) * static_cast<ScalarType>(2) *
           directionModifier;
  window += (m_CurrentDisplayCoordinate[windowIndex] - m_LastDisplayCoordinate[windowIndex]) *
            static_cast<ScalarType>(2) * directionModifier;

  lv.SetLevelWindow(level, window);
  dynamic_cast<mitk::LevelWindowProperty *>(node->GetProperty("levelwindow"))->SetLevelWindow(lv);

  sender->GetRenderingManager()->RequestUpdateAll();
}

void mitk::DisplayInteractor::StartRotation(mitk::StateMachineAction *, mitk::InteractionEvent *)
{
  this->SetMouseCursor(rotate_cursor_xpm, 0, 0);
}

void mitk::DisplayInteractor::EndRotation(mitk::StateMachineAction *, mitk::InteractionEvent *)
{
  this->ResetMouseCursor();
}

void mitk::DisplayInteractor::Rotate(mitk::StateMachineAction *, mitk::InteractionEvent *event)
{
  const auto *posEvent = dynamic_cast<const InteractionPositionEvent *>(event);
  if (posEvent == nullptr)
    return;

  Point3D cursor = posEvent->GetPositionInWorld();

  Vector3D toProjected = m_LastCursorPosition - m_CenterOfRotation;
  Vector3D toCursor = cursor - m_CenterOfRotation;

  // cross product: | A x B | = |A| * |B| * sin(angle)
  Vector3D axisOfRotation;
  vnl_vector_fixed<ScalarType, 3> vnlDirection = vnl_cross_3d(toCursor.GetVnlVector(), toProjected.GetVnlVector());
  axisOfRotation.SetVnlVector(vnlDirection);

  // scalar product: A * B = |A| * |B| * cos(angle)
  // tan = sin / cos
  ScalarType angle = -atan2((double)(axisOfRotation.GetNorm()), (double)(toCursor * toProjected));
  angle *= 180.0 / vnl_math::pi;
  m_LastCursorPosition = cursor;

  // create RotationOperation and apply to all SNCs that should be rotated
  RotationOperation rotationOperation(OpROTATE, m_CenterOfRotation, axisOfRotation, angle);

  // iterate the OTHER slice navigation controllers: these are filled in DoDecideBetweenRotationAndSliceSelection
  for (auto iter = m_SNCsToBeRotated.begin(); iter != m_SNCsToBeRotated.end(); ++iter)
  {
    TimeGeometry *timeGeometry = (*iter)->GetCreatedWorldGeometry();
    if (!timeGeometry)
      continue;

    timeGeometry->ExecuteOperation(&rotationOperation);

    (*iter)->SendCreatedWorldGeometryUpdate();
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::DisplayInteractor::Swivel(mitk::StateMachineAction *, mitk::InteractionEvent *event)
{
  const auto *posEvent = dynamic_cast<const InteractionPositionEvent *>(event);

  if (!posEvent)
    return;

  // Determine relative mouse movement projected onto world space
  Point2D cursor = posEvent->GetPointerPositionOnScreen();
  Vector2D relativeCursor = cursor - m_ReferenceCursor;
  Vector3D relativeCursorAxis = m_RotationPlaneXVector * relativeCursor[0] + m_RotationPlaneYVector * relativeCursor[1];

  // Determine rotation axis (perpendicular to rotation plane and cursor
  // movement)
  Vector3D rotationAxis = itk::CrossProduct(m_RotationPlaneNormal, relativeCursorAxis);

  ScalarType rotationAngle = relativeCursor.GetNorm() / 2.0;

  // Restore the initial plane pose by undoing the previous rotation
  // operation
  RotationOperation op(OpROTATE, m_CenterOfRotation, m_PreviousRotationAxis, -m_PreviousRotationAngle);

  SNCVector::iterator iter;
  for (iter = m_SNCsToBeRotated.begin(); iter != m_SNCsToBeRotated.end(); ++iter)
  {
    if (!(*iter)->GetSliceRotationLocked())
    {
      TimeGeometry *timeGeometry = (*iter)->GetCreatedWorldGeometry();
      if (!timeGeometry)
        continue;

      timeGeometry->ExecuteOperation(&op);
      (*iter)->SendCreatedWorldGeometryUpdate();
    }
  }

  // Apply new rotation operation to all relevant SNCs
  RotationOperation op2(OpROTATE, m_CenterOfRotation, rotationAxis, rotationAngle);

  for (iter = m_SNCsToBeRotated.begin(); iter != m_SNCsToBeRotated.end(); ++iter)
  {
    if (!(*iter)->GetSliceRotationLocked())
    {
      // Retrieve the TimeGeometry of this SliceNavigationController
      TimeGeometry *timeGeometry = (*iter)->GetCreatedWorldGeometry();
      if (!timeGeometry)
        continue;

      // Execute the new rotation
      timeGeometry->ExecuteOperation(&op2);

      // Notify listeners
      (*iter)->SendCreatedWorldGeometryUpdate();
    }
  }

  m_PreviousRotationAxis = rotationAxis;
  m_PreviousRotationAngle = rotationAngle;

  RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::DisplayInteractor::UpdateStatusbar(mitk::StateMachineAction *, mitk::InteractionEvent *event)
{
  const auto* posEvent = dynamic_cast<const InteractionPositionEvent *>(event);
  if (nullptr == posEvent)
  {
    return;
  }

  const mitk::BaseRenderer::Pointer baseRenderer = posEvent->GetSender();
  TNodePredicateDataType<mitk::Image>::Pointer isImageData = TNodePredicateDataType<mitk::Image>::New();
  auto globalCurrentTimePoint = baseRenderer->GetTime();
  mitk::DataStorage::SetOfObjects::ConstPointer nodes = baseRenderer->GetDataStorage()->GetSubset(isImageData).GetPointer();
  if (nodes.IsNull())
  {
    return;
  }

  // posEvent->GetPositionInWorld() would return the world position at the
  // time of initiating the interaction. However, we need to update the
  // status bar with the position after changing slice. Therefore, we
  // translate the same display position with the renderer again to
  // get the new world position.
  Point3D worldposition;
  baseRenderer->DisplayToWorld(posEvent->GetPointerPositionOnScreen(), worldposition);

  mitk::Image::Pointer image3D;
  mitk::DataNode::Pointer node;
  mitk::DataNode::Pointer topSourceNode;

  int component = 0;

  node = FindTopmostVisibleNode(nodes, worldposition, globalCurrentTimePoint, baseRenderer);
  if (node.IsNull())
  {
    return;
  }

  bool isBinary(false);
  node->GetBoolProperty("binary", isBinary);
  if (isBinary)
  {
    mitk::DataStorage::SetOfObjects::ConstPointer sourcenodes = baseRenderer->GetDataStorage()->GetSources(node, nullptr, true);
    if (!sourcenodes->empty())
    {
        topSourceNode = mitk::FindTopmostVisibleNode(sourcenodes, worldposition, globalCurrentTimePoint, baseRenderer);
     }
     if (topSourceNode.IsNotNull())
     {
        image3D = dynamic_cast<mitk::Image *>(topSourceNode->GetData());
        topSourceNode->GetIntProperty("Image.Displayed Component", component);
      }
      else
      {
        image3D = dynamic_cast<mitk::Image *>(node->GetData());
        node->GetIntProperty("Image.Displayed Component", component);
      }
     }
	  else
	  {
	    image3D = dynamic_cast<mitk::Image *>(node->GetData());
	    node->GetIntProperty("Image.Displayed Component", component);
	  }

  // get the position and gray value from the image and build up status bar text
  auto statusBar = StatusBar::GetInstance();
  if (image3D.IsNotNull() && statusBar != nullptr)
  {
    itk::Index<3> p;
    image3D->GetGeometry()->WorldToIndex(worldposition, p);

    auto pixelType = image3D->GetChannelDescriptor().GetPixelType().GetPixelType();
    if (pixelType == itk::ImageIOBase::RGB || pixelType == itk::ImageIOBase::RGBA)
    {
      std::string pixelValue = "Pixel RGB(A) value: ";
      pixelValue.append(ConvertCompositePixelValueToString(image3D, p));
      statusBar->DisplayImageInfo(worldposition, p, globalCurrentTimePoint, pixelValue.c_str());
    }
    else if (pixelType == itk::ImageIOBase::DIFFUSIONTENSOR3D || pixelType == itk::ImageIOBase::SYMMETRICSECONDRANKTENSOR)
    {
      std::string pixelValue = "See ODF Details view. ";
      statusBar->DisplayImageInfo(worldposition, p, globalCurrentTimePoint, pixelValue.c_str());
    }
    else
    {
      mitk::ScalarType pixelValue;
      mitkPixelTypeMultiplex5(mitk::FastSinglePixelAccess,
        image3D->GetChannelDescriptor().GetPixelType(),
        image3D,
        image3D->GetVolumeData(image3D->GetTimeGeometry()->TimePointToTimeStep(globalCurrentTimePoint)),
        p,
        pixelValue,
        component);
      statusBar->DisplayImageInfo(worldposition, p, globalCurrentTimePoint, pixelValue);
    }
  }
  else
  {
    statusBar->DisplayImageInfoInvalid();
  }
}

void mitk::DisplayInteractor::ConfigurationChanged()
{
  mitk::PropertyList::Pointer properties = GetAttributes();
  // auto repeat
  std::string strAutoRepeat = "";
  if (properties->GetStringProperty("autoRepeat", strAutoRepeat))
  {
    if (strAutoRepeat == "true")
    {
      m_AutoRepeat = true;
    }
    else
    {
      m_AutoRepeat = false;
    }
  }
  // pixel movement for scrolling one slice
  std::string strPixelPerSlice = "";
  if (properties->GetStringProperty("pixelPerSlice", strPixelPerSlice))
  {
    m_IndexToSliceModifier = atoi(strPixelPerSlice.c_str());
  }
  else
  {
    m_IndexToSliceModifier = 4;
  }
  // scroll direction
  if (!properties->GetStringProperty("scrollDirection", m_ScrollDirection))
  {
    m_ScrollDirection = "updown";
  }

  m_InvertScrollDirection = GetBoolProperty(properties, "invertScrollDirection", false);

  // zoom direction
  if (!properties->GetStringProperty("zoomDirection", m_ZoomDirection))
  {
    m_ZoomDirection = "updown";
  }

  m_InvertZoomDirection = GetBoolProperty(properties, "invertZoomDirection", false);

  m_InvertMoveDirection = GetBoolProperty(properties, "invertMoveDirection", false);

  if (!properties->GetStringProperty("levelWindowDirection", m_LevelDirection))
  {
    m_LevelDirection = "leftright";
  }

  m_InvertLevelWindowDirection = GetBoolProperty(properties, "invertLevelWindowDirection", false);

  // coupled rotation
  std::string strCoupled = "";
  if (properties->GetStringProperty("coupled", strCoupled))
  {
    if (strCoupled == "true")
      m_LinkPlanes = true;
    else
      m_LinkPlanes = false;
  }

  // zoom factor
  std::string strZoomFactor = "";
  properties->GetStringProperty("zoomFactor", strZoomFactor);
  m_ZoomFactor = .05;
  if (atoi(strZoomFactor.c_str()) > 0)
  {
    m_ZoomFactor = 1.0 + (atoi(strZoomFactor.c_str()) / 100.0);
  }
  // allwaysReact
  std::string strAlwaysReact = "";
  if (properties->GetStringProperty("alwaysReact", strAlwaysReact))
  {
    if (strAlwaysReact == "true")
    {
      m_AlwaysReact = true;
    }
    else
    {
      m_AlwaysReact = false;
    }
  }
  else
  {
    m_AlwaysReact = false;
  }
}

bool mitk::DisplayInteractor::FilterEvents(InteractionEvent *interactionEvent, DataNode * /*dataNode*/)
{
  if (interactionEvent->GetSender() == nullptr)
    return false;
  if (interactionEvent->GetSender()->GetMapperID() == BaseRenderer::Standard3D)
    return false;

  return true;
}

bool mitk::DisplayInteractor::GetBoolProperty(mitk::PropertyList::Pointer propertyList,
                                              const char *propertyName,
                                              bool defaultValue)
{
  std::string valueAsString;
  if (!propertyList->GetStringProperty(propertyName, valueAsString))
  {
    return defaultValue;
  }
  else
  {
    if (valueAsString == "true")
    {
      return true;
    }
    else
    {
      return false;
    }
  }
}
