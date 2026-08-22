/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBoundingShapeUtil.h"
#include <mitkBoundingShapeInteractor.h>
#include <mitkDisplayActionEventBroadcast.h>
#include <mitkInteractionConst.h>
#include <mitkInteractionEventObserver.h>
#include <mitkInteractionKeyEvent.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkMouseWheelEvent.h>
#include <mitkBaseRenderer.h>
#include <mitkPlaneGeometry.h>

#include <vtkCamera.h>
#include <vtkInteractorObserver.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkSmartPointer.h>

#include <usGetModuleContext.h>
#include <usModuleRegistry.h>

#include <algorithm>
#include <cmath>

// The property names shared with the mappers are defined in mitkBoundingShapeUtil.h.

namespace mitk
{
  itkEventMacroDefinition(BoundingShapeInteractionEvent, itk::AnyEvent);

  class BoundingShapeInteractor::Impl
  {
  public:
    Impl() : OriginalInteractionEnabled(false) {}

    ~Impl() {}
    bool OriginalInteractionEnabled;
    Point3D InitialPickedWorldPoint;
    Point3D LastPickedWorldPoint;
    BaseGeometry::BoundsArrayType InitialBounds;
    std::vector<Handle> Handles;
    Handle ActiveHandle;
    std::map<us::ServiceReferenceU, mitk::EventConfig> DisplayInteractionConfigs;
  };
}

namespace
{
  /**
   * \brief Display-to-world conversion for handle dragging.
   *
   * In a 2D render window this is the regular un-projection onto the slice plane. In a 3D
   * render window BaseRenderer::DisplayToWorld picks the depth buffer, so the result would
   * depend on whatever scene geometry happens to lie under the cursor and jump when the
   * cursor leaves it; instead, un-project at the depth of \p referencePoint, i.e. drag on
   * the camera-parallel plane through the grabbed handle.
   */
  mitk::Point3D GetHandleDragWorldPoint(const mitk::InteractionPositionEvent *positionEvent,
                                        const mitk::Point3D &referencePoint)
  {
    mitk::BaseRenderer *renderer = positionEvent->GetSender();
    const mitk::Point2D displayPoint = positionEvent->GetPointerPositionOnScreen();

    mitk::Point3D worldPoint;

    if (renderer->GetMapperID() == mitk::BaseRenderer::Standard2D)
    {
      renderer->DisplayToWorld(displayPoint, worldPoint);
      return worldPoint;
    }

    vtkRenderer *vtkRender = renderer->GetVtkRenderer();

    vtkRender->SetWorldPoint(referencePoint[0], referencePoint[1], referencePoint[2], 1.0);
    vtkRender->WorldToDisplay();
    const double depth = vtkRender->GetDisplayPoint()[2];

    vtkRender->SetDisplayPoint(displayPoint[0], displayPoint[1], depth);
    vtkRender->DisplayToWorld();
    const double *world = vtkRender->GetWorldPoint();

    for (int i = 0; i < 3; ++i)
      worldPoint[i] = world[i] / world[3];

    return worldPoint;
  }
}

mitk::BoundingShapeInteractor::BoundingShapeInteractor() : m_Impl(new Impl)
{
}

mitk::BoundingShapeInteractor::~BoundingShapeInteractor()
{
  this->RestoreNodeProperties();
  delete m_Impl;
}

void mitk::BoundingShapeInteractor::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually
  // executing an action
  CONNECT_CONDITION("isHoveringOverObject", CheckOverObject);
  CONNECT_CONDITION("isHoveringOverHandles", CheckOverHandles);

  // **Function** in the statemachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectObject", SelectObject);
  CONNECT_FUNCTION("deselectObject", DeselectObject);
  CONNECT_FUNCTION("deselectHandles", DeselectHandles);
  CONNECT_FUNCTION("initInteraction", InitInteraction);
  CONNECT_FUNCTION("translateObject", TranslateObject);
  CONNECT_FUNCTION("selectHandle", SelectHandle);
  CONNECT_FUNCTION("scaleObject", ScaleObject);
  // CONNECT_FUNCTION("rotateObject",RotateObject);
}

// RotateObject(StateMachineAction*, InteractionEvent* interactionEvent)
// void mitk::BoundingShapeInteractor::RotateGeometry(mitk::ScalarType angle, int rotationaxis, mitk::BaseGeometry*
// geometry)
//{
//  mitk::Vector3D rotationAxis = geometry->GetAxisVector(rotationaxis);
//  float pointX = 0.0f;
//  float pointY = 0.0f;
//  float pointZ = 0.0f;
//  mitk::Point3D pointOfRotation;
//  pointOfRotation.Fill(0.0);
//  this->GetDataNode()->GetFloatProperty(anchorPointX, pointX);
//  this->GetDataNode()->GetFloatProperty(anchorPointY, pointY);
//  this->GetDataNode()->GetFloatProperty(anchorPointZ, pointZ);
//  pointOfRotation[0] = pointX;
//  pointOfRotation[1] = pointY;
//  pointOfRotation[2] = pointZ;
//
//  mitk::RotationOperation* doOp = new mitk::RotationOperation(OpROTATE, pointOfRotation, rotationAxis, angle);
//
//  geometry->ExecuteOperation(doOp);
//  delete doOp;
//}

void mitk::BoundingShapeInteractor::DataNodeChanged()
{
  mitk::DataNode::Pointer newInputNode = this->GetDataNode();

  if (newInputNode == nullptr)
    return;

  newInputNode->AddProperty(BoundingShapeSelectedColorPropertyName, mitk::ColorProperty::New(0.0, 1.0, 0.0));

  newInputNode->SetProperty(BoundingShapePropertyName, mitk::BoolProperty::New(true));
  newInputNode->SetBoolProperty(BoundingShapeSelectedPropertyName, false);
  newInputNode->AddProperty(BoundingShapeActiveHandleIdPropertyName, mitk::IntProperty::New(-1));
  newInputNode->SetProperty("layer", mitk::IntProperty::New(101));
  newInputNode->SetBoolProperty("fixedLayer", mitk::BoolProperty::New(true));
  newInputNode->SetBoolProperty("pickable", true);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BoundingShapeInteractor::HandlePositionChanged(const InteractionEvent *interactionEvent, Point3D &center)
{
  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);

  std::vector<Point3D> cornerPoints = GetCornerPoints(geometry, true);

  // center based on half way of the distance between two opposing cornerpoints
  center = CalcAvgPoint(cornerPoints[7], cornerPoints[0]);

  const BaseRenderer *renderer = interactionEvent->GetSender();
  const PlaneGeometry *planeGeometry =
    renderer->GetMapperID() == BaseRenderer::Standard2D ? renderer->GetCurrentWorldPlaneGeometry() : nullptr;

  if (planeGeometry != nullptr && !planeGeometry->IsValid())
    planeGeometry = nullptr;

  m_Impl->Handles = ComputeHandles(cornerPoints, planeGeometry);
}

void mitk::BoundingShapeInteractor::SetDataNode(DataNode *node)
{
  this->RestoreNodeProperties(); // if there is another node set, restore it's color

  if (node == nullptr)
    return;
  DataInteractor::SetDataNode(node); // calls DataNodeChanged internally
  this->DataNodeChanged();
}

bool mitk::BoundingShapeInteractor::CheckOverObject(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  // translating the box is only offered in slice views: a 2D mouse movement cannot express
  // the intended 3D shift, so in a 3D render window the body is not even selectable
  if (interactionEvent->GetSender()->GetMapperID() != BaseRenderer::Standard2D)
    return false;

  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);

  // calculates translation based on offset+extent not on the transformation matrix (because the cube is located in the
  // center not in the origin)
  vtkSmartPointer<vtkMatrix4x4> imageTransform = geometry->GetVtkTransform()->GetMatrix();
  Point3D center = geometry->GetCenter();
  auto translation = vtkSmartPointer<vtkTransform>::New();
  auto transform = vtkSmartPointer<vtkTransform>::New();
  translation->Translate(center[0] - imageTransform->GetElement(0, 3),
                         center[1] - imageTransform->GetElement(1, 3),
                         center[2] - imageTransform->GetElement(2, 3));
  transform->SetMatrix(imageTransform);
  transform->PostMultiply();
  transform->Concatenate(translation);
  transform->Update();

  mitk::Vector3D extent;
  for (unsigned int i = 0; i < 3; ++i)
    extent[i] = (geometry->GetExtent(i));

  Point3D currentWorldPosition;
  Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();
  interactionEvent->GetSender()->DisplayToWorld(currentDisplayPosition, currentWorldPosition);

  ScalarType transformedPosition[4];
  transformedPosition[0] = currentWorldPosition[0];
  transformedPosition[1] = currentWorldPosition[1];
  transformedPosition[2] = currentWorldPosition[2];
  transformedPosition[3] = 1;
  // transform point from world to object coordinates
  transform->GetInverse()->TransformPoint(transformedPosition, transformedPosition);
  // check if the world point is within bounds
  bool isInside = (transformedPosition[0] >= (-extent[0] / 2.0)) && (transformedPosition[0] <= (extent[0] / 2.0)) &&
                  (transformedPosition[1] >= (-extent[1] / 2.0)) && (transformedPosition[1] <= (extent[1] / 2.0)) &&
                  (transformedPosition[2] >= (-extent[2] / 2.0)) && (transformedPosition[2] <= (extent[2] / 2.0));

  return isInside;
}

bool mitk::BoundingShapeInteractor::CheckOverHandles(const InteractionEvent *interactionEvent)
{
  const auto *positionEvent = dynamic_cast<const InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  Point3D boundingBoxCenter;
  HandlePositionChanged(interactionEvent, boundingBoxCenter);

  double scale = interactionEvent->GetSender()->GetScaleFactorMMPerDisplayUnit();
  mitk::DoubleProperty::Pointer handleSizeProperty =
    dynamic_cast<mitk::DoubleProperty *>(this->GetDataNode()->GetProperty(BoundingShapeHandleSizeFactorPropertyName));

  ScalarType initialHandleSize;
  if (handleSizeProperty != nullptr)
    initialHandleSize = handleSizeProperty->GetValue();
  else
    initialHandleSize = DefaultHandleSizeFactor;

  mitk::Point2D displaysize = interactionEvent->GetSender()->GetDisplaySizeInMM();
  ScalarType handlesize = ((displaysize[0] + displaysize[1]) / 2.0) * initialHandleSize;
  const ScalarType pickRadius = handlesize / scale;

  // no handle hovered yet; a match below sets the active id again
  this->GetDataNode()->GetPropertyList()->SetProperty(BoundingShapeActiveHandleIdPropertyName,
                                                      mitk::IntProperty::New(-1));

  const Point2D currentDisplayPosition = positionEvent->GetPointerPositionOnScreen();

  // never pick a handle close to the projected box center, so a degenerate/very small box
  // can still be grabbed by its body for translation
  Point2D displayCenterPoint;
  interactionEvent->GetSender()->WorldToDisplay(boundingBoxCenter, displayCenterPoint);
  if (currentDisplayPosition.EuclideanDistanceTo(displayCenterPoint) <= pickRadius)
    return false;

  // pick the nearest handle under the cursor; face and edge handles sit close together
  // on a small box, where first-in-list would shadow the visually closer one
  const Handle *nearestHandle = nullptr;
  ScalarType nearestDistance = pickRadius;

  for (const auto &handle : m_Impl->Handles)
  {
    Point2D displayHandlePosition;
    interactionEvent->GetSender()->WorldToDisplay(handle.GetPosition(), displayHandlePosition);
    const ScalarType distance = currentDisplayPosition.EuclideanDistanceTo(displayHandlePosition);

    if (distance < nearestDistance)
    {
      nearestDistance = distance;
      nearestHandle = &handle;
    }
  }

  if (nearestHandle == nullptr)
    return false;

  m_Impl->ActiveHandle = *nearestHandle;
  this->GetDataNode()->GetPropertyList()->SetProperty(BoundingShapeActiveHandleIdPropertyName,
                                                      mitk::IntProperty::New(nearestHandle->GetIndex()));
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}

void mitk::BoundingShapeInteractor::SelectHandle(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction();
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  // the body highlight is dropped while a handle is hot; the handle itself is highlighted
  // via the active handle id
  node->SetBoolProperty(BoundingShapeSelectedPropertyName, false);
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::DeselectHandles(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction();
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  this->GetDataNode()->GetPropertyList()->SetProperty(BoundingShapeActiveHandleIdPropertyName,
                                                      mitk::IntProperty::New(-1));
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();

  return;
}

void mitk::BoundingShapeInteractor::SelectObject(StateMachineAction *, InteractionEvent *)
{
  this->DisableOriginalInteraction(); // disable crosshair interaction and scrolling if user is hovering over the object
  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  node->SetBoolProperty(BoundingShapeSelectedPropertyName, true);
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::DeselectObject(StateMachineAction *, InteractionEvent *)
{
  this->EnableOriginalInteraction(); // enable crosshair interaction and scrolling if user is hovering over the object

  DataNode::Pointer node = this->GetDataNode();

  if (node.IsNull())
    return;

  node->SetBoolProperty(BoundingShapeSelectedPropertyName, false);
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  RenderingManager::GetInstance()->RequestUpdateAll();
  return;
}

void mitk::BoundingShapeInteractor::InitInteraction(StateMachineAction *, InteractionEvent *interactionEvent)
{
  InitMembers(interactionEvent);
}

bool mitk::BoundingShapeInteractor::InitMembers(InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return false;

  // get initial position coordinates; a scale drag in a 3D window is measured on the
  // camera-parallel plane through the grabbed handle
  m_Impl->InitialPickedWorldPoint = GetHandleDragWorldPoint(positionEvent, m_Impl->ActiveHandle.GetPosition());
  m_Impl->LastPickedWorldPoint = m_Impl->InitialPickedWorldPoint;

  // scale drags apply absolute mouse offsets to the bounds captured at the press
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  m_Impl->InitialBounds = this->GetDataNode()->GetData()->GetGeometry(timeStep)->GetBounds();

  return true;
}

void mitk::BoundingShapeInteractor::TranslateObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry =
    this->GetDataNode()->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep(timeStep);
  Point3D currentPickedPoint;
  interactionEvent->GetSender()->DisplayToWorld(positionEvent->GetPointerPositionOnScreen(), currentPickedPoint);

  Vector3D worldMove;
  worldMove[0] = currentPickedPoint[0] - m_Impl->LastPickedWorldPoint[0];
  worldMove[1] = currentPickedPoint[1] - m_Impl->LastPickedWorldPoint[1];
  worldMove[2] = currentPickedPoint[2] - m_Impl->LastPickedWorldPoint[2];

  // Snap the translation to whole voxels in the box's own (index) space so the box stays on the
  // voxel grid for any orientation, matching the resize behaviour (a per-world-axis rounding would
  // drift an oblique box off the grid).
  auto inverse = mitk::AffineTransform3D::New();
  geometry->GetIndexToWorldTransform()->GetInverse(inverse);
  Vector3D indexMove = inverse->TransformVector(worldMove);
  indexMove[0] = std::round(indexMove[0]);
  indexMove[1] = std::round(indexMove[1]);
  indexMove[2] = std::round(indexMove[2]);
  Vector3D interactionMove = geometry->GetIndexToWorldTransform()->TransformVector(indexMove);

  if (indexMove[0] != 0.0 || indexMove[1] != 0.0 || indexMove[2] != 0.0) // only update if at least one voxel moved
  {
    m_Impl->LastPickedWorldPoint = currentPickedPoint;

    geometry->SetOrigin(geometry->GetOrigin() + interactionMove);

    this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
    this->GetDataNode()->GetData()->Modified();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
  return;
}

void mitk::BoundingShapeInteractor::ScaleObject(StateMachineAction *, InteractionEvent *interactionEvent)
{
  auto *positionEvent = dynamic_cast<InteractionPositionEvent *>(interactionEvent);
  if (positionEvent == nullptr)
    return;

  GeometryData::Pointer geometryData = dynamic_cast<GeometryData *>(this->GetDataNode()->GetData());
  int timeStep = interactionEvent->GetSender()->GetTimeStep(this->GetDataNode()->GetData());
  mitk::BaseGeometry::Pointer geometry = geometryData->GetGeometry(timeStep);

  const Point3D currentPickedPoint = GetHandleDragWorldPoint(positionEvent, m_Impl->ActiveHandle.GetPosition());

  // Total mouse offset since the press, in the box's own (index) space. Snapping the total
  // offset to whole voxels keeps the box on the voxel grid for any orientation, preserves
  // sub-voxel remainders between events, and makes dragging back an exact restore.
  auto inverse = mitk::AffineTransform3D::New();
  geometry->GetIndexToWorldTransform()->GetInverse(inverse);
  const Vector3D totalIndexMove = inverse->TransformVector(currentPickedPoint - m_Impl->InitialPickedWorldPoint);

  const AxisMovedBounds &movedBounds = m_Impl->ActiveHandle.GetMovedBounds();
  BaseGeometry::BoundsArrayType newBounds = m_Impl->InitialBounds;

  for (unsigned int axis = 0; axis < 3; ++axis)
  {
    if (movedBounds[axis] == MovedBound::None)
      continue;

    const ScalarType delta = std::round(totalIndexMove[axis]);

    // clamp so the extent stays positive: the moved bound stops just before the anchored
    // one instead of pushing through it
    const ScalarType extent = m_Impl->InitialBounds[2 * axis + 1] - m_Impl->InitialBounds[2 * axis];

    if (movedBounds[axis] == MovedBound::Maximum)
      newBounds[2 * axis + 1] += std::max(delta, -std::floor(extent - 0.01));
    else
      newBounds[2 * axis] += std::min(delta, std::floor(extent - 0.01));
  }

  if (newBounds == geometry->GetBounds()) // update only if the box changed by at least one voxel
    return;

  geometry->SetBounds(newBounds);
  geometry->Modified();
  this->GetDataNode()->GetData()->UpdateOutputInformation(); // Geometry is up-to-date
  this->GetDataNode()->GetData()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BoundingShapeInteractor::RestoreNodeProperties()
{
  mitk::DataNode::Pointer inputNode = this->GetDataNode();
  if (inputNode.IsNull())
    return;

  inputNode->SetProperty("layer", mitk::IntProperty::New(99));
  inputNode->SetProperty(BoundingShapePropertyName, mitk::BoolProperty::New(false));
  // removing the interaction properties reverts the body to its regular color and hides
  // the handles
  inputNode->GetPropertyList()->DeleteProperty(BoundingShapeActiveHandleIdPropertyName);
  inputNode->GetPropertyList()->DeleteProperty(BoundingShapeSelectedPropertyName);

  EnableOriginalInteraction();
  // update rendering
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::BoundingShapeInteractor::EnableOriginalInteraction()
{
  // Re-enabling InteractionEventObservers that have been previously disabled for legacy handling of Tools
  // in new interaction framework
  for (const auto& displayInteractionConfig : m_Impl->DisplayInteractionConfigs)
  {
    if (displayInteractionConfig.first)
    {
      auto displayActionEventBroadcast = static_cast<mitk::DisplayActionEventBroadcast *>(
        us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(displayInteractionConfig.first));

      if (nullptr != displayActionEventBroadcast)
      {
        // here the regular configuration is loaded again
        displayActionEventBroadcast->SetEventConfig(displayInteractionConfig.second);
      }
    }
  }

  m_Impl->DisplayInteractionConfigs.clear();
  m_Impl->OriginalInteractionEnabled = true;
}

void mitk::BoundingShapeInteractor::DisableOriginalInteraction()
{
  // dont deactivate twice, else we will clutter the config list ...
  if (false == m_Impl->OriginalInteractionEnabled)
    return;

  // As a legacy solution the display interaction of the new interaction framework is disabled here to avoid conflicts
  // with tools
  // Note: this only affects InteractionEventObservers (formerly known as Listeners) all DataNode specific interaction
  // will still be enabled
  m_Impl->DisplayInteractionConfigs.clear();
  auto eventObservers = us::GetModuleContext()->GetServiceReferences<mitk::InteractionEventObserver>();
  for (const auto& eventObserver : eventObservers)
  {
    auto *displayActionEventBroadcast = dynamic_cast<mitk::DisplayActionEventBroadcast *>(
      us::GetModuleContext()->GetService<mitk::InteractionEventObserver>(eventObserver));
    if (nullptr != displayActionEventBroadcast)
    {
      // remember the original configuration
      m_Impl->DisplayInteractionConfigs.insert(std::make_pair(eventObserver, displayActionEventBroadcast->GetEventConfig()));
      // here the alternative configuration is loaded
      displayActionEventBroadcast->AddEventConfig("DisplayConfigBlockLMB.xml");
    }
  }

  m_Impl->OriginalInteractionEnabled = false;
}
